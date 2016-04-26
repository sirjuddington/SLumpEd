////////////////////////////////////////////////////////////////////////
// SLumpEd - A doom wad/lump manager                                  //
// By Simon Judd, 2006-07                                             //
// ------------------------------------------------------------------ //
// lump.cpp - Lump class functions                                    //
////////////////////////////////////////////////////////////////////////

// INCLUDES ////////////////////////////////////////////////////////////
#include "main.h"
#include "FreeImage.h"

#include <wx/filename.h>

CVAR(Bool, lump_determine_gfx, true, CVAR_SAVE)

Lump::Lump(long offset, long size, string name, Wad* parent)
{
	this->offset = offset;
	this->size = size;
	this->name = name;
	this->parent = parent;

	changed = 0;
	type = LUMP_UNKNOWN;
	data = NULL;
	loaded = false;

	if (size > 0)
	{
		data = new BYTE[size];
		memset(data, 0, size);
	}
}

Lump::~Lump()
{
	if (data)
		delete[] data;
}

BYTE* Lump::getData()
{
	if (loaded)
		return data;

	if (size > 0 && parent)
	{
		FILE* fp = fopen(parent->path.c_str(), "rb");
		fseek(fp, offset, SEEK_SET);
		fread(data, size, 1, fp);
		fclose(fp);

		loaded = true;
	}

	return data;
}

string Lump::getName(bool full, bool ext)
{
	if (parent->zip)
	{
		string ret;

		if (full)
			ret += getFullDir();

		if (ext)
			ret += name;
		else
		{
			wxFileName fn(name);
			ret += fn.GetName();
		}

		return ret;
	}
	else
		return name;
}

void Lump::setOffset(DWORD o)
{
	offset = o;
	setChanged(1);
}

void Lump::reSize(DWORD s)
{
	if (data)
	{
		delete[] data;
		data = NULL;
		loaded = false;
	}

	size = s;

	if (size > 0)
	{
		data = new BYTE[size];
		memset(data, 0, size);
	}

	setChanged(1);
}

void Lump::setName(string n)
{
	name = n;
	setChanged(1);
}

void Lump::dumpToFile(string filename)
{
	if (!data)
		return;

	FILE *fp = fopen((char*)filename.c_str(), "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
}

bool Lump::loadFile(string filename)
{
	FILE *fp = fopen((char*)filename.c_str(), "rb");

	if (!fp)
		return false;

	// Get size
	fseek(fp, 0, SEEK_END);
	long filesize = ftell(fp);
	size = filesize;
	fseek(fp, 0, SEEK_SET);

	reSize(size);
	fread(data, size, 1, fp);
	fclose(fp);

	loaded = true;
	setChanged(1);
	
	if (parent->zip)
		offset = -1;

	return true;
}

void Lump::loadStream(FILE *fp)
{
	loaded = true;

	if (size == 0)
		return;

	fseek(fp, offset, SEEK_SET);
	fread(data, size, 1, fp);

	if (parent->zip)
		offset = -1;
}

void Lump::loadData(BYTE* buf, DWORD size)
{
	reSize(size);
	memcpy(data, buf, size);
	loaded = true;
	setChanged(1);

	if (parent->zip)
		offset = -1;
}

string Lump::getEx(string key)
{
	for (int a = 0; a < ex_info.size(); a++)
	{
		if (ex_info[a].key == key)
			return ex_info[a].value;
	}

	return _T("");
}

void Lump::setEx(string key, string val)
{
	for (int a = 0; a < ex_info.size(); a++)
	{
		if (ex_info[a].key == key)
		{
			ex_info[a].value = val;
			return;
		}
	}

	key_value_t kv;
	kv.key = key;
	kv.value = val;

	ex_info.push_back(kv);
}

void Lump::determineType(bool force)
{
	if (!force && type != LUMP_UNKNOWN)
		return;

	if (type == LUMP_FOLDER)
		return;

	setEx(_T("LumpType"), _T(""));

	if (parent)
	{
		if (parent->zip)
		{
			wxFileName fn(getName(), wxPATH_UNIX);

			if (fn.GetExt().CmpNoCase(_T("txt")) == 0)
				type = LUMP_TEXT;

			if (fn.GetExt().CmpNoCase(_T("png")) == 0)
				type = LUMP_PNG;

			if (fn.GetExt().CmpNoCase(_T("wad")) == 0)
				type = LUMP_WAD;

			if (fn.GetExt().CmpNoCase(_T("acs")) == 0)
			{
				type = LUMP_TEXT;
				setEx(_T("TextFormat"), _T("SCRIPTS"));
			}

			if (fn.GetName().CmpNoCase(_T("animdefs")) == 0)
				setEx(_T("TextFormat"), _T("ANIMDEFS"));

			if (fn.GetName().CmpNoCase(_T("sndinfo")) == 0)
				setEx(_T("TextFormat"), _T("SNDINFO"));

			if (fn.GetName().CmpNoCase(_T("decorate")) == 0)
				setEx(_T("TextFormat"), _T("DECORATE"));

			if (fn.GetName().CmpNoCase(_T("language")) == 0)
				type = LUMP_TEXT;

			if (directory.size() > 0)
			{
				if (directory[0].CmpNoCase(_T("patches")) == 0 && type == LUMP_UNKNOWN)
					type = LUMP_PATCH;

				if (directory[0].CmpNoCase(_T("sprites")) == 0 && type == LUMP_UNKNOWN)
					type = LUMP_SPRITE;

				if (directory[0].CmpNoCase(_T("flats")) == 0 && type == LUMP_UNKNOWN)
					type = LUMP_FLAT;

				if (directory[0].CmpNoCase(_T("sounds")) == 0 && type == LUMP_UNKNOWN)
					type = LUMP_SOUND;

				if (directory[0].CmpNoCase(_T("actors")) == 0)
					setEx(_T("TextFormat"), _T("DECORATE"));

				if (directory[0].CmpNoCase(_T("mapinfo")) == 0)
					setEx(_T("TextFormat"), _T("MAPINFO"));
			}
		}
	}

	// Marker
	if (size == 0 && type != LUMP_MAP)
	{
		type = LUMP_MARKER;
		return;
	}

	// PNG
	if (size > 8)
	{
		if (data[0] == 137 && data[1] == 80 &&
			data[2] == 78 && data[3] == 71 &&
			data[4] == 13 && data[5] == 10 &&
			data[6] == 26 && data[7] == 10)
		{
			type = LUMP_PNG;
			return;
		}
	}

	// BMP
	if (size > 14)
	{
		if (data[0] == 'B' && data[1] == 'M' &&
			data[6] == 0 && data[7] == 0 && data[8] == 0 && data[9] == 0)
		{
			type = LUMP_IMAGE;
			setEx(_T("LumpType"), _T("BMP Image"));
		}
	}

	// JPEG
	if (size > 128)
	{
		if ((data[6] == 'J' && data[7] == 'F' && data[8] == 'I' && data[9] == 'F') ||
			(data[6] == 'E' && data[7] == 'x' && data[8] == 'i' && data[9] == 'f'))
		{
			if (data[0] == 255 && data[1] == 216 && data[2] == 255)
			{
				type = LUMP_IMAGE;
				setEx(_T("LumpType"), _T("JPEG Image"));
				gfx_checked = true;
			}
		}
	}

	// TEXTUREx
	if (name == _T("TEXTURE1") || name == _T("TEXTURE2"))
	{
		type = LUMP_TEXTURES;
		return;
	}

	// PNames
	if (name == _T("PNAMES"))
	{
		type = LUMP_PNAMES;
		return;
	}

	// THINGS
	if (name == _T("THINGS"))
	{
		if (size % 10 == 0)
		{
			type = LUMP_THINGS;
			return;
		}
	}

	// LINEDEFS
	if (name == _T("LINEDEFS"))
	{
		if (size % 14 == 0 || size % 16 == 0)
		{
			type = LUMP_LINEDEFS;
			return;
		}
	}

	// SIDEDEFS
	if (name == _T("SIDEDEFS"))
	{
		if (size % 30 == 0)
		{
			type = LUMP_SIDEDEFS;
			return;
		}
	}

	// SECTORS
	if (name == _T("SECTORS"))
	{
		if (size % 26 == 0)
		{
			type = LUMP_SECTORS;
			return;
		}
	}

	// VERTEXES
	if (name == _T("VERTEXES"))
	{
		if (size % 4 == 0)
		{
			type = LUMP_VERTEXES;
			return;
		}
	}

	// NODES
	if (name == _T("NODES"))
	{
		type = LUMP_NODES;
		return;
	}

	// SSECTORS
	if (name == _T("SSECTORS"))
	{
		type = LUMP_SSECTS;
		return;
	}

	// SEGS
	if (name == _T("SEGS"))
	{
		type = LUMP_SEGS;
		return;
	}

	// REJECT
	if (name == _T("REJECT"))
	{
		type = LUMP_REJECT;
		return;
	}

	// BLOCKMAP
	if (name == _T("BLOCKMAP"))
	{
		type = LUMP_BLOCKMAP;
		return;
	}

	// BEHAVIOR
	if (name == _T("BEHAVIOR"))
	{
		type = LUMP_BEHAVIOR;
		return;
	}
	

	// PLAYPAL
	if (name == _T("PLAYPAL"))
	{
		type = LUMP_PLAYPAL;
		return;
	}

	// COLORMAP
	if (name == _T("COLORMAP"))
	{
		type = LUMP_COLORMAP;
		return;
	}

	// WAD
	if (size >= 12 && type == LUMP_UNKNOWN)
	{
		if (data[1] == 'W' && data[2] == 'A' && data[3] == 'D' &&
			(data[0] == 'P' || data[0] == 'I'))
		{
			type = LUMP_WAD;
			return;

			/*
			short dir_offset = 0;
			short numlumps = 0;
			memcpy(&numlumps, data + 4, 4);
			memcpy(&dir_offset, data + 8, 4);

			if ((short)(data + dir_offset) >= 0 && (short)(data + dir_offset) <= size)
			{
				type = LUMP_WAD;
				return;
			}
			*/
		}
	}

	// MUS
	if (size > 16)
	{
		if (data[0] == 'M' && data[1] == 'U' && data[2] == 'S' && data[3] == 0x1A)
		{
			type = LUMP_MUS;
			return;
		}
	}

	// MIDI
	if (size > 16)
	{
		if (data[0] == 'M' && data[1] == 'T' && data[2] == 'h' && data[3] == 'd')
		{
			type = LUMP_MIDI;
			return;
		}
	}

	// IT Module
	if (size > 32)
	{
		if (data[0] == 'I' && data[1] == 'M' && data[2] == 'P' && data[3] == 'M')
		{
			type = LUMP_MOD;
			setEx(_T("LumpType"), _T("Music (IT Module)"));
			return;
		}
	}

	// XM Module
	if (size > 80)
	{
		char temp[17] = "";
		memcpy(temp, data, 17);

		if (strnicmp(temp, "Extended module: ", 17) == 0)
		{
			if (data[37] == 0x1a)
			{
				type = LUMP_MOD;
				setEx(_T("LumpType"), _T("Music (XM Module)"));
				return;
			}
		}
	}

	// S3M Module
	if (size > 60)
	{
		if (data[44] == 'S' && data[45] == 'C' && data[46] == 'R' && data[47] == 'M')
		{
			type = LUMP_MOD;
			setEx(_T("LumpType"), _T("Music (S3M Module)"));
			return;
		}
	}

	// MOD Module
	if (size > 1084)
	{
		if (data[950] >= 1 && data[950] <= 128 && data[951] == 127)
		{
			if ((data[1080] == 'M' && data[1081] == '.' && data[1082] == 'K' && data[1083] == '.') ||
				(data[1080] == 'M' && data[1081] == '!' && data[1082] == 'K' && data[1083] == '!') ||
				(data[1080] == 'F' && data[1081] == 'L' && data[1082] == 'T' && data[1083] == '4') ||
				(data[1080] == 'F' && data[1081] == 'L' && data[1082] == 'T' && data[1083] == '8') ||
				(data[1081] == 'C' && data[1082] == 'H' && data[1083] == 'N'))
			{
				type = LUMP_MOD;
				setEx(_T("LumpType"), _T("Music (MOD Module)"));
			}
		}
	}

	// Doom Sound
	if (size > 8)
	{
		if ((WORD)data[0] == 3 && (WORD)data[6] == 0 && (WORD)data[4] <= size - 8)
		{
			type = LUMP_SOUND;

			if (name.Left(2) == _T("DS"))
				return;
		}
	}

	// WAV Sound
	if (size > 8)
	{
		if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F')
		{
			type = LUMP_WAV;
		}
	}

	// Patch
	if (parent->lumpIndex(this) > parent->patches[START] && parent->lumpIndex(this) < parent->patches[END])
	{
		type = LUMP_PATCH;
		return;
	}

	// Flat
	if (parent->lumpIndex(this) > parent->flats[START] && parent->lumpIndex(this) < parent->flats[END])
	{
		if (size == 64*64 || size == 64*128)
		{
			type = LUMP_FLAT;
			return;
		}
	}

	// Sprite
	if (parent->lumpIndex(this) > parent->sprites[START] && parent->lumpIndex(this) < parent->sprites[END])
	{
		type = LUMP_SPRITE;
		return;
	}

	// Misc Gfx
	if ((name == _T("TITLE") || name == _T("HELP1") ||
		name == _T("HELP2") || name == _T("CREDIT") ||
		name == _T("FINALE1") || name == _T("FINALE2") ||
		name == _T("FINALE3") || name == _T("INTERPIC")) &&
		size == 320*200)
	{
		type = LUMP_GFX2;
		return;
	}

	// Script
	if (name == _T("SCRIPTS"))
		type = LUMP_TEXT;

	// Detect doom patch format
	if (size > sizeof(patch_header_t) && (type == LUMP_UNKNOWN || type == LUMP_SOUND))
	{
		patch_header_t *header = (patch_header_t *)data;

		if (header->height > 0 && header->height < 4096 &&
			header->width > 0 && header->width < 4096 &&
			header->top > -2000 && header->top < 2000 &&
			header->left > -2000 && header->left < 2000)
		{
			long *col_offsets = (long *)((BYTE *)data + sizeof(patch_header_t));
			
			if (size < sizeof(patch_header_t) + (header->width * sizeof(long)))
			{
				//wxLogMessage("lump %s not a patch, col_offsets error 1", lump->Name().c_str());
				return;
			}

			for (int a = 0; a < header->width; a++)
			{
				if (col_offsets[a] > size || col_offsets[a] < 0)
				{
					//wxLogMessage("lump %s not a patch, col_offsets error 2", lump->Name().c_str());
					return;
				}
			}

			type = LUMP_GFX;
		}
	}

	checkGfxFormat();
}

void Lump::checkGfxFormat()
{
	if (gfx_checked)
		return;

	if (type == LUMP_FLAT)
		return;

	// Detect doom patch format
	if (size > sizeof(patch_header_t))
	{
		patch_header_t *header = (patch_header_t *)data;

		if (header->height > 0 && header->height < 512 &&
			header->width > 0 && header->width < 4096 &&
			header->top > -2000 && header->top < 2000 &&
			header->left > -2000 && header->left < 2000)
		{
			bool isgfx = true;
			long *col_offsets = (long *)((BYTE *)data + sizeof(patch_header_t));

			if (size < sizeof(patch_header_t) + (header->width * sizeof(long)))
			{
				//wxLogMessage("lump %s not a patch, col_offsets error 1", lump->Name().c_str());
				isgfx = false;
			}

			for (int a = 0; a < header->width; a++)
			{
				if (col_offsets[a] > size || col_offsets[a] < 0)
				{
					//wxLogMessage("lump %s not a patch, col_offsets error 2", lump->Name().c_str());
					isgfx = false;
				}
			}

			if (type == LUMP_UNKNOWN && isgfx)
			{
				type = LUMP_GFX;
				gfx_checked = true;
				return;
			}
		}
	}

	// Other Gfx format (bmp, tga, etc)
	if (size > 0 && lump_determine_gfx)
	{
		FIMEMORY *mem = FreeImage_OpenMemory(getData(), size);
		FREE_IMAGE_FORMAT fmt = FreeImage_GetFileTypeFromMemory(mem, size);

		if (fmt != FIF_UNKNOWN)
		{
			if (fmt == FIF_PNG)
			{
				type = LUMP_PNG;
				return;
			}

			string format = FreeImage_GetFIFDescription(fmt);
			setEx(_T("LumpType"), format);

			type = LUMP_IMAGE;
			gfx_checked = true;
		}
	}
}

string Lump::getTypeString()
{
	string lump_type = getEx(_T("LumpType"));

	if (lump_type != _T(""))
		return lump_type;

	if (type == LUMP_MARKER) return _T("Marker");
	if (type == LUMP_TEXT) return _T("Text");

	if (type == LUMP_PATCH) return _T("Patch");
	if (type == LUMP_SPRITE) return _T("Sprite");
	if (type == LUMP_FLAT) return _T("Flat");
	if (type == LUMP_GFX) return _T("Graphic");
	if (type == LUMP_GFX2) return _T("Graphic");
	if (type == LUMP_PNG) return _T("PNG");
	if (type == LUMP_IMAGE) return _T("Image");

	if (type == LUMP_SOUND) return _T("Sound");
	if (type == LUMP_WAV) return _T("WAV Sound");
	if (type == LUMP_MP3) return _T("MP3 Audio");
	if (type == LUMP_FLAC) return _T("FLAC Audio");
	if (type == LUMP_MUS) return _T("Music (MUS)");
	if (type == LUMP_MIDI) return _T("Music (MIDI)");
	if (type == LUMP_MOD) return _T("Music (Module)");

	if (type == LUMP_TEXTURES) return _T("TEXTUREx");
	if (type == LUMP_PNAMES) return _T("PNames");

	if (type == LUMP_MAP) return _T("Map");
	if (type == LUMP_LINEDEFS) return _T("Map Linedefs");
	if (type == LUMP_SIDEDEFS) return _T("Map Sidedefs");
	if (type == LUMP_VERTEXES) return _T("Map Vertices");
	if (type == LUMP_SECTORS) return _T("Map Sectors");
	if (type == LUMP_THINGS) return _T("Map Things");
	if (type == LUMP_NODES) return _T("Map Nodes");
	if (type == LUMP_SEGS) return _T("Map Segs");
	if (type == LUMP_SSECTS) return _T("Map Ssectors");
	if (type == LUMP_REJECT) return _T("Reject Table");
	if (type == LUMP_BLOCKMAP) return _T("Blockmap");
	if (type == LUMP_BEHAVIOR) return _T("Compiled ACS");

	if (type == LUMP_PLAYPAL) return _T("Palette");
	if (type == LUMP_COLORMAP) return _T("Colormap");

	if (type == LUMP_WAD) return _T("Wad File");

	if (type == LUMP_FOLDER) return _T("Dir");

	return _T("Unknown");
}

void Lump::addDir(string name, int index)
{
	if (index == -1)
		directory.push_back(name);
	else
		directory.insert(directory.begin() + index, name);
}

string Lump::getDir(int index)
{
	if (index < 0 || index > directory.size())
		return _T("");
	else
		return directory[index];
}

void Lump::renameDir(string nname, int index)
{
	if (index >= 0 && index < directory.size())
		directory[index] = nname;
}

string Lump::getFullDir()
{
	if (directory.size() == 0)
		return _T("");

	string ret;
	for (int a = 0; a < directory.size(); a++)
	{
		ret += directory[a];
		ret += _T("/");
	}

	return ret;
}

void Lump::setChanged(int c, bool offset)
{
	if (c == 1 && changed == 2)
		return;
	else
	{
		changed = c;

		// If the lump is set to 'changed' and it's parent is a zip, reset the offset
		// so that the lump will actually be saved when the zip is
		if (c == 1 && parent->zip)
			offset = -1;
	}
}
