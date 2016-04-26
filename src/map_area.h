
#include "lump_area.h"
#include "misc.h"

#include <wx/glcanvas.h>

class MapCanvas : public wxGLCanvas
{
private:
	vector<rect_t>	preview_lines;
	point2_t		dimensions;

public:
	MapCanvas(wxWindow *parent);
	~MapCanvas();

	void loadMap(Wad* wad, string mapname);

	void onPaint(wxPaintEvent &event);
	void onResize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};

class MapLumpArea : public LumpArea
{
private:
	MapCanvas	*map_canvas;
	Wad			*wadfile;

public:
	MapLumpArea(WadPanel *parent, Wad *wadfile);
	~MapLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();
};
