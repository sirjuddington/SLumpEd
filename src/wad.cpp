////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// wad.cpp - Wadfile handing functions                                //
////////////////////////////////////////////////////////////////////////

// INCLUDES ////////////////////////////////////////////////////////////
#include "main.h"
#include "FreeImage.h"
#include "progbar.h"

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>

// VARIABLES ///////////////////////////////////////////////////////////
string map_lumps[12] =
{
	_T("THINGS"),
	_T("VERTEXES"),
	_T("LINEDEFS"),
	_T("SIDEDEFS"),
	_T("SECTORS"),
	_T("SEGS"),
	_T("SSECTORS"),
	_T("NODES"),
	_T("BLOCKMAP"),
	_T("REJECT"),
	_T("SCRIPTS"),
	_T("BEHAVIOR")
};

CVAR(Bool, determine_type_load, true, CVAR_SAVE)

void Wad::findMaps()
{
	// Find all maps
	for (DWORD l = 0; l < numLumps(); l++)
	{
		bool done = false;
		bool existing_map_lumps[12] = { false, false, false, false,
			false, false, false, false,
			false, false, false, false };

		int index = l;
		while (!done)
		{
			l++;

			if (l == numLumps())
				break;

			string name = directory[l]->getName();
			done = true;

			for (int s = 0; s < 12; s++)
			{
				if (name == map_lumps[s])
				{
					existing_map_lumps[s] = true;
					done = false;
				}
			}
		}

		l--;

		if (!memchr(existing_map_lumps, 0, 5))
			directory[index]->setType(LUMP_MAP);
	}
}

// Wad::open: Opens a wad file
// ---------------------------
bool Wad::open(string filename, bool load_data)
{
	if (filename == _T(""))
		return false;

	close();
	path = filename;

	// Open the file
	FILE *fp = fopen(chr(filename), "rb");

	// Check if it opened correctly
	if (!fp)
	{
		wxLogMessage(_T("Wad file \"%s\" cannot be found.\n"), filename);
		close();
		return false;
	}

	long num_lumps = 0;
	long filesize = 0;
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Read the header
	fread(&type, 1, 4, fp);			// Wad type
	fread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	fread(&dir_offset, 4, 1, fp);	// Offset to directory

	// Lock the wad if IWAD
	if (type[0] == 'I')
		locked = true;
	else
		locked = false;

	if (type[1] != 'W' || type[2] != 'A' || type[3] != 'D')
	{
		wxMessageBox(_T("Not a valid Doom wad file!"), _T("Error"), wxICON_ERROR);
		return false;
	}

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (long d = 0; d < num_lumps; d++)
	{
		char name[9] = "";
		long offset = 0;
		long size = 0;

		fread(&offset, 4, 1, fp);	// Offset
		fread(&size, 4, 1, fp);		// Size
		fread(name, 1, 8, fp);		// Name

		if (offset + size > filesize)
		{
			wxMessageBox(_T("Not a valid Doom wad file!"), _T("Error"), wxICON_ERROR);
			return false;
		}

		name[8] = '\0';
		Lump* nlump = new Lump(offset, size, wxString::FromAscii(name).Upper(), this);

		if (nlump->getName() == _T("P_START") || nlump->getName() == _T("PP_START"))
			patches[START] = d;

		if (nlump->getName() == _T("P_END") || nlump->getName() == _T("PP_END"))
			patches[END] = d;

		if (nlump->getName() == _T("F_START") || nlump->getName() == _T("FF_START"))
			flats[START] = d;

		if (nlump->getName() == _T("F_END") || nlump->getName() == _T("FF_END"))
			flats[END] = d;

		if (nlump->getName() == _T("S_START") || nlump->getName() == _T("SS_START"))
			sprites[START] = d;

		if (nlump->getName() == _T("S_END") || nlump->getName() == _T("SS_END"))
			sprites[END] = d;

		if (nlump->getName() == _T("TX_START"))
			tx[START] = d;

		if (nlump->getName() == _T("TX_END"))
			tx[END] = d;

		directory.push_back(nlump);
	}

	// Read wad data into memory
	if (load_data)
	{
		for (DWORD d = 0; d < numLumps(); d++)
			directory[d]->loadStream(fp);
	}

	if (determine_type_load)
	{
		for (DWORD d = 0; d < numLumps(); d++)
			directory[d]->determineType();
	}

	fclose(fp);

	findMaps();

	zip = false;
	changed = false;
	return true;
}

void Wad::addDir(string path)
{
	for (int a = 0; a < dirs.size(); a++)
	{
		if (dirs[a] == path)
			return;
	}

	dirs.push_back(path);
}

bool Wad::openZip(string filename, bool load_data)
{
	path = filename;
	this->zip = true;

	wxZipEntry* entry;

	wxFFileInputStream in(filename);
	if (!in.IsOk())
		return false;

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		wxMessageBox(_T("Invalid zip file!"));
		return false;
	}

	int entry_index = 0;
	entry = zip.GetNextEntry();
	while (entry)
	{
		if (!entry->IsDir())
		{
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create lump
			Lump *nlump = new Lump(entry_index, entry->GetSize(), fn.GetFullName(), this);

			// Load data
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			nlump->loadData(buffer, entry->GetSize());
			delete[] buffer;

			// Get directory info
			wxArrayString dirs = fn.GetDirs();
			for (int a = 0; a < dirs.size(); a++)
				nlump->addDir(dirs[a]);

			if (dirs.size() > 0)
			{
				string dir = dirs[0] + _T("/");
				addDir(dir);
				for (int a = 1; a < dirs.size(); a++)
				{
					dir += dirs[a] + _T("/");
					addDir(dir);
				}
			}

			// Add to directory
			nlump->setOffset(entry_index);
			nlump->setChanged(0);
			directory.push_back(nlump);

			// Determine type
			if (determine_type_load)
				nlump->determineType();
		}
		else
			addDir(entry->GetName(wxPATH_UNIX));

		entry = zip.GetNextEntry();
		entry_index++;
	}

	for (int a = 0; a < this->dirs.size(); a++)
	{
		wxFileName fn(this->dirs[a], wxPATH_UNIX);
		wxArrayString dirs2 = fn.GetDirs();

		//Lump* lump = addLump(this->dirs[a], 0);
		Lump* lump = addLump(dirs2[fn.GetDirCount()-1] + _T("/"), 0);

		for (int a = 0; a < dirs2.size()-1; a++)
			lump->addDir(dirs2[a]);

		lump->setType(LUMP_FOLDER);
	}

	findMaps();

	changed = false;
	return true;
}

// Wad::dump_directory: Writes all directory entries to the logfile
// ----------------------------------------------------------------
void Wad::dumpDirectory()
{
	for (DWORD l = 0; l < numLumps(); l++)
		wxLogMessage(_T("%d: %s (%db at %d)\n"), l, directory[l]->getName(), directory[l]->getSize(), directory[l]->getOffset());
}

// Wad::get_lump_index: Returns the index of the first lump with the specified name
//                      Returns -1 if no matching lump is found
// --------------------------------------------------------------------------------
long Wad::getLumpIndex(string name, DWORD offset)
{
	if (numLumps() == 0 || offset > numLumps())
		return -1;

	for (DWORD l = offset; l < numLumps(); l++)
	{
		if (directory[l]->getName().CmpNoCase(name) == 0)
			return l;
	}

	return -1;
}

// Wad::get_lump: Returns a pointer to the first matching lump from offset
// -----------------------------------------------------------------------
Lump* Wad::getLump(string name, DWORD offset)
{
	for (DWORD l = offset; l < numLumps(); l++)
	{
		if (directory[l]->getName(false).CmpNoCase(name) == 0)
			return directory[l];
	}

	return NULL;
}

// Wad::add_lump: Adds a new 0-sized lump before <index>
// -----------------------------------------------------
Lump* Wad::addLump(string name, long index)
{
	Lump* nlump = new Lump(-1, 0, name, this);

	if (index < 0 || index >= numLumps())
		directory.push_back(nlump);
	else
		directory.insert(directory.begin() + index, nlump);

	changed = true;
	return nlump;
}

// Wad::replace_lump: Replaces lump data with new data
// ---------------------------------------------------
void Wad::replaceLump(string name, DWORD new_size, BYTE *data, DWORD offset)
{
	int index = getLumpIndex(name, offset);

	if (index > -1)
	{
		directory[index]->reSize(new_size);
		memcpy(directory[index]->getData(), data, new_size);
	}

	changed = true;
}

// Wad::delete_lump: Deletes a lump
// --------------------------------
void Wad::deleteLump(string name, DWORD offset)
{
	long index = getLumpIndex(name, offset);
	deleteLump(index);
}

// Wad::delete_lump: Deletes a lump
// --------------------------------
void Wad::deleteLump(long index, bool delfolder)
{
	if (index < 0 || index > numLumps())
		return;

	Lump *lump = lumpAt(index);
	if (lump->getType() == LUMP_FOLDER && delfolder)
	{
		vector<string> dirs;
		for (int a = 0; a < lump->dirLevel(); a++)
			dirs.push_back(lump->getDir(a));

		string ldir = lump->getName(false);
		ldir.RemoveLast();
		dirs.push_back(ldir);

		for (int a = 0; a < numLumps(); a++)
		{
			if (lumpAt(a)->dirLevel() < dirs.size())
				continue;

			bool in = true;
			for (int d = 0; d < dirs.size(); d++)
			{
				if (dirs[d] != lumpAt(a)->getDir(d))
				{
					in = false;
					break;
				}
			}

			if (in)
			{
				deleteLump(a, false);
				a--;
			}
		}

		index = lumpIndex(lump);
	}

	delete lump;
	directory.erase(directory.begin() + index);

	changed = true;
}

// copy_file: Creates a copy of a file
// -----------------------------------
void copy_file(string input, string output)
{
	FILE *in = fopen(chr(input), "rb");
	if (in)
	{
		FILE *out = fopen(chr(output), "wb");
		while (!feof(in))
		{
			BYTE b = 0;
			fread(&b, 1, 1, in);
			fwrite(&b, 1, 1, out);
		}
		fclose(out);
		fclose(in);
	}
}

// Wad::save: Saves a wad file to disk (backs up the file, then overwrites it)
// ---------------------------------------------------------------------------
void Wad::save(string newname)
{
	if (locked)
	{
		wxMessageBox(_T("Saving to an IWAD is disallowed. Use 'Save As' instead."), _T("Not Allowed"));
		return;
	}

	if (zip)
	{
		saveZip(newname);
		return;
	}

	if (newname == _T(""))
		newname = path;

	if (!parent && wxFileName::FileExists(newname))
	{
		string bakfile = newname + _T(".bak");

		// Remove old backup file
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		//copy_file(newname, bakfile);
		wxCopyFile(newname, bakfile);
	}

	// Determine directory offset & individual lump offsets
	dir_offset = 12;

	for (DWORD l = 0; l < numLumps(); l++)
	{
		directory[l]->setOffset(dir_offset);
		dir_offset += directory[l]->getSize();
	}

	// Open wadfile for writing
	FILE *fp = NULL;

	if (!parent)
		fp = fopen(chr(newname), "wb");
	else
		fp = fopen(chr(c_path(_T("slumptemp.wad"), DIR_TMP)), "wb");

	if (!fp)
	{
		wxMessageBox("Unable to open file for saving. Make sure it isn't in use by another program.");
		return;
	}

	// Write the header
	long num_lumps = numLumps();
	fwrite(type, 1, 4, fp);
	fwrite(&num_lumps, 4, 1, fp);
	fwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (DWORD l = 0; l < num_lumps; l++)
		fwrite(directory[l]->getData(), directory[l]->getSize(), 1, fp);

	// Write the directory
	for (DWORD l = 0; l < numLumps(); l++)
	{
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = directory[l]->getOffset();
		long size = directory[l]->getSize();

		for (int c = 0; c < directory[l]->getName().length(); c++)
			name[c] = directory[l]->getName()[c];

		fwrite(&offset, 4, 1, fp);
		fwrite(&size, 4, 1, fp);
		fwrite(name, 1, 8, fp);

		directory[l]->setChanged(0);
	}

	fclose(fp);

	if (parent)
	{
		parent->loadFile(c_path(_T("slumptemp.wad"), DIR_TMP));
		remove(chr(c_path(_T("slumptemp.wad"), DIR_TMP)));
	}

	changed = false;
	path = newname;
}

void Wad::saveZip(string newname)
{
	if (newname == _T(""))
		newname = path;

	string bakfile = newname + _T(".bak");
	if (wxFileName::FileExists(newname))
	{
		// Remove old backup file
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		wxCopyFile(newname, bakfile);
	}
	else
		bakfile = path;

	wxFFileOutputStream out(newname);
	if (!out.IsOk())
	{
		wxMessageBox("Unable to open file for saving. Make sure it isn't in use by another program.");
		return;
	}

	wxZipOutputStream zip(out, 9);

	if (!zip.IsOk())
	{
		wxLogMessage("Zip not ok or something");
		return;
	}

	// Open old zip for copying, if it exists
	wxFFileInputStream in(bakfile);
	wxZipInputStream inzip(in);

	wxZipEntry **entries = new wxZipEntry*[inzip.GetTotalEntries()];
	for (int a = 0; a < inzip.GetTotalEntries(); a++)
		entries[a] = inzip.GetNextEntry();

	for (int a = 0; a < directory.size(); a++)
	{
		if (directory[a]->getType() == LUMP_FOLDER)
			zip.PutNextDirEntry(directory[a]->getName());
		else
		{
			if (!inzip.IsOk() || directory[a]->getOffset() == -1)
			{
				wxZipEntry* entry = new wxZipEntry(directory[a]->getName());
				zip.PutNextEntry(entry);
				zip.Write(directory[a]->getData(), directory[a]->getSize());
			}
			else
			{
				zip.CopyEntry(entries[directory[a]->getOffset()], inzip);
				inzip.Reset();
			}
		}

		directory[a]->setOffset(a);
		directory[a]->setChanged(0);
	}

	delete[] entries;

	zip.Close();
	changed = false;
	path = newname;
}

// Wad::close: Closes the wad file
// -------------------------------
void Wad::close()
{
	if (path == _T(""))
		return;

	available_maps.clear();

	for (int a = 0; a < directory.size(); a++)
		delete directory[a];

	directory.clear();

	path = _T("");
	locked = true;
}

// Wad::needs_save: Returns true if the wad file is changed and needs saving
// -------------------------------------------------------------------------
bool Wad::needsSave()
{
	for (int a = 0; a < numLumps(); a++)
	{
		if (directory[a]->isChanged())
			changed = true;
	}

	return changed;
}

Lump* Wad::lumpAt(int index)
{
	if (index < 0 || index >= numLumps())
		 return NULL;

	return directory[index];
}

long Wad::lumpIndex(Lump* lump)
{
	for (int a = 0; a < directory.size(); a++)
	{
		if (directory[a] == lump)
			return a;
	}

	return -1;
}

Lump* Wad::lastLump()
{
	if (directory.size() == 0)
		return NULL;
	else
		return directory.back();
}

void Wad::swapLumps(long index1, long index2)
{
	if (!lumpAt(index1) || !lumpAt(index2))
		return;

	Lump* temp = lumpAt(index2);
	directory[index2] = lumpAt(index1);
	directory[index1] = temp;

	changed = true;
}

void Wad::swapLumps(Lump* lump1, Lump* lump2)
{
	swapLumps(lumpIndex(lump1), lumpIndex(lump2));
}

void Wad::renameDir(Lump* dirlump, string newname)
{
	for (int a = 0; a < numLumps(); a++)
	{
		Lump* lump = directory[a];

		if (lump == dirlump || lump->dirLevel() <= dirlump->dirLevel())
			continue;

		bool in = true;
		for (int d = 0; d < dirlump->dirLevel(); d++)
		{
			if (lump->getDir(d) != dirlump->getDir(d))
			{
				in = false;
				break;
			}
		}

		string dname = dirlump->getName(false);
		dname.RemoveLast();
		if (in && lump->getDir(dirlump->dirLevel()) == dname)
			lump->renameDir(newname, dirlump->dirLevel());
	}

	dirlump->setName(newname + _T("/"));
}

void Wad::deleteAllLumps()
{
	while (numLumps() > 0)
		deleteLump(0);
}

void Wad::loadLump(Lump* lump)
{
	if (path == _T(""))
		return;

	if (lumpIndex(lump) == -1)
		return;

	if (lump->getOffset() == -1)
		return;

	if (zip)
	{
		wxFFileInputStream in(path);
		if (!in.IsOk())
			return;

		wxZipInputStream zip(in);
		if (!zip.IsOk())
			return;

		wxZipEntry* entry = zip.GetNextEntry();
		for (long a = 0; a < lump->getOffset(); a++)
			entry = zip.GetNextEntry();

		BYTE* buffer = new BYTE[entry->GetSize()];
		zip.Read(buffer, entry->GetSize());
		lump->loadData(buffer, entry->GetSize());
		delete[] buffer;
		return;
	}
	else
	{
		FILE* fp = fopen(chr(path), "rb");

		// Get no. lumps
		DWORD num_lumps;
		fseek(fp, 4, SEEK_SET);
		fread(&num_lumps, 4, 1, fp);

		// Get directory offset
		DWORD dir_offset = 0;
		fread(&dir_offset, 4, 1, fp);

		// Go to directory
		fseek(fp, dir_offset, SEEK_SET);

		// Find lump with the same offset and load it
		for (int a = 0; a < num_lumps; a++)
		{
			char name[8];
			DWORD offset = 0;
			DWORD size = 0;

			fread(&offset, 4, 1, fp);	// Offset
			fread(&size, 4, 1, fp);		// Size
			fread(name, 1, 8, fp);		// Name

			if (offset == lump->getOffset())
			{
				lump->reSize(size);
				fseek(fp, offset, SEEK_SET);
				lump->loadStream(fp);
				break;
			}
		}

		fclose(fp);
	}
}
