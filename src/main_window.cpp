////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006                                                //
// ------------------------------------------------------------------ //
// main_window.cpp - MainWindow class functions                       //
////////////////////////////////////////////////////////////////////////

// INCLUDES ////////////////////////////////////////////////////////////
#include "main.h"
#include "main_window.h"
#include "wad_panel.h"
#include "cfg_dialog.h"

#include <wx/image.h>

// VARIABLES ///////////////////////////////////////////////////////////
vector<string>	recent_wads;

CVAR(Bool, win_maximize, true, CVAR_SAVE)
CVAR(Int, win_width, 640, CVAR_SAVE)
CVAR(Int, win_height, 480, CVAR_SAVE)
CVAR(Int, win_x, -1, CVAR_SAVE)
CVAR(Int, win_y, -1, CVAR_SAVE)
CVAR(Int, tb_style, 0, CVAR_SAVE)
CVAR(String, iwad_path, "", CVAR_SAVE)
CVAR(Int, last_filter, 0, CVAR_SAVE)
CVAR(Int, n_recent_wads, 4, CVAR_SAVE)

// EXTERNAL VARIABLES //////////////////////////////////////////////////
extern vector<WadPanel*> open_wad_panels;
extern Wad iwad;
extern MainWindow *main_window;

void load_recent_wads(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "wad")
			recent_wads.push_back(tz->get_token());

		token = tz->get_token();
	}
}

void save_recent_wads(FILE *fp)
{
	fprintf(fp, "recent_wads\n{\n");

	for (int a = 0; a < recent_wads.size(); a++)
		fprintf(fp, "\twad \"%s\"\n", recent_wads[a].c_str());

	fprintf(fp, "}\n\n");
}

void add_recent_wad(string path)
{
	// If the wad exists in the list, remove it
	if (vector_exists(recent_wads, path))
		recent_wads.erase(find(recent_wads.begin(), recent_wads.end(), path));

	// If the list is full, delete the least recent
	if (recent_wads.size() == n_recent_wads)
		recent_wads.pop_back();

	// Add the path
	recent_wads.insert(recent_wads.begin(), path);

	main_window->updateRecentWadsMenu();
}

void remove_recent_wad(string path)
{
	for (int a = 0; a < recent_wads.size(); a++)
	{
		if (recent_wads[a] == path)
		{
			recent_wads.erase(recent_wads.begin() + a);
			main_window->updateRecentWadsMenu();
			return;
		}
	}
}

// MainWindow::MainWindow: Constructor
// -----------------------------------
MainWindow::MainWindow()
:	wxFrame(NULL, -1, _T("SLumpEd"))
{
	init = false;

	// Set window size/position
	SetSize(win_width, win_height);
	SetPosition(wxPoint(win_x, win_y));

	// Setup status bar
	CreateStatusBar(3);
	int widths[] = { -1, 128, 128 };
	SetStatusWidths(3, widths);

	// Setup menu
	setupMenu();

	// Setup toolbar
	setupToolBar();

	// Disable wad menus/toolbars
	enableWadMenus(false);

	wxPanel *back = new wxPanel(this);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	back->SetSizer(vbox);

	// Setup wad tabs
	long tabstyle = /*wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|*/wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_CLOSE_ON_ALL_TABS;
	//tabs_wadfiles = new wxNotebook(back, MW_TABS_WADFILES, wxDefaultPosition, wxDefaultSize, wxNB_NOPAGETHEME);
	tabs_wadfiles = new wxAuiNotebook(back, MW_TABS_WADFILES, wxDefaultPosition, wxDefaultSize, tabstyle);
	vbox->Add(tabs_wadfiles, 1, wxEXPAND);

	SetIcon(wxIcon("res/icon.ico",  wxBITMAP_TYPE_ICO, -1, -1));
	Show(true);

	if (win_maximize)
		Maximize();

	back->Layout();

	init = true;
}

// MainWindow::~MainWindow: Destructor
// -----------------------------------
MainWindow::~MainWindow()
{
	if (!IsMaximized())
	{
		win_width = GetRect().width;
		win_height = GetRect().height;
		win_x = GetRect().x;
		win_y = GetRect().y;
		win_maximize = false;
	}
	else
		win_maximize = true;
}

void MainWindow::updateRecentWadsMenu()
{
	wxMenuItemList items = menu_recent->GetMenuItems();
	for (int a = 0; a < items.size(); a++)
		menu_recent->Delete(items[a]);

	for (int a = 0; a < recent_wads.size(); a++)
		menu_recent->Append(MENU_FILE_RECENT+a, recent_wads[a], _T(""));
}

// MainWindow::setupMenu: Sets up the main window menu
// ---------------------------------------------------
void MainWindow::setupMenu()
{
	menu_bar = new wxMenuBar();
	SetMenuBar(menu_bar);

	wxMenu *file_menu = new wxMenu();
	file_menu->Append(MENU_FILE_NEW, _T("&New"), _T("Create a new wad file"));
	file_menu->Append(MENU_FILE_OPEN, _T("&Open"), _T("Open a wad file"));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_SAVEALL, _T("&Save All"), _T("Saves all open wad files"));
	file_menu->Append(MENU_FILE_CLOSEALL, _T("Close All"), _T("Close all open wad files"));
	file_menu->AppendSeparator();
	menu_recent = new wxMenu();
	updateRecentWadsMenu();
	file_menu->Append(-1, _T("&Recent Wads"), menu_recent, _T(""));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_OPTIONS, _T("Options"), _T("Open SLumpEd configuration options"));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_QUIT, _T("&Quit"), _T("Quit SLumpEd"));
	menu_bar->Append(file_menu, _T("&File"));

	wxMenu *menu_wad = new wxMenu();
	menu_wad->Append(MENU_WAD_SAVE, _T("&Save"), _T("Save the wad file"));
	menu_wad->Append(MENU_WAD_SAVEAS, _T("&Save As"), _T("Save the wad to a new file"));
	menu_wad->AppendSeparator();
	menu_wad->Append(MENU_WAD_CLOSE, _T("&Close"), _T("Close the wad file"));
	menu_bar->Append(menu_wad, _T("&Wad"));

	wxMenu *lump_menu = new wxMenu();
	lump_menu->Append(MENU_LUMP_NEW, _T("&New"), _T("Create a new blank lump"));
	lump_menu->Append(MENU_LUMP_NEW_FOLDER, _T("New Folder"), _T("Create a new empty folder"));
	lump_menu->Append(MENU_LUMP_NEWFILE, _T("New from &File"), _T("Load a file as a new lump"));
	lump_menu->AppendSeparator();
	lump_menu->Append(MENU_LUMP_DELETE, _T("&Delete"), _T("Delete selected lump(s)"));
	lump_menu->Append(MENU_LUMP_RENAME, _T("&Rename"), _T("Rename selected lump(s)"));
	lump_menu->AppendSeparator();
	lump_menu->Append(MENU_LUMP_IMPORT, _T("&Import"), _T("Import file(s) to selected lump(s)"));
	lump_menu->Append(MENU_LUMP_EXPORT, _T("&Export"), _T("Export selected lump(s) to file(s)"));
	lump_menu->Append(MENU_LUMP_EXPORTWAD, _T("Export to New &Wad"), _T("Creates a new Wad file with the selected lump(s)"));
	lump_menu->AppendSeparator();
	lump_menu->Append(MENU_LUMP_CUT, _T("Cut"), _T("Cut selected lump(s) to the clipboard"));
	lump_menu->Append(MENU_LUMP_COPY, _T("&Copy"), _T("Copy selected lump(s) to the clipboard"));
	lump_menu->Append(MENU_LUMP_PASTE, _T("&Paste"), _T("Paste lump(s) from the clipboard"));
	menu_bar->Append(lump_menu, _T("&Lump"));
}

// MainWindow::setupToolBar: Sets up the main window toolbar
// ---------------------------------------------------------
void MainWindow::setupToolBar()
{
	if (tb_style == 1)
		CreateToolBar(wxTB_HORIZONTAL|wxNO_BORDER|wxTB_TEXT|wxTB_FLAT);
	else if (tb_style == 2)
		CreateToolBar(wxTB_HORIZONTAL|wxNO_BORDER|wxTB_HORZ_TEXT|wxTB_FLAT);
	else
		CreateToolBar(wxTB_HORIZONTAL|wxNO_BORDER|wxTB_FLAT);

	wxToolBar *tb = GetToolBar();
	tb->SetToolBitmapSize(wxSize(16, 16));

	tb->AddTool(MENU_FILE_NEW, _T("New"), wxBitmap(wxImage(_T("res/toolbar/new_wad.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/new_wad_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("New Wad"), _T("Create a new wad file"));

	tb->AddTool(MENU_FILE_NEWZIP, _T("New Zip"), wxBitmap(wxImage(_T("res/toolbar/new_zip.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/new_zip_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("New Zip"), _T("Create a new zip file"));

	tb->AddTool(MENU_FILE_OPEN, _T("Open"), wxBitmap(wxImage(_T("res/toolbar/open.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/open_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Open"), _T("Open a wad file"));

	tb->AddTool(MENU_FILE_SAVEALL, _T("Save All"), wxBitmap(wxImage(_T("res/toolbar/save_all.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/save_all_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Save All"), _T("Save all open wad files"));

	tb->AddTool(MENU_FILE_CLOSEALL, _T("Close All"), wxBitmap(wxImage(_T("res/toolbar/close_all.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/close_all_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Close All"), _T("Close all open wad files"));

	tb->AddSeparator();

	tb->AddTool(MENU_WAD_SAVE, _T("Save"), wxBitmap(wxImage(_T("res/toolbar/save.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/save_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Save"), _T("Save the wad file"));

	tb->AddTool(MENU_WAD_SAVEAS, _T("Save As"), wxBitmap(wxImage(_T("res/toolbar/save_as.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/save_as_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Save As"), _T("Save the wad to a new file"));

	tb->AddTool(MENU_WAD_CLOSE, _T("Close"), wxBitmap(wxImage(_T("res/toolbar/close.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/close_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Close"), _T("Close the wad file"));

	tb->AddSeparator();

	tb->AddTool(MENU_LUMP_NEW, _T("New"), wxBitmap(wxImage(_T("res/toolbar/new_lump.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/new_lump_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("New Lump"), _T("Create a new blank lump"));

	tb->AddTool(MENU_LUMP_NEW_FOLDER, _T("New Folder"), wxBitmap(wxImage(_T("res/toolbar/new_folder.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/new_folder_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("New Folder"), _T("Create a new empty folder"));

	tb->AddTool(MENU_LUMP_NEWFILE, _T("Load"), wxBitmap(wxImage(_T("res/toolbar/new_lump_file.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/new_lump_file_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("New Lump from File"), _T("Load a file as a new lump"));

	tb->AddTool(MENU_LUMP_DELETE, _T("Delete"), wxBitmap(wxImage(_T("res/toolbar/delete_lump.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/delete_lump_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Delete Lump"), _T("Delete selected lump(s)"));

	tb->AddTool(MENU_LUMP_RENAME, _T("Rename"), wxBitmap(wxImage(_T("res/toolbar/rename.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/rename_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Rename Lump"), _T("Rename selected lump(s)"));

	tb->AddTool(MENU_LUMP_MOVEUP, _T("Up"), wxBitmap(wxImage(_T("res/toolbar/up.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/up_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Move Lump Up"), _T("Moves selected lump(s) up one position"));

	tb->AddTool(MENU_LUMP_MOVEDOWN, _T("Down"), wxBitmap(wxImage(_T("res/toolbar/down.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/down_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Move Lump Down"), _T("Moves selected lump(s) down one position"));

	tb->AddTool(MENU_LUMP_IMPORT, _T("Import"), wxBitmap(wxImage(_T("res/toolbar/import.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/import_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Import"), _T("Import file(s) to selected lump(s)"));

	tb->AddTool(MENU_LUMP_EXPORT, _T("Export"), wxBitmap(wxImage(_T("res/toolbar/export.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/export_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Export"), _T("Export selected lump(s) to file(s)"));

	tb->AddTool(MENU_LUMP_CUT, _T("Cut"), wxBitmap(wxImage(_T("res/toolbar/cut.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/cut_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Cut"), _T("Cut selected lump(s) to the clipboard"));

	tb->AddTool(MENU_LUMP_COPY, _T("Copy"), wxBitmap(wxImage(_T("res/toolbar/copy.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/copy_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Copy"), _T("Copy selected lump(s) to the clipboard"));

	tb->AddTool(MENU_LUMP_PASTE, _T("Paste"), wxBitmap(wxImage(_T("res/toolbar/paste.png"), wxBITMAP_TYPE_PNG)),
										wxBitmap(wxImage(_T("res/toolbar/paste_d.png"), wxBITMAP_TYPE_PNG)),
										wxITEM_NORMAL, _T("Paste"), _T("Paste lump(s) from the clipboard"));

	tb->Realize();
}

// MainWindow::enableWadMenus: Enables/disables any menus/toolbar items that rely on a wad being open
// --------------------------------------------------------------------------------------------------
void MainWindow::enableWadMenus(bool enable, bool zip)
{
	menu_bar->EnableTop(1, enable);
	menu_bar->EnableTop(2, enable);

	wxToolBar *tb = GetToolBar();
	tb->EnableTool(MENU_FILE_SAVEALL, enable);
	tb->EnableTool(MENU_FILE_CLOSEALL, enable);
	tb->EnableTool(MENU_WAD_SAVE, enable);
	tb->EnableTool(MENU_WAD_SAVEAS, enable);
	tb->EnableTool(MENU_WAD_CLOSE, enable);
	tb->EnableTool(MENU_LUMP_NEW, enable);
	tb->EnableTool(MENU_LUMP_NEW_FOLDER, enable && zip);
	tb->EnableTool(MENU_LUMP_NEWFILE, enable);
	tb->EnableTool(MENU_LUMP_DELETE, enable);
	tb->EnableTool(MENU_LUMP_RENAME, enable);
	tb->EnableTool(MENU_LUMP_MOVEUP, enable);
	tb->EnableTool(MENU_LUMP_MOVEDOWN, enable);
	tb->EnableTool(MENU_LUMP_IMPORT, enable);
	tb->EnableTool(MENU_LUMP_EXPORT, enable);
	tb->EnableTool(MENU_LUMP_CUT, enable);
	tb->EnableTool(MENU_LUMP_COPY, enable);
	tb->EnableTool(MENU_LUMP_PASTE, enable);
}

// MainWindow::updateTabLabels: Updates wad tab labels with their respective wad paths
// -----------------------------------------------------------------------------------
void MainWindow::updateTabLabels()
{
	for (int a = 0; a < open_wad_panels.size(); a++)
		tabs_wadfiles->SetPageText(a, open_wad_panels[a]->getFilename());
}

Wad* MainWindow::openWad(string filename, bool addrecent)
{
	WadPanel *wp = new WadPanel(tabs_wadfiles, filename);

	if (wp->isOk())
	{
		tabs_wadfiles->AddPage(wp, filename, true);
		if (addrecent) add_recent_wad(filename);
		return wp->getWad();
	}
	else
	{
		delete wp;
		return NULL;
	}
}



////////////////////////////////////////////////////////////////////////
// EVENTS //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	//EVT_SIZE(MainWindow::onResize)
	//EVT_MOVE(MainWindow::onMove)
	EVT_CLOSE(MainWindow::onClose)
	EVT_AUINOTEBOOK_PAGE_CHANGED(MW_TABS_WADFILES, MainWindow::onTabChanged)
	EVT_AUINOTEBOOK_PAGE_CLOSE(MW_TABS_WADFILES, MainWindow::onTabClosed)

	EVT_MENU(MENU_FILE_NEW, MainWindow::onFileNew)
	EVT_MENU(MENU_FILE_NEWZIP, MainWindow::onFileNewZip)
	EVT_MENU(MENU_FILE_OPEN, MainWindow::onFileOpen)
	EVT_MENU(MENU_FILE_SAVEALL, MainWindow::onFileSaveAll)
	EVT_MENU(MENU_FILE_CLOSEALL, MainWindow::onFileCloseAll)
	EVT_MENU(MENU_FILE_OPTIONS, MainWindow::onFileOptions)
	EVT_MENU(MENU_FILE_QUIT, MainWindow::onFileQuit)
	EVT_MENU_RANGE(MENU_FILE_RECENT, MENU_FILE_RECENT + 200, MainWindow::onFileRecent)

	EVT_MENU(MENU_WAD_SAVE, MainWindow::onWadSave)
	EVT_MENU(MENU_WAD_SAVEAS, MainWindow::onWadSaveAs)
	EVT_MENU(MENU_WAD_CLOSE, MainWindow::onWadClose)

	EVT_MENU(MENU_LUMP_NEW, MainWindow::onLumpNew)
	EVT_MENU(MENU_LUMP_NEW_FOLDER, MainWindow::onLumpNewFolder)
	EVT_MENU(MENU_LUMP_NEWFILE, MainWindow::onLumpNewFile)
	EVT_MENU(MENU_LUMP_DELETE, MainWindow::onLumpDelete)
	EVT_MENU(MENU_LUMP_RENAME, MainWindow::onLumpRename)
	EVT_MENU(MENU_LUMP_MOVEUP, MainWindow::onLumpMoveUp)
	EVT_MENU(MENU_LUMP_MOVEDOWN, MainWindow::onLumpMoveDown)
	EVT_MENU(MENU_LUMP_IMPORT, MainWindow::onLumpImport)
	EVT_MENU(MENU_LUMP_EXPORT, MainWindow::onLumpExport)
	EVT_MENU(MENU_LUMP_EXPORTWAD, MainWindow::onLumpExportWad)
	EVT_MENU(MENU_LUMP_COPY, MainWindow::onLumpCopy)
	EVT_MENU(MENU_LUMP_CUT, MainWindow::onLumpCut)
	EVT_MENU(MENU_LUMP_PASTE, MainWindow::onLumpPaste)
END_EVENT_TABLE()

// MainWindow::onClose: When the window is closed, check if any wads need saving etc
// ---------------------------------------------------------------------------------
void MainWindow::onClose(wxCloseEvent &event)
{
	for (int a = 0; a < open_wad_panels.size(); a++)
	{
		if (!open_wad_panels[a]->closeWad())
		{
			if (event.CanVeto())
			{
				event.Veto();
				return;
			}
		}
	}

	Destroy();
	wxTheApp->ExitMainLoop();
}

// MainWindow::onTabChanged: When changing wad tabs
// ------------------------------------------------
void MainWindow::onTabChanged(wxAuiNotebookEvent &event)
{
	enableWadMenus(true, open_wad_panels[tabs_wadfiles->GetSelection()]->getWad()->zip);
}

// MainWindow::onTabClosed: When the close tab button is clicked
// -------------------------------------------------------------
void MainWindow::onTabClosed(wxAuiNotebookEvent &event)
{
	if (open_wad_panels[event.GetSelection()]->closeWad())
	{
		if (open_wad_panels.size() == 1)
			enableWadMenus(false);
	}
	else
		event.Veto();
}

// MainWindow::onResize: When the main window is resized (UNUSED)
// -----------------------------------------------------
void MainWindow::onResize(wxSizeEvent &event)
{
	/*
	if (init)
	{
		if (!IsMaximized())
		{
			win_width = GetRect().width;
			win_height = GetRect().height;
			win_maximize = false;
		}
		else
			win_maximize = true;
	}

	Layout();
	*/
}

// MainWindow::onMove: When the window is moved (UNUSED)
// --------------------------------------------
void MainWindow::onMove(wxMoveEvent &event)
{
	/*
	if (init)
	{
		win_x = GetRect().x;
		win_y = GetRect().y;
	}
	*/
}

/////////////////////////////////////////////////////////////////
// MENU EVENTS //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void MainWindow::onFileNew(wxCommandEvent &event)
{
	WadPanel *wp = new WadPanel(tabs_wadfiles, "");
	tabs_wadfiles->AddPage(wp, _T("UNSAVED"), true);
}

void MainWindow::onFileNewZip(wxCommandEvent &event)
{
	WadPanel *wp = new WadPanel(tabs_wadfiles, "", true);
	tabs_wadfiles->AddPage(wp, _T("UNSAVED"), true);
}

void MainWindow::onFileOpen(wxCommandEvent &event)
{
	wxFileDialog *OpenDialog = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
		_T("Any Supported File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3"),
		wxOPEN|wxMULTIPLE|wxFILE_MUST_EXIST, wxDefaultPosition);

	OpenDialog->SetFilterIndex(last_filter);
	if (OpenDialog->ShowModal() == wxID_OK)
	{
		wxArrayString files;
		OpenDialog->GetPaths(files);

		last_filter = OpenDialog->GetFilterIndex();

		for (int a = 0; a < files.size(); a++)
		{
			WadPanel *wp = new WadPanel(tabs_wadfiles, files[a]);

			if (wp->isOk())
			{
				tabs_wadfiles->AddPage(wp, files[a], true);
				if (!wp->getWad()->parent) add_recent_wad(files[a]);
			}
			else
				delete wp;
		}
	}

	Layout();
	Refresh();
}

void MainWindow::onFileSaveAll(wxCommandEvent &event)
{
	for (int a = 0; a < open_wad_panels.size(); a++)
		open_wad_panels[a]->saveWad();

	updateTabLabels();
}

void MainWindow::onFileCloseAll(wxCommandEvent &event)
{
	int a = 0;

	while (a < open_wad_panels.size())
	{
		if (open_wad_panels[a]->closeWad())
			tabs_wadfiles->DeletePage(a);
		else
			a++;
	}

	if (open_wad_panels.size() == 0)
		enableWadMenus(false);
}

void MainWindow::onFileOptions(wxCommandEvent &event)
{
	/*
	wxFileDialog *OpenDialog = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
		_T("Doom Wad files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST, wxDefaultPosition);

	if (OpenDialog->ShowModal() == wxID_OK)
	{
		iwad_path = OpenDialog->GetPath();
		iwad.close();
		iwad.open(iwad_path);

		for (int a = 0; a < open_wad_panels.size(); a++)
			open_wad_panels[a]->loadPalette();
	}
	*/

	ConfigDialog cfg_dialog;
	cfg_dialog.ShowModal();
}

void MainWindow::onFileQuit(wxCommandEvent &event)
{
	Close();
}

void MainWindow::onFileRecent(wxCommandEvent &event)
{
	int i = event.GetId() - MENU_FILE_RECENT;

	if (i >= recent_wads.size())
		return;

	string path = recent_wads[i];

	for (int a = 0; a < open_wad_panels.size(); a++)
	{
		if (open_wad_panels[a]->getFilename() == path)
			return;
	}

	WadPanel *wp = new WadPanel(tabs_wadfiles, path);

	if (wp->isOk())
	{
		tabs_wadfiles->AddPage(wp, path, true);
		add_recent_wad(path);
	}
	else
	{
		delete wp;
		wxMessageBox(_T("File not found!"));
		remove_recent_wad(path);
	}
}

void MainWindow::onWadSave(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->saveWad();
	open_wad_panels[tabs_wadfiles->GetSelection()]->updateList();
	updateTabLabels();
}

void MainWindow::onWadSaveAs(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->saveWadAs();
	open_wad_panels[tabs_wadfiles->GetSelection()]->updateList();
	add_recent_wad(open_wad_panels[tabs_wadfiles->GetSelection()]->getFilename());
	updateTabLabels();
}

void MainWindow::onWadClose(wxCommandEvent &event)
{
	if (open_wad_panels[tabs_wadfiles->GetSelection()]->closeWad())
	{
		tabs_wadfiles->DeletePage(tabs_wadfiles->GetSelection());

		if (open_wad_panels.size() == 0)
			enableWadMenus(false);
	}
}

void MainWindow::onLumpNew(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->newLump();
}

void MainWindow::onLumpNewFolder(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->newFolder();
}

void MainWindow::onLumpNewFile(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->newLumpFromFile();
}

void MainWindow::onLumpDelete(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->deleteLump();
}

void MainWindow::onLumpRename(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->renameLump();
}

void MainWindow::onLumpImport(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->importLump();
}

void MainWindow::onLumpExport(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->exportLump();
}

void MainWindow::onLumpExportWad(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->exportLumpToWad();
}

void MainWindow::onLumpMoveUp(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->moveLump(true);
}

void MainWindow::onLumpMoveDown(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->moveLump(false);
}

void MainWindow::onLumpCopy(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->copyLump();
}

void MainWindow::onLumpCut(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->cutLump();
}

void MainWindow::onLumpPaste(wxCommandEvent &event)
{
	open_wad_panels[tabs_wadfiles->GetSelection()]->pasteLump();
}
