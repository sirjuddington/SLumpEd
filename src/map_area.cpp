////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// map_area.cpp - MapArea class functions                             //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "map_area.h"

MapCanvas::MapCanvas(wxWindow *parent)
:	wxGLCanvas(parent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
	SetCurrent();
	glViewport(0, 0, GetSize().x, GetSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Enable(false);
	Show(true);
	Raise();
}

MapCanvas::~MapCanvas()
{
}

void MapCanvas::loadMap(Wad* wad, string mapname)
{
	preview_lines.clear();
	dimensions.set(0, 0);

	if (!wad || mapname == "")
		return;

	vector<point2_t> verts;
	bool hexen_format = false;

	long offset = wad->getLumpIndex(mapname);

	// Check for BEHAVIOR lump
	long index = offset;
	bool done = false;

	while (!done)
	{
		index++;

		if (index >= wad->numLumps())
			done = true;
		else if (wad->lumpAt(index)->getName() == _T("THINGS")	||
				wad->lumpAt(index)->getName() == _T("LINEDEFS") ||
				wad->lumpAt(index)->getName() == _T("SIDEDEFS") ||
				wad->lumpAt(index)->getName() == _T("VERTEXES") ||
				wad->lumpAt(index)->getName() == _T("SEGS")		||
				wad->lumpAt(index)->getName() == _T("SSECTORS") ||
				wad->lumpAt(index)->getName() == _T("NODES")	||
				wad->lumpAt(index)->getName() == _T("SECTORS")	||
				wad->lumpAt(index)->getName() == _T("REJECT")	||
				wad->lumpAt(index)->getName() == _T("SCRIPTS")	||
				wad->lumpAt(index)->getName() == _T("BLOCKMAP"))
		{
			done = false;
		}
		else if (wad->lumpAt(index)->getName() == _T("BEHAVIOR"))
		{
			hexen_format = true;
			done = true;
		}
		else
			done = true;
	}

	// Read vertices
	Lump *lump = wad->getLump("VERTEXES", offset);

	if (!lump)
		return;

	int n_verts = lump->getSize() / 4;

	short min_x = 20000;
	short min_y = 20000;
	short max_x = -20000;
	short max_y = -20000;

	BYTE* data = lump->getData();
	for (int a = 0; a < n_verts; a++)
	{
		short x, y;
		memcpy(&x, data, 2);
		data += 2;
		memcpy(&y, data, 2);
		data += 2;

		y = -y;

		if (x < min_x)
			min_x = x;
		if (x > max_x)
			max_x = x;

		if (y < min_y)
			min_y = y;
		if (y > max_y)
			max_y = y;

		verts.push_back(point2_t(x, y));
	}

	for (int a = 0; a < n_verts; a++)
	{
		verts[a].x -= min_x;
		verts[a].y -= min_y;
	}

	min_x -= 256;
	min_y -= 256;
	max_x += 256;
	max_y += 256;
	dimensions.set(max_x - min_x, max_y - min_y);

	// Read lines
	lump = wad->getLump("LINEDEFS", offset);

	if (!lump)
		return;

	int size = 0;
	if (hexen_format) size = 16;
	else size = 14;

	int n_lines = lump->getSize() / size;

	data = lump->getData();
	for (int a = 0; a < n_lines; a++)
	{
		short v1, v2;
		memcpy(&v1, data, 2);
		data += 2;
		memcpy(&v2, data, 2);
		data += 2;

		preview_lines.push_back(rect_t(verts[v1], verts[v2]));
		data += (size - 4);
	}
}

BEGIN_EVENT_TABLE(MapCanvas, wxGLCanvas)
	EVT_PAINT(MapCanvas::onPaint)
	EVT_SIZE(MapCanvas::onResize)
END_EVENT_TABLE()

void MapCanvas::onPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	SetCurrent();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (preview_lines.size() == 0)
		return;

	for (int a = 0; a < preview_lines.size(); a++)
	{
		int dim = min(GetClientSize().x, GetClientSize().y);
		int dim2 = max(dimensions.x, dimensions.y);

		float x1_m = (float)(preview_lines[a].x1() + 256) / (float)dim2;
		float x2_m = (float)(preview_lines[a].x2() + 256) / (float)dim2;
		float y1_m = (float)(preview_lines[a].y1() + 256) / (float)dim2;
		float y2_m = (float)(preview_lines[a].y2() + 256) / (float)dim2;

		glEnable(GL_LINE_SMOOTH);
		glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
		glBegin(GL_LINES);
		glVertex2d(x1_m * dim, y1_m * dim);
		glVertex2d(x2_m * dim, y2_m * dim);
		glEnd();
	}

	SwapBuffers();
}

void MapCanvas::onResize(wxSizeEvent &event)
{
	SetCurrent();

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetClientSize().x, GetClientSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	OnSize(event);
}


MapLumpArea::MapLumpArea(WadPanel *parent, Wad* wadfile)
:	LumpArea(parent)
{
	type = LAREA_MAP;
	this->wadfile = wadfile;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	map_canvas = new MapCanvas(this);
	vbox->Add(map_canvas, 1, wxEXPAND|wxALL, 4);
}

MapLumpArea::~MapLumpArea()
{
}

void MapLumpArea::checkSave()
{
}

void MapLumpArea::loadLump(Lump *lump)
{
	map_canvas->loadMap(wadfile, lump->getName());
	map_canvas->Refresh();
	map_canvas->Update();
}

