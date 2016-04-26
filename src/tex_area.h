
#include "lump_area.h"
#include "misc.h"
#include "gfx_box.h"

class TextureLumpArea;

class TextureArea : public wxPanel
{
private:
	texture_t*					texture;
	vector<wxImage*>			images;
	Wad*						wadfile;
	int							sel_patch;
	wxColour					*palette;
	float						zoom;
	TextureLumpArea				*parent;

	point2_t	move_origin;
	point2_t	orig_offset;
	point2_t	screen_origin;
	bool		moving;

public:
	TextureArea(TextureLumpArea *parent, Wad *wadfile, wxColour* palette = NULL);
	~TextureArea();

	void setTexture(texture_t* tex);
	void selPatch(int p) { sel_patch = p; }
	void changeZoom(float amount);

	void paint(wxPaintEvent &event);
	void mouseEvent(wxMouseEvent &event);
	void onKeyDown(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};

class TextureLumpArea : public LumpArea
{
private:
	wxListBox			*list_textures;
	vector<texture_t>	textures;
	wxButton			*btn_newtexture;
	wxButton			*btn_rentexture;
	wxButton			*btn_deltexture;

	TextureArea			*ta_texture;
	wxButton			*btn_zoomin;
	wxButton			*btn_zoomout;
	wxTextCtrl			*entry_texwidth;
	wxTextCtrl			*entry_texheight;
	wxTextCtrl			*entry_xscale;
	wxTextCtrl			*entry_yscale;
	wxStaticText		*label_scalesize;

	wxListBox			*list_pnames;
	vector<string>		pnames;
	GfxBox				*gbox_patch;
	wxButton			*btn_addpatch;

	wxListBox			*list_patches;
	wxTextCtrl			*entry_patchxoff;
	wxTextCtrl			*entry_patchyoff;
	wxButton			*btn_removepatch;

	Wad					*wadfile;
	wxColour			*palette;

public:
	bool		change;

	enum
	{
		TLA_LIST_TEXTURES,
		TLA_LIST_PATCHES,
		TLA_LIST_PNAMES,
		TLA_BTN_ZOOMIN,
		TLA_BTN_ZOOMOUT,
		TLA_ENTRY_TEXWIDTH,
		TLA_ENTRY_TEXHEIGHT,
		TLA_ENTRY_PATCHXOFF,
		TLA_ENTRY_PATCHYOFF,
		TLA_BTN_ADDPATCH,
		TLA_BTN_REMOVEPATCH,
		TLA_BTN_NEWTEXTURE,
		TLA_BTN_DELTEXTURE,
		TLA_BTN_RENTEXTURE,
		TLA_ENTRY_XSCALE,
		TLA_ENTRY_YSCALE,
	};

	TextureLumpArea(WadPanel *parent, Wad* wadfile, wxColour* palette = NULL);
	~TextureLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	void changePatch(int index);
	void addPatch(int index);
	void removePatch(int index);
	void orderPatch(int index, bool up = true);

	void updateScaleLabel();

	void onTextureListChange(wxCommandEvent &event);
	void onPatchesListChange(wxCommandEvent &event);
	void onPNamesListChange(wxCommandEvent &event);
	void onZoomIn(wxCommandEvent &event);
	void onZoomOut(wxCommandEvent &event);
	void onWidthChanged(wxCommandEvent &event);
	void onHeightChanged(wxCommandEvent &event);
	void onXOffChanged(wxCommandEvent &event);
	void onYOffChanged(wxCommandEvent &event);
	void onAddPatch(wxCommandEvent &event);
	void onRemovePatch(wxCommandEvent &event);
	void onNewTexture(wxCommandEvent &event);
	void onDelTexture(wxCommandEvent &event);
	void onRenTexture(wxCommandEvent &event);
	void onXScaleChanged(wxCommandEvent &event);
	void onYScaleChanged(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
