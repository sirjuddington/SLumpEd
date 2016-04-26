////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// wad_panel.cpp - WadPanel class functions                           //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "wad_panel.h"
#include "main_window.h"
#include "gfx_area.h"
#include "tex_area.h"
#include "map_area.h"
#include "text_area.h"
#include "hex_area.h"
#include "media_area.h"
#include "data_area.h"
#include "misc_dialog.h"

#include <wx/filename.h>

vector<WadPanel*> open_wad_panels;

CVAR(Bool, col_size, true, CVAR_SAVE)
CVAR(Bool, col_type, true, CVAR_SAVE)
CVAR(String, path_acc, "", CVAR_SAVE)
CVAR(Bool, keep_library_scripts, true, CVAR_SAVE)
CVAR(Bool, zip_askopenwad, true, CVAR_SAVE)
CVAR(Bool, force_uppercase, false, CVAR_SAVE)
CVAR(Bool, colour_lumps, true, CVAR_SAVE)

int type_col = 0;
int size_col = 0;
int cur_index = -1;

extern MainWindow *main_window;
extern Wad iwad, clipboard;
extern wxColour* palette;

LumpList::LumpList(WadPanel *parent, int id)
:	wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES/*|wxLC_EDIT_LABELS*/)
{
	this->parent = parent;
}

LumpList::~LumpList()
{
}

void LumpList::updateEntry(int index)
{
	if (index < 0 || index >= GetItemCount())
		return;

	Lump* lump = (Lump*)GetItemData(index);

	if (!lump)
		return;

	wxListItem li;
	li.SetId(index);
	li.SetText(lump->getName(false));
	SetItem(li);

	if (col_size)
	{
		li.SetText(wxString::Format(_T("%d"), lump->getSize()));
		li.SetColumn(size_col);
		SetItem(li);
	}

	if (col_type)
	{
		li.SetText(lump->getTypeString());
		li.SetColumn(type_col);
		SetItem(li);
	}

	SetItemImage(index, lump->getType());

	if (colour_lumps)
	{
		SetItemBackgroundColour(index, wxColour(255, 255, 255));

		if (lump->isNew())
			SetItemBackgroundColour(index, wxColour(220, 220, 255));
		else if (lump->isChanged())
			SetItemBackgroundColour(index, wxColour(255, 220, 220));
	}
}

BEGIN_EVENT_TABLE(LumpList, wxListCtrl)
	EVT_KEY_DOWN(LumpList::onKeyDown)
END_EVENT_TABLE()

void LumpList::onKeyDown(wxKeyEvent &event)
{
	int key = event.GetKeyCode();

	if (key == WXK_DELETE)
		parent->deleteLump();

	if (key == WXK_INSERT)
		parent->newLump();

	if (key == WXK_F2)
		parent->renameLump();

	if (event.ControlDown())
	{
		if (key == 'U')
			parent->moveLump(true);

		if (key == 'D')
			parent->moveLump(false);

		if (key == 'I')
			parent->importLump();

		if (key == 'E')
			parent->exportLump();

		if (key == 'W')
			parent->exportLumpToWad();

		if (key == 'C')
			parent->copyLump();

		if (key == 'V')
			parent->pasteLump();

		if (key == 'X')
			parent->cutLump();

		if (key == 'R')
		{
			if (event.AltDown())
				parent->reloadLump();
			else
				parent->renameLump();
		}
	}

	event.Skip();
}

WadPanel::WadPanel(wxWindow *parent, string wadpath, bool zip)
:	wxPanel(parent, -1)
{
	// Load the wad file
	opened_ok = false;
	open_wad_panels.push_back(this);

	if (wadpath != _T(""))
	{
		string ext = wadpath.Right(3);

		if (ext.CmpNoCase(_T("wad"))==0)
		{
			// If the wadfile failed to open, return
			if (!wadfile.open(wadpath, true))
				return;
		}

		if (ext.CmpNoCase(_T("zip"))==0 || ext.CmpNoCase(_T("pk3"))==0)
		{
			// If the wadfile failed to open, return
			if (!wadfile.openZip(wadpath, true))
				return;
		}
	}
	else
		wadfile.zip = zip;

	// Wadfile opened ok
	opened_ok = true;
	main_window->enableWadMenus(true, wadfile.zip);

	// Setup palette
	loadPalette();

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Lumps"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);

	wxString filters[] = {
		_T("All Lumps"),
		_T("Text Lumps"),
		_T("Patches"),
		_T("Sprites"),
		_T("Flats"),
		_T("Graphics"),
		_T("Maps"),
		_T("Textures"),
		_T("Sounds"),
		_T("Music"),
	};

	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox2, 0, wxEXPAND|wxALL, 4);

	lump_filter = new wxChoice(this, WP_LUMP_FILTER, wxDefaultPosition, wxDefaultSize, 10, filters);
	lump_filter->Select(0);
	hbox2->Add(new wxStaticText(this, -1, _T("Show:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox2->Add(lump_filter, 1, wxEXPAND);

	lump_list = new LumpList(this, WP_LIST_LUMPS);
	//lump_list = new wxListCtrl(this, WP_LIST_LUMPS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES/*|wxLC_EDIT_LABELS*/);
	lump_list->SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	framesizer->Add(lump_list, 1, wxEXPAND);

	hbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);
	hbox->AddSpacer(4);

	frame = new wxStaticBox(this, -1, _T("Lump Content"));
	lump_area_sizer = new wxStaticBoxSizer(frame, wxVERTICAL);

	lump_area = new LumpArea(this);
	//lump_area_sizer->Add(new wxStaticText(this, -1, _T("Testing etc")), 0, wxEXPAND);
	lump_area_sizer->Add(lump_area, 1, wxEXPAND);
	hbox->Add(lump_area_sizer, 1, wxEXPAND|wxALL, 4);

	cur_dir = _T("");

	populateLumpList();

	int width = 0;
	for (int a = 0; a < lump_list->GetColumnCount(); a++)
		width += lump_list->GetColumnWidth(a);

	lump_list->SetSizeHints(width + 20, -1, width + 20, -1);

	image_list = new wxImageList(16, 16, false, 0);

	image_list->Add(wxBitmap(wxImage(_T("res/icons/unknown.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/marker.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/text.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/patch.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/sprite.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/flat.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/gfx.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/gfx.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/png.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/gfx.png"), wxBITMAP_TYPE_PNG))); // image
	image_list->Add(wxBitmap(wxImage(_T("res/icons/sound.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/sound.png"), wxBITMAP_TYPE_PNG))); // wav
	image_list->Add(wxBitmap(wxImage(_T("res/icons/sound.png"), wxBITMAP_TYPE_PNG))); // mp3
	image_list->Add(wxBitmap(wxImage(_T("res/icons/sound.png"), wxBITMAP_TYPE_PNG))); // flac
	image_list->Add(wxBitmap(wxImage(_T("res/icons/music.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/music.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/music.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/texturex.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/pnames.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/map_data.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/default.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/default.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/wad.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/folder.png"), wxBITMAP_TYPE_PNG)));
	image_list->Add(wxBitmap(wxImage(_T("res/icons/up_folder.png"), wxBITMAP_TYPE_PNG)));

	lump_list->SetImageList(image_list, wxIMAGE_LIST_SMALL);

	updateColumnWidths();

	menu_cols = new wxMenu(_T(""));
	menu_cols->AppendCheckItem(WP_COL_SIZE, _T("Size"));
	menu_cols->AppendCheckItem(WP_COL_TYPE, _T("Type"));
	menu_cols->Check(WP_COL_SIZE, col_size);
	menu_cols->Check(WP_COL_TYPE, col_type);

	Layout();
	parent->Layout();
	Refresh();
	parent->Refresh();
}

WadPanel::~WadPanel()
{
	open_wad_panels.erase(find(open_wad_panels.begin(), open_wad_panels.end(), this));
}

bool WadPanel::lumpIsFiltered(Lump *lump)
{
	int type = lump->getType();

	// Show text lumps only
	if (lump_filter->GetSelection() == 1)
	{
		if (type != LUMP_TEXT)
			return true;
	}

	// Show patches only
	if (lump_filter->GetSelection() == 2)
	{
		if (type != LUMP_PATCH)
			return true;
	}

	// Show sprites only
	if (lump_filter->GetSelection() == 3)
	{
		if (type != LUMP_SPRITE)
			return true;
	}

	// Show flats only
	if (lump_filter->GetSelection() == 4)
	{
		if (type != LUMP_FLAT)
			return true;
	}

	// Show gfx only
	if (lump_filter->GetSelection() == 5)
	{
		if (!(type == LUMP_GFX || type == LUMP_GFX2))
			return true;
	}

	// Show maps only
	if (lump_filter->GetSelection() == 6)
	{
		if (type < LUMP_MAP || type > LUMP_BEHAVIOR)
			return true;
	}

	// Show textures only
	if (lump_filter->GetSelection() == 7)
	{
		if (!(type == LUMP_TEXTURES || type == LUMP_PNAMES))
			return true;
	}

	// Show sounds only
	if (lump_filter->GetSelection() == 8)
	{
		if (!(type == LUMP_SOUND || type == LUMP_WAV || type == LUMP_FLAC))
			return true;
	}

	// Show music only
	if (lump_filter->GetSelection() == 9)
	{
		if (!(type == LUMP_MIDI || type == LUMP_MUS || type == LUMP_MOD))
			return true;
	}

	return false;
}

void WadPanel::populateLumpList()
{
	// Delete stuff
	lump_list->ClearAll();

	lump_list->InsertColumn(0, _T("Name"));

	// Add/setup columns
	if (col_size)
	{
		size_col = 1;
		lump_list->InsertColumn(size_col, _T("Size"), wxLIST_FORMAT_RIGHT);
	}
	else
		size_col = 0;

	if (col_type)
	{
		type_col = size_col+1;
		lump_list->InsertColumn(type_col, _T("Type"), wxLIST_FORMAT_RIGHT);
	}
	else
		type_col = 0;

	// Add lumps in current dir
	int index = 0;
	for (int a = 0; a < wadfile.numLumps(); a++)
	{
		if (wadfile.lumpAt(a)->getFullDir() != cur_dir)
			continue;

		if (lumpIsFiltered(wadfile.lumpAt(a)))
			continue;

		wxListItem li;
		int i = index;

		if (wadfile.lumpAt(a)->getType() == LUMP_FOLDER)
		{
			i = 0;
			gotoNonFolder(i);
		}

		li.SetId(i);
		li.SetData(wadfile.lumpAt(a));
		lump_list->InsertItem(li);
		lump_list->updateEntry(i);

		/*
		li.SetId(i);
		li.SetText(wadfile.lumpAt(a)->getName(false));
		li.SetColumn(0);
		li.SetData(wadfile.lumpAt(a));
		lump_list->InsertItem(li);

		if (col_size)
		{
			li.SetText(wxString::Format(_T("%d"), wadfile.lumpAt(a)->getSize()));
			li.SetColumn(size_col);
			lump_list->SetItem(li);
		}

		if (col_type)
		{
			li.SetText(wadfile.lumpAt(a)->getTypeString());
			li.SetColumn(type_col);
			lump_list->SetItem(li);
		}

		lump_list->SetItemImage(i, wadfile.lumpAt(a)->getType());
		*/

		index++;
	}

	// Add '..' directory item
	if (cur_dir != _T(""))
	{
		wxListItem li;
		li.SetText(_T(".."));
		li.SetId(0);
		lump_list->InsertItem(li);
		lump_list->SetItemImage(0, image_list->GetImageCount()-1);
	}

	updateColumnWidths();
	updateSelection();
}

void WadPanel::updateColumnWidths()
{
	// Autosize columns
	lump_list->SetColumnWidth(0, wxLIST_AUTOSIZE);

	if (col_size)
		lump_list->SetColumnWidth(size_col, wxLIST_AUTOSIZE);

	if (col_type)
		lump_list->SetColumnWidth(type_col, wxLIST_AUTOSIZE);

	// Resize to total column width
	int width = 0;
	for (int a = 0; a < lump_list->GetColumnCount(); a++)
		width += lump_list->GetColumnWidth(a);

	lump_list->SetSizeHints(width + 20, -1, width + 20, -1);

	Layout();
}

string WadPanel::getFilename()
{
	if (wadfile.path == _T(""))
		return _T("UNSAVED");
	else
		return wadfile.path;
}

void WadPanel::loadPalette()
{
	Lump* lump = wadfile.getLump("PLAYPAL", 0);

	if (!lump)
		lump = iwad.getLump("PLAYPAL", 0);

	palette = new wxColour[256];

	for (int a = 0; a < 256; a++)
	{
		if (lump)
			palette[a].Set(lump->getData()[a*3], lump->getData()[(a*3)+1], lump->getData()[(a*3)+2]);
		else
			palette[a].Set(a, a, a);
	}
}

bool WadPanel::closeWad()
{
	lump_area->checkSave();

	if (wadfile.needsSave())
	{
		string question;
		if (wadfile.path == _T(""))
			question = _T("Save Changes to the wad?");
		else
			question = wxString::Format(_T("Save changes to \"%s\"?"), wadfile.path);

		int ret = wxMessageBox(question, _T("Close Wad"), wxICON_QUESTION|wxYES_NO|wxCANCEL);

		if (ret == wxCANCEL)
			return false;

		if (ret == wxYES)
			saveWad();
	}

	return true;
}

void WadPanel::saveWad()
{
	if (wadfile.path == _T(""))
		saveWadAs();
	else
		wadfile.save();
}

void WadPanel::saveWadAs()
{
	if (wadfile.parent)
		return;

	wxString formats = _T("Doom Wad File (*.wad)|*.wad");
	wxString deftype = _T("*.wad");

	if (wadfile.zip)
	{
		formats = _T("Zip File (*.zip)|*.zip|PK3 Zip File (*.pk3)|*.pk3");
		deftype = _T("*.zip");
	}

	wxString filename = wxFileSelector(_T("Save Wad"), _T(""), _T(""), deftype, formats, wxSAVE|wxOVERWRITE_PROMPT);

	if (!filename.empty())
	{
		//wadfile.path = filename;
		wadfile.save(filename);
	}
}

void WadPanel::addLumpToList(int index, Lump *lump)
{
	/*
	wxListItem li;
	li.SetId(index);
	li.SetText(lump->getName(false));
	li.SetData(lump);
	lump_list->InsertItem(li);

	if (col_size)
	{
		li.SetText(wxString::Format(_T("%d"), lump->getSize()));
		li.SetColumn(size_col);
		lump_list->SetItem(li);
	}

	if (col_type)
	{
		li.SetText(lump->getTypeString());
		li.SetColumn(type_col);
		lump_list->SetItem(li);
	}

	lump_list->SetItemImage(index, lump->getType());
	updateSelection();
	*/

	wxListItem li;
	li.SetId(index);
	li.SetData(lump);
	lump_list->InsertItem(li);
	lump_list->updateEntry(index);
	updateSelection();
}

void WadPanel::updateSelection()
{
	selection.clear();

	int s = 0;
	for (int a = 0; a < lump_list->GetItemCount(); a++)
	{
		if (lump_list->GetItemState(a, wxLIST_STATE_SELECTED))
		{
			selection.push_back(a);
			s++;
		}

		if (s == lump_list->GetSelectedItemCount())
			return;
	}
}

void WadPanel::updateList(int index)
{
	if (index == -1)
	{
		for (int a = 0; a < lump_list->GetItemCount(); a++)
			lump_list->updateEntry(a);
	}
	else
		lump_list->updateEntry(index);
}

void WadPanel::updateList(Lump* lump)
{
	for (int a = 0; a < lump_list->GetItemCount(); a++)
	{
		Lump* tlump = (Lump*)lump_list->GetItemData(a);

		if (lump == tlump)
		{
			updateList(a);
			return;
		}
	}
}

void WadPanel::newLump()
{
	int index = lump_list->GetItemCount();
	if (selection.size() > 0)
		index = selection.back() + 1;

	gotoNonFolder(index);

	int lindex = wadfile.lumpIndex((Lump*)lump_list->GetItemData(index-1)) + 1;

	wxString name = wxGetTextFromUser(_T("Enter new lump name:"), _T("New Lump"));

	if (wadfile.zip)
	{
		if (name.Find('/') != -1)
		{
			wxLogMessage("Invalid lump name");
			return;
		}
	}
	else
	{
		if (force_uppercase) name.UpperCase();
		name.Truncate(8);
	}

	Lump *nlump = wadfile.addLump(name, lindex);
	nlump->setChanged(2);
	addLumpToList(index, nlump);

	wxFileName fn(cur_dir);
	wxArrayString dirs = fn.GetDirs();
	for (int a = 0; a < dirs.size(); a++)
		nlump->addDir(dirs[a]);

	updateColumnWidths();
}

void WadPanel::newFolder()
{
	if (!wadfile.zip)
		return;

	int index = 0;
	gotoNonFolder(index);

	wxString name = wxGetTextFromUser(_T("Enter new lump name:"), _T("New Lump"));

	if (name.Find('/') != -1)
	{
		wxLogMessage("Invalid lump name");
		return;
	}

	name += _T("/");

	Lump *nlump = wadfile.addLump(name, wadfile.numLumps());
	nlump->setType(LUMP_FOLDER);
	addLumpToList(index, nlump);

	wxFileName fn(cur_dir);
	wxArrayString dirs = fn.GetDirs();
	for (int a = 0; a < dirs.size(); a++)
		nlump->addDir(dirs[a]);

	updateColumnWidths();
}

void WadPanel::newLumpFromFile()
{
	updateSelection();

	int index = lump_list->GetItemCount();
	if (selection.size() > 0)
		index = selection.back() + 1;

	gotoNonFolder(index);

	int lindex = wadfile.lumpIndex((Lump*)lump_list->GetItemData(index-1)) + 1;

	wxFileDialog *OpenDialog = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
		_T("All files (*.*)|*.*"), wxOPEN|wxMULTIPLE|wxFILE_MUST_EXIST, wxDefaultPosition);

	if (OpenDialog->ShowModal() == wxID_OK)
	{
		wxArrayString files;
		OpenDialog->GetPaths(files);

		int start = 0;
		if (files.size() > 1)
			start = 1;

		for (int a = start; a < files.size(); a++)
		{
			wxFileName name(files[a]);
			Lump *nlump;

			if (wadfile.zip)
			{
				nlump = wadfile.addLump(name.GetFullName(), lindex);

				wxFileName fn(cur_dir);
				wxArrayString dirs = fn.GetDirs();
				for (int d = 0; d < dirs.size(); d++)
					nlump->addDir(dirs[d]);
			}
			else
			{
				if (force_uppercase)
					nlump = wadfile.addLump(name.GetName().Truncate(8).Upper(), lindex);
				else
					nlump = wadfile.addLump(name.GetName().Truncate(8), lindex);
			}

			nlump->loadFile(files[a]);
			nlump->determineType();
			nlump->setChanged(2);
			addLumpToList(index, nlump);
			index++;
			lindex++;
		}

		if (start == 1)
		{
			wxFileName name(files[0]);
			Lump *nlump;

			if (wadfile.zip)
			{
				nlump = wadfile.addLump(name.GetFullName(), lindex);

				wxFileName fn(cur_dir);
				wxArrayString dirs = fn.GetDirs();
				for (int d = 0; d < dirs.size(); d++)
					nlump->addDir(dirs[d]);
			}
			else
			{
				if (force_uppercase)
					nlump = wadfile.addLump(name.GetName().Truncate(8).Upper(), lindex);
				else
					nlump = wadfile.addLump(name.GetName().Truncate(8), lindex);
			}

			nlump->loadFile(files[0]);
			nlump->determineType();
			nlump->setChanged(2);
			addLumpToList(index, nlump);
		}
	}

	updateColumnWidths();
}

void WadPanel::deleteLump(bool ignoredir)
{
	int a = 0;
	while (selection.size() > 0)
	{
		Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);

		if (!lump)
			break;

		if (lump->getType() == LUMP_FOLDER)
		{
			if (ignoredir)
				continue;

			if (wxMessageBox(wxString::Format(_T("Are you sure you want to delete the folder %s?\n(All files and subfolders in it will be deleted too)"), lump->getName()), _T("Warning"), wxICON_QUESTION|wxYES_NO) == wxNO)
			{
				lump_list->SetItemState(selection[0], 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
				updateSelection();
				continue;
			}
		}

		wadfile.deleteLump(wadfile.lumpIndex(lump));
		lump_list->DeleteItem(selection[a]);
		updateSelection();
	}

	updateColumnWidths();
}

void WadPanel::renameLump()
{
	if (selection.size() > 0)
	{
		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);

			if (!lump)
				continue;

			string name = wxGetTextFromUser(wxString::Format(_T("Enter new lump name for %s:"), lump->getName()), _T("Rename Lump"), lump->getName(false));

			if (!name.empty())
			{
				if (wadfile.zip)
				{
					if (name.Find('/') != -1)
					{
						wxLogMessage(wxString::Format(_T("Invalid lump name \"%s\""), name));
						continue;
					}
				}
				else
				{
					if (force_uppercase) name.UpperCase();
					name.Truncate(8);
				}

				if (lump->getType() == LUMP_FOLDER)
				{
					wadfile.renameDir(lump, name);
					populateLumpList();
					return;
				}

				lump->setName(name);
				lump_list->SetItemText(selection[a], name);
				lump->determineType(true);

				// This shouldn't be needed but for whatever reason it is
				lump->setOffset(-1);
			}
		}
	}

	updateColumnWidths();
}

void WadPanel::importLump()
{
	if (selection.size() > 0)
	{
		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);

			if (!lump)
				continue;

			string filename = wxFileSelector(wxString::Format(_T("Import %s"), lump->getName()), _T(""), _T(""), _T("*.*"),
												_T("All Files (*.*)|*.*"), wxOPEN|wxFILE_MUST_EXIST);

			if (filename != _T(""))
			{
				lump->loadFile(filename);
				lump->setType(LUMP_UNKNOWN);
				lump->determineType(true);
				updateList(a);
			}
		}
	}

	if (selection.size() == 1)
		openLump(hl_lump);
}

void WadPanel::exportImageLump(Lump* lump)
{
	wxString formats = _T("Raw Lump (*.lmp)|*.lmp|PNG (*.png)|*.png|Windows Bitmap (*.bmp)|*.bmp|All Files (*.*)|*.*");
	wxString deftype = _T("*.lmp");
	wxString fname = lump->getName(false, false);

	if (lump->getType() == LUMP_PNG)
		deftype = _T("*.png");

	wxString filename = wxFileSelector(wxString::Format(_T("Export %s"), lump->getName()),
										_T(""), fname, deftype, formats, wxSAVE|wxOVERWRITE_PROMPT);

	wxString ftype = filename.Right(3).Lower();

	if (filename.empty())
		return;

	if (ftype.CmpNoCase(_T("png")) == 0 && lump->getType() != LUMP_PNG)
	{
		Image img(palette);
		img.loadLump(lump);
		img.savePNG(filename);
		return;
	}

	if (ftype.CmpNoCase(_T("bmp")) == 0)
	{
		Image img(palette);
		img.loadLump(lump);
		img.saveBMP(filename);
		return;
	}

	if (ftype.CmpNoCase(_T("lmp")) == 0)
	{
		Image img(palette);
		img.loadLump(lump);
		img.saveDoomGfx(filename);
		return;
	}

	lump->dumpToFile(filename);
}

void WadPanel::exportLump()
{
	if (selection.size() > 0)
	{
		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);

			if (!lump)
				continue;

			// Process gfx lumps differently
			if (lump->getType() == LUMP_GFX ||
				lump->getType() == LUMP_SPRITE ||
				lump->getType() == LUMP_PATCH ||
				lump->getType() == LUMP_FLAT ||
				lump->getType() == LUMP_PNG ||
				lump->getType() == LUMP_GFX2 ||
				lump->getType() == LUMP_IMAGE)
			{
				exportImageLump(lump);
				continue;
			}

			wxString filename = wxFileSelector(wxString::Format(_T("Export %s"), lump->getName()), _T(""),
												wxString::Format(_T("%s"), lump->getName(false)), _T("*.*"),
												_T("All Files (*.*)|*.*"), wxSAVE|wxOVERWRITE_PROMPT);

			if (!filename.empty())
				lump->dumpToFile(filename);
		}
	}
}

void WadPanel::exportLumpToWad()
{
	if (selection.size() == 0)
		return;

	wxString filename = wxFileSelector(_T("Export to Wad"), _T(""), _T(""), _T("*.wad"), _T("Doom Wad Files (*.wad)|*.wad"),
										wxSAVE|wxOVERWRITE_PROMPT);

	if (filename == getWad()->path)
		return;

	if (!filename.empty())
	{
		Wad newwad;
		newwad.path = filename;
		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);
			if (!lump) continue;
			newwad.addLump(lump->getName(), -1);
			newwad.lastLump()->loadData(lump->getData(), lump->getSize());
		}

		newwad.save();
	}
}

void WadPanel::moveLump(bool up)
{
	updateSelection();

	if (up)
	{
		vector<wxListItem> items;

		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump1 = (Lump*)lump_list->GetItemData(selection[a]);
			Lump* lump2 = (Lump*)lump_list->GetItemData(selection[a]-1);

			if (!lump1 || !lump2)
				continue;

			// Swap lumps in the wad
			wadfile.swapLumps(lump1, lump2);

			// Update the lump list
			lump_list->SetItemData(selection[a], (long)lump2);
			lump_list->SetItemData(selection[a]-1, (long)lump1);
			updateList(selection[a]);
			updateList(selection[a]-1);
		}

		// Modify selection
		for (int a = 0; a < wadfile.numLumps(); a++)
		{
			if (lump_list->GetItemState(a, wxLIST_STATE_SELECTED))
			{
				lump_list->SetItemState(a, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
				if (a > 0)
					lump_list->SetItemState(a-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			}
		}
	}
	else
	{
		for (int a = selection.size() - 1; a >= 0; a--)
		{
			Lump* lump1 = (Lump*)lump_list->GetItemData(selection[a]);
			Lump* lump2 = (Lump*)lump_list->GetItemData(selection[a]+1);

			if (!lump1 || !lump2)
				continue;

			// Swap lumps in the wad
			wadfile.swapLumps(lump1, lump2);

			// Update the lump list
			lump_list->SetItemData(selection[a], (long)lump2);
			lump_list->SetItemData(selection[a]+1, (long)lump1);
			updateList(selection[a]);
			updateList(selection[a]+1);
		}

		// Modify selection
		for (int a = wadfile.numLumps() - 1; a >= 0; a--)
		{
			if (lump_list->GetItemState(a, wxLIST_STATE_SELECTED))
			{
				lump_list->SetItemState(a, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
				if (a < wadfile.numLumps() - 1)
					lump_list->SetItemState(a+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			}
		}
	}
}

void WadPanel::copyLump()
{
	if (selection.size() == 0)
		return;

	clipboard.deleteAllLumps();
	for (int a = 0; a < selection.size(); a++)
	{
		Lump *lump = (Lump*)lump_list->GetItemData(selection[a]);

		if (!lump)
			continue;

		if (lump->getType() == LUMP_FOLDER)
			continue;

		Lump *nlump = clipboard.addLump(lump->getName(false), clipboard.numLumps());
		nlump->loadData(lump->getData(), lump->getSize());
	}
}

void WadPanel::pasteLump()
{
	updateSelection();

	int index = lump_list->GetItemCount();
	if (selection.size() > 0)
		index = selection.back() + 1;

	gotoNonFolder(index);

	int lindex = wadfile.lumpIndex((Lump*)lump_list->GetItemData(index-1)) + 1;

	for (int a = 0; a < clipboard.numLumps(); a++)
	{
		Lump *lump = clipboard.lumpAt(a);
		Lump *nlump;

		if (wadfile.zip)
		{
			nlump = wadfile.addLump(lump->getName(false), lindex);

			wxFileName fn(cur_dir);
			wxArrayString dirs = fn.GetDirs();
			for (int d = 0; d < dirs.size(); d++)
				nlump->addDir(dirs[d]);
		}
		else
			nlump = wadfile.addLump(lump->getName(false).Truncate(8).Upper(), lindex);

		nlump->loadData(lump->getData(), lump->getSize());
		nlump->setType(lump->getType());
		addLumpToList(index, nlump);
		index++;
		lindex++;
	}

	updateColumnWidths();
}

void WadPanel::cutLump()
{
	copyLump();
	deleteLump(true);
}

void WadPanel::gotoNonFolder(int &index)
{
	bool done = false;
	while (!done)
	{
		done = true;

		if (index < lump_list->GetItemCount())
		{
			Lump *lump = (Lump*)lump_list->GetItemData(index);

			if (!lump)
			{
				index++;
				done = false;
			}
			else if (lump->getType() == LUMP_FOLDER)
			{
				index++;
				done = false;
			}
		}
	}
}

void WadPanel::openLump(Lump *lump)
{
	if (!lump)
		return;

	lump_area->checkSave();
	lump->determineType();
	updateList(lump);

	if (lump->getType() == LUMP_GFX ||
		lump->getType() == LUMP_SPRITE ||
		lump->getType() == LUMP_PATCH ||
		lump->getType() == LUMP_FLAT ||
		lump->getType() == LUMP_PNG ||
		lump->getType() == LUMP_GFX2 ||
		lump->getType() == LUMP_IMAGE)
	{
		if (lump_area->type != LAREA_GFX)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new GfxLumpArea(this, palette);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	else if (lump->getType() == LUMP_UNKNOWN || lump->getType() == LUMP_TEXT || lump->getType() == LUMP_MARKER)
	{
		if (lump_area->type != LAREA_TEXT)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new TextLumpArea(this);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	else if (lump->getType() == LUMP_TEXTURES)
	{
		if (lump_area->type != LAREA_TEXTURES)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new TextureLumpArea(this, &wadfile, palette);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	else if (lump->getType() == LUMP_MAP)
	{
		if (lump_area->type != LAREA_MAP)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new MapLumpArea(this, &wadfile);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	else if (lump->getType() == LUMP_SOUND ||
			lump->getType() == LUMP_MIDI ||
			lump->getType() == LUMP_MUS ||
			lump->getType() == LUMP_WAV ||
			lump->getType() == LUMP_MOD)
	{
		if (lump_area->type != LAREA_MEDIA)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new MediaLumpArea(this);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	else if (lump->getType() == LUMP_PNAMES)
	{
		if (lump_area->type != LAREA_DATA)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new DataLumpArea(this, &wadfile);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	/*
	else if (lump->getType() == LUMP_UNKNOWN)
	{
		if (lump->getType() != LAREA_HEX)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new HexLumpArea(this);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}
	*/
	else
	{
		if (lump_area->type != LAREA_NONE)
		{
			lump_area_sizer->Detach(lump_area);
			delete(lump_area);
			lump_area = new LumpArea(this);
			lump_area_sizer->Add(lump_area, 1, wxEXPAND);
		}
	}

	lump_area->loadLump(lump);
	main_window->SetStatusText(wxString::Format(_T("Size: %d"), lump->getSize()), 1);
	main_window->SetStatusText(wxString::Format(_T("Offset: %d"), lump->getOffset()), 2);
	lump_area->Show();
	lump_area->Update();
	lump_area_sizer->Layout();

	Layout();
}

void WadPanel::compileScript(Lump* lump, int type)
{
	if (type == SCRIPT_ACS)
	{
		// Check for acc path
		if (path_acc == _T(""))
		{
			wxMessageBox(_T("You must browse for acc.exe"), _T("ACC.exe Path Undefined"));

			wxFileDialog *OpenDialog = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
				_T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxOPEN|wxFILE_MUST_EXIST, wxDefaultPosition);

			if (OpenDialog->ShowModal() == wxID_OK)
			{
				if (OpenDialog->GetPath() != _T(""))
					path_acc = OpenDialog->GetPath();
				else
					return;
			}
		}

		wxFileName fn(path_acc);
		string acs_path = fn.GetPath(true) + lump->getName(false);

		if (acs_path.Right(4).CmpNoCase(_T(".acs")) != 0)
			acs_path += _T(".acs");

		lump->dumpToFile(acs_path);

		// Compile the script
		string temp = _T(path_acc);
		if (wxExecute(wxString::Format(_T("\"%s\" \"%s\""), temp, acs_path), wxEXEC_SYNC) == -1)
		{
			wxMessageBox(_T("Acc(.exe) not found! Please check the path"));
			return;
		}

		// Open the output
		FILE* fp = fopen(chr(wxString::Format(_T("%s.o"), fn.GetPath(true) + lump->getName(false, false))), "rb");

		bool lib = false;

		// If output doesn't exist some kind of error occurred
		if (!fp)
		{
			string err_path = fn.GetPath(true) + _T("acs.err");
			fp = fopen(chr(err_path), "rt");

			if (!fp)
				wxMessageBox(_T("Compile failed"), _T("Error"), wxICON_ERROR);
			else
			{
				// Open a dialog with the contents of acs.err
				fseek(fp, 0, SEEK_END);
				int len = ftell(fp);
				char* text = (char*)malloc(len);
				fseek(fp, 0, SEEK_SET);
				fread(text, 1, len, fp);
				string error = _T(text);
				free(text);

				wxDialog dlg(NULL, -1, _T("ACC Error Message"), wxDefaultPosition, wxDefaultSize);
				wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
				dlg.SetSizer(vbox);
				wxTextCtrl *error_text = new wxTextCtrl(&dlg, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
				error_text->SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
				error_text->SetValue(error);
				vbox->Add(error_text, 1, wxEXPAND|wxALL, 4);
				vbox->Add(dlg.CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

				dlg.ShowModal();

				fclose(fp);

				remove("acs.err");
			}
		}
		else
		{
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			BYTE* data = new BYTE[len];
			fseek(fp, 0, SEEK_SET);
			fread(data, 1, len, fp);
			fclose(fp);

			remove(chr(wxString::Format(_T("%s.o"), fn.GetPath(true) + lump->getName(false, false))));

			//map.behavior->LoadData(data, len);

			// Map script
			if (lump->getName(false, false).CmpNoCase(_T("SCRIPTS")) == 0)
			{
				Lump* script_lump = wadfile.lumpAt(wadfile.lumpIndex(lump) - 1);

				if (script_lump)
				{
					if (script_lump->getName(false, false).CmpNoCase(_T("BEHAVIOR")) == 0)
						script_lump->loadData(data, len);
					else
					{
						script_lump = wadfile.addLump(cur_dir + _T("BEHAVIOR"), wadfile.lumpIndex(lump));
						script_lump->loadData(data, len);
						addLumpToList(wadfile.lumpIndex(lump)-1, script_lump);
					}
				}
				else
				{
					script_lump = wadfile.addLump(cur_dir + _T("BEHAVIOR"), wadfile.lumpIndex(lump));
					script_lump->loadData(data, len);
					addLumpToList(wadfile.lumpIndex(lump)-1, script_lump);
				}
			}
			else // Library
			{
				lib = true;

				if (wadfile.zip)
				{
					string lumpname = lump->getName(false, false) + _T(".o");
					Lump* script_lump = wadfile.getLump(lumpname);

					if (!script_lump)
					{
						// Create 'acs' folder if needed
						if (!wadfile.getLump(_T("acs/")))
						{
							Lump* nlump = wadfile.addLump(_T("acs/"), -1);
							nlump->setType(LUMP_FOLDER);
						}

						script_lump = wadfile.addLump(lumpname, -1);
						script_lump->addDir(_T("acs"));
					}

					script_lump->loadData(data, len);
					populateLumpList();
				}
				else
				{
					int index = wadfile.getLumpIndex(_T("A_START"), 0);

					if (index == -1)
					{
						wadfile.addLump(_T("A_START"), -1);
						addLumpToList(wadfile.numLumps()-1, wadfile.lastLump());
						wadfile.addLump(_T("A_END"), -1);
						addLumpToList(wadfile.numLumps()-1, wadfile.lastLump());

						index = wadfile.numLumps() - 1;
					}

					Lump* script_lump = wadfile.getLump(lump->getName(false), index);

					if (!script_lump || script_lump == lump)
					{
						script_lump = wadfile.addLump(lump->getName(false), wadfile.getLumpIndex(_T("A_END"), 0));
						addLumpToList(wadfile.lumpIndex(script_lump), script_lump);
					}

					script_lump->loadData(data, len);
					updateList(script_lump);
				}
			}

			delete[] data;

			wxMessageBox(_T("Compiled Successfully"));
		}

		if (!(lib && keep_library_scripts))
			remove(chr(acs_path));
	}
}

void WadPanel::modifyGfxOffsets()
{
	// Get all selected lumps
	vector<Lump*> sel_lumps;
	for (int a = 0; a < selection.size(); a++)
	{
		Lump* l = (Lump*)lump_list->GetItemData(selection[a]);
		if (l)
			sel_lumps.push_back(l);
	}

	GfxOffsetDialog dl(this);
	if (dl.ShowModal() != wxID_OK)
		return;

	for (int a = 0; a < sel_lumps.size(); a++)
	{
		Lump* lump = sel_lumps[a];

		if (lump->getType() == LUMP_PNG)
		{
			Image img;
			img.loadLump(lump);

			if (dl.getOption() == 0)
			{
				point2_t offset = dl.getOffset();

				if (dl.relativeOffset())
				{
					offset.x += img.xOff();
					offset.y += img.yOff();
				}

				if (dl.xOffChange())
					img.setXOff(offset.x);

				if (dl.yOffChange())
					img.setYOff(offset.y);
			}
			else
			{
				int w = img.getWidth();
				int h = img.getHeight();

				if (dl.getAlignType() == 0)			// Monster
					img.setOffsets(w*0.5, h-4);
				else if (dl.getAlignType() == 1)	// Projectile
					img.setOffsets(w*0.5, h*0.5);
				else if (dl.getAlignType() == 2)	// Weapon
					img.setOffsets(-160 + (w*0.5), -200 + h);
			}

			img.savePNGToLump(lump, ISV_OFFSETS);
		}

		if (lump->getType() == LUMP_GFX || lump->getType() == LUMP_PATCH || lump->getType() == LUMP_SPRITE)
		{
			patch_header_t *header = (patch_header_t*)lump->getData();

			if (dl.getOption() == 0)
			{
				point2_t offset = dl.getOffset();

				if (dl.relativeOffset())
				{
					offset.x += header->left;
					offset.y += header->top;
				}

				if (dl.xOffChange())
					header->left = offset.x;

				if (dl.yOffChange())
					header->top = offset.y;
			}
			else
			{
				int w = header->width;
				int h = header->height;

				if (dl.getAlignType() == 0)			// Monster
				{
					header->left = w*0.5;
					header->top = h-4;
				}
				else if (dl.getAlignType() == 1)	// Projectile
				{
					header->left = w*0.5;
					header->top = h*0.5;
				}
				else if (dl.getAlignType() == 2)	// Weapon
				{
					header->left = -160 + (w*0.5);
					header->top = -200 + h;
				}
			}

			lump->setChanged(1, true);
		}
	}

	if (selection.size() == 1)
		openLump(hl_lump);
}

void WadPanel::reloadLump()
{
	for (int a = 0; a < selection.size(); a++)
	{
		Lump* l = (Lump*)lump_list->GetItemData(selection[a]);
		if (l)
		{
			if (l->getType() == LUMP_FOLDER)
				continue;

			wadfile.loadLump(l);
			l->setChanged(0);
			updateList(selection[a]);
		}
	}

	if (selection.size() == 1)
		openLump(hl_lump);
}

void WadPanel::addToPNames()
{
	Lump* pnames = wadfile.getLump(_T("PNAMES"));

	// If PNAMES lump doesn't exist, either copy it from the iwad or
	// create a blank lump
	if (!pnames)
	{
		pnames = iwad.getLump(_T("PNAMES"));
		Lump* nlump = wadfile.addLump(_T("PNAMES"), -1);

		if (pnames)
			nlump->loadData(pnames->getData(), pnames->getSize());

		pnames = nlump;
	}

	vector<string> pnlist;

	// Read existing pnames if any
	if (pnames->getSize() != 0)
	{
		BYTE* data = pnames->getData();
		DWORD n_pnames = *((DWORD*)data);

		DWORD a = 4;
		for (DWORD p = 0; p < n_pnames; p++)
		{
			char name[9] = "";
			memset(name, 0, 9);
			memcpy(name, (data + a), 8);
			pnlist.push_back(wxString::FromAscii(name));
			a += 8;
		}
	}

	// Add selected lumps to list
	for (int a = 0; a < selection.size(); a++)
		vector_add_nodup(pnlist, ((Lump*)(lump_list->GetItemData(selection[a])))->getName(false, false));

	// Write new PNAMES lump
	FILE *fp = fopen("slumptemp", "wb");
	DWORD np = pnlist.size();
	fwrite(&np, 4, 1, fp);
	for (int a = 0; a < pnlist.size(); a++)
	{
		const char* pname = pnlist[a].Truncate(8).ToAscii();
		fwrite(pname, 1, 8, fp);
	}
	fclose(fp);

	pnames->loadFile(_T("slumptemp"));
	remove("slumptemp");
}

void WadPanel::addToTexturex()
{
	// Read existing pnames
	vector<string> pnlist;
	Lump* pnames = wadfile.getLump(_T("PNAMES"));
	if (pnames->getSize() != 0)
	{
		BYTE* data = pnames->getData();
		DWORD n_pnames = *((DWORD*)data);

		DWORD a = 4;
		for (DWORD p = 0; p < n_pnames; p++)
		{
			char name[9] = "";
			memset(name, 0, 9);
			memcpy(name, (data + a), 8);
			pnlist.push_back(wxString::FromAscii(name));
			a += 8;
		}
	}

	// Load existing textures
	vector<texture_t> ntex;
	Lump* texturex = wadfile.getLump(_T("TEXTURE1"));

	if (texturex)
		load_texturex(texturex, ntex, pnlist);
	else
	{
		texturex = wadfile.addLump(_T("TEXTURE1"), -1);
		load_texturex(iwad.getLump(_T("TEXTURE1")), ntex, pnlist);
	}

	// Add new textures
	for (int a = 0; a < selection.size(); a++)
	{
		Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);
		texture_t tex;
		tex.name = lump->getName(false, false);

		Image img;
		if (!img.loadLump(lump))
			continue;

		tex.width = img.getWidth();
		tex.height = img.getHeight();
		tex.flags = 0;
		tex.x_scale = 0;
		tex.y_scale = 0;

		patch_info_t pi;
		int index = -1;
		for (int p = 0; p < pnlist.size(); p++)
		{
			if (pnlist[p] == lump->getName(false, false))
			{
				index = p;
				break;
			}
		}

		if (index == -1)
			continue;

		pi.patch_index = index;
		pi.xoff = 0;
		pi.yoff = 0;

		tex.patches.push_back(pi);
		ntex.push_back(tex);
	}

	// Save new texturex lump
	save_texturex(texturex, ntex);
}

void WadPanel::gfxConvert(int type)
{
	for (int a = 0; a < selection.size(); a++)
	{
		Lump* l = (Lump*)lump_list->GetItemData(selection[a]);

		if (l)
		{
			Image img(palette);
			img.loadLump(l);

			// Convert to Doom GFX
			if (type == 0)
			{
				bool ok = false;
				if (l->getType() == LUMP_PNG)
					ok = img.saveDoomGfx(_T("slumptemp"), 0, -1);
				else
					ok = img.saveDoomGfx(_T("slumptemp"));

				if (ok)
				{
					l->loadFile(_T("slumptemp"));
					remove("slumptemp");
					l->setType(LUMP_GFX);
					updateList(selection[a]);
				}
			}

			// Convert to Doom Flat
			if (type == 1)
			{
				if (img.saveDoomFlat(_T("slumptemp")))
				{
					l->loadFile(_T("slumptemp"));
					remove("slumptemp");
					l->setType(LUMP_FLAT);
					updateList(selection[a]);
				}
			}

			// Convert to PNG
			if (type == 2)
			{
				img.savePNGToLump(l, ISV_OFFSETS);
				l->setType(LUMP_PNG);
				updateList(selection[a]);
			}

			// Convert to BMP
			if (type == 3)
			{
				img.saveBMPToLump(l);
				l->setType(LUMP_IMAGE);
				updateList(selection[a]);
			}
		}
	}

	updateColumnWidths();

	if (selection.size() == 1)
		openLump(hl_lump);
}


BEGIN_EVENT_TABLE(WadPanel, wxPanel)
	EVT_LIST_ITEM_FOCUSED(WP_LIST_LUMPS, WadPanel::onLumpListChange)
	EVT_LIST_ITEM_SELECTED(WP_LIST_LUMPS, WadPanel::onLumpListSelect)
	EVT_LIST_ITEM_DESELECTED(WP_LIST_LUMPS, WadPanel::onLumpListDeSelect)
	EVT_LIST_COL_END_DRAG(WP_LIST_LUMPS, WadPanel::onLumpListColResize)
	//EVT_LIST_END_LABEL_EDIT(WP_LIST_LUMPS, WadPanel::onLumpListEditName)
	EVT_LIST_ITEM_ACTIVATED(WP_LIST_LUMPS, WadPanel::onLumpListActivate)
	EVT_LIST_ITEM_RIGHT_CLICK(WP_LIST_LUMPS, WadPanel::onLumpListRightClick)
	EVT_LIST_COL_RIGHT_CLICK(WP_LIST_LUMPS, WadPanel::onColumnRightClick)

	EVT_LIST_BEGIN_LABEL_EDIT(WP_LIST_LUMPS, WadPanel::onBeginLabelEdit)
	EVT_LIST_END_LABEL_EDIT(WP_LIST_LUMPS, WadPanel::onEndLabelEdit)

	EVT_CHOICE(WP_LUMP_FILTER, WadPanel::onLumpFilterChange)

	// Context menu
	EVT_MENU_RANGE(WP_CM_RENAME, WP_COL_SIZE - 1, WadPanel::onContextMenuSelect)

	EVT_MENU(WP_COL_SIZE, WadPanel::onContextMenuSelect)
	EVT_MENU(WP_COL_TYPE, WadPanel::onContextMenuSelect)
END_EVENT_TABLE()

void WadPanel::onBeginLabelEdit(wxListEvent &event)
{
	Lump* lump = (Lump*)lump_list->GetItemData(event.GetIndex());
	if (!lump)
		event.Veto();
}

void WadPanel::onEndLabelEdit(wxListEvent &event)
{
	if (event.IsEditCancelled())
	{
		wxMessageBox(_T("Edit cancelled"));
		return;
	}

	Lump* lump = (Lump*)lump_list->GetItemData(event.GetIndex());
	string name = event.GetLabel();
	wxMessageBox(name);
	
	/*
	if (!name.empty())
	{
		if (wadfile.zip)
		{
			if (name.Find('/') != -1)
			{
				wxLogMessage(wxString::Format(_T("Invalid lump name \"%s\""), name));
				event.Veto();
			}
		}
		else
		{
			name.UpperCase();
			name.Truncate(8);
		}

		if (lump->getType() == LUMP_FOLDER)
		{
			wadfile.renameDir(lump, name);
			populateLumpList();
			return;
		}

		lump->setName(name);
		//lump_list->SetItemText(event.GetIndex(), name);
		lump->determineType(true);
	}
	*/
}


void WadPanel::onContextMenuSelect(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case WP_CM_RENAME:
		renameLump();
		break;

	case WP_CM_DELETE:
		deleteLump();
		break;

	case WP_CM_CUT:
		cutLump();
		break;

	case WP_CM_COPY:
		copyLump();
		break;

	case WP_CM_PASTE:
		pasteLump();
		break;

	case WP_CM_IMPORT:
		importLump();
		break;

	case WP_CM_EXPORT:
		exportLump();
		break;

	case WP_CM_RELOAD:
		reloadLump();
		break;

	case WP_CM_COMPILEACS:
		compileScript(hl_lump, SCRIPT_ACS);
		hl_lump = NULL;
		break;

	case WP_CM_GFXOFFSETS:
		modifyGfxOffsets();
		break;

	case WP_CM_CONVERTGFX:
		gfxConvert(0);
		break;

	case WP_CM_CONVERTFLAT:
		gfxConvert(1);
		break;

	case WP_CM_CONVERTPNG:
		gfxConvert(2);
		break;

	case WP_CM_CONVERTBMP:
		gfxConvert(3);
		break;

	case WP_CM_ADDPNAMES:
		addToPNames();
		populateLumpList();
		break;

	case WP_CM_ADDTEXTUREX:
		addToPNames();
		addToTexturex();
		populateLumpList();
		break;

	case WP_COL_SIZE:
		col_size = !col_size;
		menu_cols->Check(WP_COL_SIZE, col_size);
		populateLumpList();
		break;

	case WP_COL_TYPE:
		col_type = !col_type;
		menu_cols->Check(WP_COL_TYPE, col_type);
		populateLumpList();
		break;

	default:
		break;
	};
}

void WadPanel::onColumnRightClick(wxListEvent &event)
{
	PopupMenu(menu_cols, event.GetPoint().x, event.GetPoint().y + 20);
}

void WadPanel::onLumpListRightClick(wxListEvent &event)
{
	hl_lump = (Lump*)lump_list->GetItemData(event.GetIndex());

	if (!hl_lump)
		return;

	bool dir = hl_lump->getType() == LUMP_FOLDER;

	wxMenu *popup = new wxMenu(_T(""));
	popup->Append(WP_CM_RENAME, _T("&Rename"));
	popup->Append(WP_CM_DELETE, _T("&Delete"));
	popup->AppendSeparator();
	if (!dir) popup->Append(WP_CM_CUT, _T("Cut"));
	if (!dir) popup->Append(WP_CM_COPY, _T("&Copy"));
	popup->Append(WP_CM_PASTE, _T("&Paste"));
	if (!dir) popup->AppendSeparator();
	if (!dir) popup->Append(WP_CM_IMPORT, _T("&Import"));
	if (!dir) popup->Append(WP_CM_EXPORT, _T("&Export"));
	if (!dir) popup->Append(WP_CM_RELOAD, _T("&Restore"));

	if (hl_lump->getType() == LUMP_TEXT || hl_lump->getType() == LUMP_UNKNOWN)
	{
		popup->AppendSeparator();
		popup->Append(WP_CM_COMPILEACS, _T("Compile ACS"));
	}

	// Get all selected lumps
	vector<Lump*> sel_lumps;
	for (int a = 0; a < selection.size(); a++)
	{
		Lump* l = (Lump*)lump_list->GetItemData(selection[a]);
		if (l)
			sel_lumps.push_back(l);
	}

	// If all lumps are gfx
	bool all_gfx = true;
	for (int a = 0; a < sel_lumps.size(); a++)
	{
		if (!(sel_lumps[a]->getType() == LUMP_GFX ||
			sel_lumps[a]->getType() == LUMP_PATCH ||
			sel_lumps[a]->getType() == LUMP_SPRITE ||
			sel_lumps[a]->getType() == LUMP_PNG))
		{
			all_gfx = false;
			break;
		}
	}

	if (all_gfx)
	{
		popup->AppendSeparator();
		popup->Append(WP_CM_GFXOFFSETS, _T("Modify Offsets"));
		popup->Append(WP_CM_ADDPNAMES, _T("Add To PNAMES"));
		popup->Append(WP_CM_ADDTEXTUREX, _T("Add To TEXTURE1"));
	}

	// If all lumps are images
	bool all_image = true;
	for (int a = 0; a < sel_lumps.size(); a++)
	{
		if (!(sel_lumps[a]->getType() == LUMP_IMAGE ||
			sel_lumps[a]->getType() == LUMP_FLAT ||
			sel_lumps[a]->getType() == LUMP_GFX2 ||
			sel_lumps[a]->getType() == LUMP_GFX ||
			sel_lumps[a]->getType() == LUMP_PATCH ||
			sel_lumps[a]->getType() == LUMP_SPRITE ||
			sel_lumps[a]->getType() == LUMP_PNG))
		{
			all_image = false;
			break;
		}
	}

	if (all_image)
	{
		popup->AppendSeparator();

		wxMenu *convert_menu = new wxMenu(_T(""));
		convert_menu->Append(WP_CM_CONVERTGFX, _T("Doom &Gfx"));
		convert_menu->Append(WP_CM_CONVERTFLAT, _T("Doom &Flat"));
		convert_menu->Append(WP_CM_CONVERTPNG, _T("&PNG"));
		convert_menu->Append(WP_CM_CONVERTBMP, _T("&Bitmap"));

		popup->Append(-1, _T("&Convert To"), convert_menu, _T(""));
	}

	PopupMenu(popup, event.GetPoint().x, event.GetPoint().y + 20);

	delete popup;
}

void WadPanel::onLumpListActivate(wxListEvent &event)
{
	Lump* lump = (Lump*)lump_list->GetItemData(event.GetIndex());

	if (!lump)
	{
		if (event.GetText() == _T(".."))
		{
			wxFileName fn(cur_dir, wxPATH_UNIX);
			wxArrayString dirs = fn.GetDirs();

			if (dirs.size() == 1)
				cur_dir = _T("");
			else
			{
				string dir;
				for (int a = 0; a < dirs.size()-1; a++)
				{
					dir += dirs[a];
					dir += _T("/");
				}
				cur_dir = dir;
			}

			//wxMessageBox(cur_dir);
			populateLumpList();
		}

		return;
	}

	if (lump->getType() == LUMP_FOLDER)
	{
		cur_dir = lump->getName();
		populateLumpList();
	}

	if (lump->getType() == LUMP_WAD)
	{
		if (!zip_askopenwad || wxMessageBox(_T("This lump appears to be a wad file, open it in another tab?\n(changes made to the wad will be saved back to this lump)"), _T("SLumpEd"), wxOK|wxCANCEL) == wxOK)
		{
			lump->dumpToFile(c_path(_T("slumptemp.wad"), DIR_TMP));
			Wad* nwad = main_window->openWad(c_path(_T("slumptemp.wad"), DIR_TMP), false);
			remove(chr(c_path(_T("slumptemp.wad"), DIR_TMP)));

			if (nwad)
			{
				nwad->path = lump->getName();
				nwad->parent = lump;
				main_window->updateTabLabels();
			}
		}
	}
}

void WadPanel::onLumpListColResize(wxListEvent &event)
{
	int width = 0;
	for (int a = 0; a < lump_list->GetColumnCount(); a++)
		width += lump_list->GetColumnWidth(a);

	lump_list->SetSizeHints(width + 20, -1, width + 20, -1);

	Layout();
}

void WadPanel::onLumpListSelect(wxListEvent &event)
{
	selection.push_back(event.GetIndex());
}

void WadPanel::onLumpListDeSelect(wxListEvent &event)
{
	for (int a = 0; a < selection.size(); a++)
	{
		if (selection[a] == event.GetIndex())
		{
			selection[a] = selection.back();
			selection.pop_back();
			return;
		}
	}
}

void WadPanel::onLumpListChange(wxListEvent &event)
{
	hl_lump = (Lump*)lump_list->GetItemData(event.GetIndex());

	// Multiple items selected
	if (selection.size() > 1)
	{
		lump_area_sizer->Detach(lump_area);
		delete(lump_area);
		lump_area = new LumpArea(this);
		lump_area_sizer->Add(lump_area, 1, wxEXPAND);

		int totalsize = 0;

		for (int a = 0; a < selection.size(); a++)
		{
			Lump* lump = (Lump*)lump_list->GetItemData(selection[a]);

			if (lump)
				totalsize += lump->getSize();
		}

		main_window->SetStatusText(wxString::Format(_T("Size: %d"), totalsize), 1);

		main_window->SetStatusText(_T(""), 2);
		lump_area->Show();
		lump_area_sizer->Layout();

		return;
	}

	openLump((Lump*)lump_list->GetItemData(event.GetIndex()));

	updateList(event.GetIndex());
}

void WadPanel::onLumpListEditName(wxListEvent &event)
{
	/*
	wxMessageBox(event.GetText());

	Lump* lump = (Lump*)lump_list->GetItemData(event.GetIndex());

	if (lump)
		lump->setName(event.GetLabel());
		*/
}

void WadPanel::onLumpFilterChange(wxCommandEvent &event)
{
	populateLumpList();
}
