
#include "lump_area.h"
#include <wx/listctrl.h>

class WadPanel;

class LumpList : public wxListCtrl
{
private:
	WadPanel	*parent;

public:
	LumpList(WadPanel *parent, int id);
	~LumpList();

	void updateEntry(int index);

	// Events
	void onKeyDown(wxKeyEvent &event);
	void onBeginLabelEdit(wxListEvent &event);
	void onEndLabelEdit(wxListEvent &event);

	DECLARE_EVENT_TABLE()
};

class WadPanel : public wxPanel
{
private:
	Wad		wadfile;
	bool	opened_ok;

	vector<DWORD>		selection;
	LumpList			*lump_list;
	LumpArea			*lump_area;
	wxStaticBoxSizer	*lump_area_sizer;
	wxImageList			*image_list;
	wxMenu				*menu_cols;
	Lump				*hl_lump;
	string				cur_dir;
	wxColour			*palette;
	wxChoice			*lump_filter;

public:
	enum
	{
		WP_LIST_LUMPS,
		WP_LUMP_FILTER,

		WP_CM_RENAME,
		WP_CM_DELETE,
		WP_CM_CUT,
		WP_CM_COPY,
		WP_CM_PASTE,
		WP_CM_EXPORT,
		WP_CM_IMPORT,
		WP_CM_RELOAD,

		WP_CM_COMPILEACS,

		WP_CM_GFXOFFSETS,
		WP_CM_CONVERTGFX,
		WP_CM_CONVERTFLAT,
		WP_CM_CONVERTPNG,
		WP_CM_CONVERTBMP,

		WP_CM_ADDPNAMES,
		WP_CM_ADDTEXTUREX,

		WP_COL_SIZE,
		WP_COL_TYPE,
	};

	WadPanel(wxWindow *parent, string wadpath, bool zip = false);
	~WadPanel();

	bool	isOk() { return opened_ok; }
	string	getFilename();
	Wad*	getWad() { return &wadfile; }

	void	populateLumpList();
	void	loadPalette();
	bool	closeWad();
	void	saveWad();
	void	saveWadAs();

	void	newLump();
	void	newFolder();
	void	newLumpFromFile();
	void	deleteLump(bool ignoredir = false);
	void	renameLump();
	void	importLump();
	void	exportLump();
	void	exportImageLump(Lump* lump);
	void	exportLumpToWad();
	void	moveLump(bool up);
	void	copyLump();
	void	pasteLump();
	void	cutLump();
	void	reloadLump();
	void	addToPNames();
	void	addToTexturex();

	void	addLumpToList(int index, Lump* lump);
	void	updateList(int index = -1);
	void	updateList(Lump* lump);

	void	updateSelection();
	void	updateColumnWidths();

	void	gotoNonFolder(int &index);

	void	openLump(Lump* lump);

	void	compileScript(Lump* lump, int type);
	void	modifyGfxOffsets();
	void	gfxConvert(int type);

	bool	lumpIsFiltered(Lump* lump);

	// Events
	void onLumpListChange(wxListEvent &event);
	void onLumpListEditName(wxListEvent &event);
	void onLumpListSelect(wxListEvent &event);
	void onLumpListDeSelect(wxListEvent &event);
	void onLumpListColResize(wxListEvent &event);
	void onLumpListActivate(wxListEvent &event);
	void onLumpListRightClick(wxListEvent &event);
	void onColumnRightClick(wxListEvent &event);

	void onLumpFilterChange(wxCommandEvent &event);

	void onBeginLabelEdit(wxListEvent &event);
	void onEndLabelEdit(wxListEvent &event);

	void onContextMenuSelect(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#define SCRIPT_ACS	0
