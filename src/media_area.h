
#include <wx/glcanvas.h>
#include "lump_area.h"
#include "fmod.h"

class MediaVisArea: public wxGLCanvas
{
private:

public:
	MediaVisArea(wxWindow* parent);
	~MediaVisArea();

	void onPaint(wxPaintEvent &event);
	void onResize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};

class MediaLumpArea : public LumpArea
{
private:
	wxBitmapButton	*btn_playpause;
	wxBitmapButton	*btn_stop;
	wxSlider		*sld_position;
	MediaVisArea	*vis_area;

	FSOUND_SAMPLE	*sound;
	FMUSIC_MODULE	*music;

public:
	enum
	{
		MLA_BTN_PLAYPAUSE,
		MLA_BTN_STOP,
		MLA_SLD_POSITION,
	};

	MediaLumpArea(WadPanel *parent);
	~MediaLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	void onIdle(wxIdleEvent &event);
	void onPlayPause(wxCommandEvent &event);
	void onStop(wxCommandEvent &event);
	void onPositionChange(wxScrollEvent &event);

	DECLARE_EVENT_TABLE()
};
