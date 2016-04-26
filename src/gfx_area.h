
#include "gfx_box.h"
#include "lump_area.h"
#include "image.h"

class GfxLumpArea : public LumpArea
{
private:
	Image		image;
	GfxBox		*draw_area;
	bool		change;

	wxButton	*btn_zoomin;
	wxButton	*btn_zoomout;
	wxTextCtrl	*entry_xoff;
	wxTextCtrl	*entry_yoff;
	wxChoice	*combo_offtype;
	wxColour	*palette;
	wxButton	*btn_save;

public:
	enum
	{
		GA_BTN_ZOOMIN,
		GA_BTN_ZOOMOUT,
		GA_BTN_SAVE,
		GA_ENTRY_XOFF,
		GA_ENTRY_YOFF,
		GA_COMBO_OFFTYPE,
	};

	GfxLumpArea(WadPanel *parent, wxColour *pal);
	~GfxLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	void readPatch();
	void readFlat();

	void savePatch();
	void savePNG();

	void onZoomIn(wxCommandEvent &event);
	void onZoomOut(wxCommandEvent &event);
	void onXOffChanged(wxCommandEvent &event);
	void onYOffChanged(wxCommandEvent &event);
	void onOffTypeChanged(wxCommandEvent &event);
	void onSave(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
