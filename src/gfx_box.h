
#ifndef __GFX_BOX_H__
#define __GFX_BOX_H__

#include <wx/image.h>
#include "misc.h"

class GfxBox : public wxPanel
{
private:
	wxImage		*image;
	int			zoom;
	int			x_offset;
	int			y_offset;
	bool		allow_offset_change;
	bool		moving;
	bool		change;
	point2_t	move_origin;
	point2_t	orig_offset;
	point2_t	screen_origin;
	int			offset_type;

	wxTextCtrl	*entry_xoff;
	wxTextCtrl	*entry_yoff;

public:
	GfxBox(wxWindow *parent);
	~GfxBox();

	void setImage(wxImage* image);
	void zoomIn();
	void zoomOut();
	void setOffsets(int x, int y);
	void setXOff(int x);
	void setYOff(int y);
	void allowOffsetChange(bool a) { allow_offset_change = a; }
	void setEntries(wxTextCtrl *x, wxTextCtrl *y) { entry_xoff = x; entry_yoff = y; }
	void setOffType(int type) { offset_type = type; }
	void setZoom(int z) { zoom = z; }
	void setChanged(bool c) { change = c; }

	int getXOff() { return x_offset; }
	int getYOff() { return y_offset; }
	bool changed() { return change; }
	int getOffType() { return offset_type; }

	void autoOffset(int type = -1);

	void detectOffsetType();

	void paint(wxPaintEvent &event);
	void mouseEvent(wxMouseEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif

#define OTYPE_NONE		0
#define OTYPE_NORMAL	1
#define OTYPE_WEAPON	2
#define OTYPE_MONSTER	3
#define OTYPE_CENTERED	4
