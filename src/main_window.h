
#include <wx/aui/auibook.h>

class MainWindow : public wxFrame
{
private:
	bool	init;

	wxMenuBar		*menu_bar;
	//wxNotebook	*tabs_wadfiles;
	wxAuiNotebook	*tabs_wadfiles;

	wxMenu		*menu_recent;

	void setupMenu();
	void setupToolBar();

public:
	enum
	{
		MW_TABS_WADFILES,

		MENU_FILE_NEW,
		MENU_FILE_NEWZIP,
		MENU_FILE_OPEN,
		MENU_FILE_SAVEALL,
		MENU_FILE_CLOSEALL,
		MENU_FILE_OPTIONS,
		MENU_FILE_QUIT,

		MENU_WAD_SAVE,
		MENU_WAD_SAVEAS,
		MENU_WAD_CLOSE,

		MENU_LUMP_NEW,
		MENU_LUMP_NEW_FOLDER,
		MENU_LUMP_NEWFILE,
		MENU_LUMP_DELETE,
		MENU_LUMP_RENAME,
		MENU_LUMP_IMPORT,
		MENU_LUMP_EXPORT,
		MENU_LUMP_EXPORTWAD,
		MENU_LUMP_MOVEUP,
		MENU_LUMP_MOVEDOWN,
		MENU_LUMP_COPY,
		MENU_LUMP_CUT,
		MENU_LUMP_PASTE,


		MENU_FILE_RECENT,
	};

	MainWindow();
	~MainWindow();

	void enableWadMenus(bool enable, bool zip = false);
	void updateTabLabels();
	Wad* openWad(string filename, bool addrecent = true);

	void updateRecentWadsMenu();

	// Events
	void onResize(wxSizeEvent &event);
	void onMove(wxMoveEvent &event);
	void onClose(wxCloseEvent &event);

	void onTabChanged(wxAuiNotebookEvent &event);
	void onTabClosed(wxAuiNotebookEvent &event);

	void onFileNew(wxCommandEvent &event);
	void onFileNewZip(wxCommandEvent &event);
	void onFileOpen(wxCommandEvent &event);
	void onFileSaveAll(wxCommandEvent &event);
	void onFileCloseAll(wxCommandEvent &event);
	void onFileOptions(wxCommandEvent &event);
	void onFileQuit(wxCommandEvent &event);
	void onFileRecent(wxCommandEvent &event);

	void onWadSave(wxCommandEvent &event);
	void onWadSaveAs(wxCommandEvent &event);
	void onWadClose(wxCommandEvent &event);

	void onLumpNew(wxCommandEvent &event);
	void onLumpNewFolder(wxCommandEvent &event);
	void onLumpNewFile(wxCommandEvent &event);
	void onLumpDelete(wxCommandEvent &event);
	void onLumpRename(wxCommandEvent &event);
	void onLumpImport(wxCommandEvent &event);
	void onLumpExport(wxCommandEvent &event);
	void onLumpExportWad(wxCommandEvent &event);
	void onLumpMoveUp(wxCommandEvent &event);
	void onLumpMoveDown(wxCommandEvent &event);
	void onLumpCopy(wxCommandEvent &event);
	void onLumpCut(wxCommandEvent &event);
	void onLumpPaste(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

void load_recent_wads(Tokenizer *tz);
void save_recent_wads(FILE *fp);
