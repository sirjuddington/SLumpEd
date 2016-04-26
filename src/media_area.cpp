////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// media_area.cpp - MediaArea class functions                         //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "media_area.h"
#include "qmus2mid.h"
#include "misc.h"

wxStopWatch sw_media;

CVAR(Bool, media_autoplay, true, CVAR_SAVE)

MediaVisArea::MediaVisArea(wxWindow *parent)
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

	//Enable(false);
	Show(true);
	Raise();
}

MediaVisArea::~MediaVisArea()
{
}

BEGIN_EVENT_TABLE(MediaVisArea, wxGLCanvas)
	EVT_PAINT(MediaVisArea::onPaint)
	EVT_SIZE(MediaVisArea::onResize)
END_EVENT_TABLE()

void MediaVisArea::onPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	SetCurrent();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SwapBuffers();
}

void MediaVisArea::onResize(wxSizeEvent &event)
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

MediaLumpArea::MediaLumpArea(WadPanel *parent)
:	LumpArea(parent)
{
	type = LAREA_MEDIA;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	vis_area = new MediaVisArea(this);
	vbox->Add(vis_area, 1, wxEXPAND);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_playpause = new wxBitmapButton(this, MLA_BTN_PLAYPAUSE, wxBitmap(wxImage(_T("res/icons/play.png"), wxBITMAP_TYPE_PNG)));
	hbox->Add(btn_playpause, 0, wxEXPAND);

	btn_stop = new wxBitmapButton(this, MLA_BTN_STOP, wxBitmap(wxImage(_T("res/icons/stop.png"), wxBITMAP_TYPE_PNG)));
	hbox->Add(btn_stop, 0, wxEXPAND);

	sld_position = new wxSlider(this, MLA_SLD_POSITION, 0, 0, 10000);
	hbox->Add(sld_position, 1, wxEXPAND|wxLEFT, 4);

	music = NULL;
	sound = NULL;
}

MediaLumpArea::~MediaLumpArea()
{
	if (sound) FSOUND_Sample_Free(sound);
	if (music) FMUSIC_FreeSong(music);
}

void MediaLumpArea::loadLump(Lump *lump)
{
	sld_position->Enable(true);
	sld_position->SetValue(0);

	// Free sound
	if (sound)
	{
		FSOUND_StopSound(0);
		FSOUND_Sample_Free(sound);
		sound = NULL;
	}

	// Free music
	if (music)
	{
		FMUSIC_StopSong(music);
		FMUSIC_FreeSong(music);
		music = NULL;
	}



	// Doom Sound
	if (lump->getType() == LUMP_SOUND)
	{
		string wav = lump->getName(false, false) + _T(".wav");
		if (!dsnd_to_wav((char*)chr(wav), lump))
			return;

		sound = FSOUND_Sample_Load(FSOUND_FREE, chr(wav), 0, 0, 0);
		remove(chr(wav));

		sld_position->SetRange(0, FSOUND_Sample_GetLength(sound));
	}

	// WAV Sound
	if (lump->getType() == LUMP_WAV)
	{
		string wav = lump->getName(false, false) + _T(".wav");
		lump->dumpToFile(wav);
		sound = FSOUND_Sample_Load(FSOUND_FREE, chr(wav), 0, 0, 0);
		remove(chr(wav));

		sld_position->SetRange(0, FSOUND_Sample_GetLength(sound));
	}

	// MUS
	if (lump->getType() == LUMP_MUS)
	{
		string mus = lump->getName(false, false) + _T(".mus");
		string mid = lump->getName(false, false) + _T(".mid");
		lump->dumpToFile(mus);
		int err = qmus2mid(chr(mus), chr(mid), 1, 0, 128, 0);
		music = FMUSIC_LoadSong(chr(mid));
		remove(chr(mus));
		remove(chr(mid));

		sld_position->Enable(false);
	}

	// MIDI
	if (lump->getType() == LUMP_MIDI)
	{
		string mid = lump->getName(false, false) + _T(".mid");
		lump->dumpToFile(mid);
		music = FMUSIC_LoadSong(chr(mid));
		remove(chr(mid));

		sld_position->Enable(false);
	}

	// MOD (it, xm)
	if (lump->getType() == LUMP_MOD)
	{
		string mid = lump->getName(false, false) + _T(".mid");
		lump->dumpToFile(mid);
		music = FMUSIC_LoadSong(chr(mid));
		remove(chr(mid));

		sld_position->SetRange(0, FMUSIC_GetNumOrders(music));
	}



	// Autoplay if necessary
	if (media_autoplay)
	{
		if (sound)
			FSOUND_PlaySound(0, sound);
		else if (music)
			FMUSIC_PlaySong(music);

		btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/pause.png"), wxBITMAP_TYPE_PNG)));
	}
}

void MediaLumpArea::checkSave()
{
}

BEGIN_EVENT_TABLE(MediaLumpArea, wxPanel)
	EVT_IDLE(MediaLumpArea::onIdle)
	EVT_BUTTON(MLA_BTN_PLAYPAUSE, MediaLumpArea::onPlayPause)
	EVT_BUTTON(MLA_BTN_STOP, MediaLumpArea::onStop)
	EVT_COMMAND_SCROLL(MLA_SLD_POSITION, MediaLumpArea::onPositionChange)
END_EVENT_TABLE()

bool icon_change = false;
void MediaLumpArea::onIdle(wxIdleEvent &event)
{
	if (sound)
	{
		if (FSOUND_IsPlaying(0))
		{
			sld_position->SetValue(FSOUND_GetCurrentPosition(0));
			icon_change = true;
		}
		else
		{
			if (icon_change)
			{
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/play.png"), wxBITMAP_TYPE_PNG)));
				icon_change = false;
				sld_position->SetValue(0);
			}
		}

		event.RequestMore();
		return;
	}

	if (music)
	{
		if (FMUSIC_IsPlaying(music))
		{
			if (sld_position->IsEnabled())
				sld_position->SetValue(FMUSIC_GetOrder(music));

			icon_change = true;
		}
		else
		{
			if (icon_change)
			{
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/play.png"), wxBITMAP_TYPE_PNG)));
				icon_change = false;
				sld_position->SetValue(0);
			}
		}

		event.RequestMore();
	}
}

void MediaLumpArea::onPlayPause(wxCommandEvent &event)
{
	if (sound)
	{
		if (FSOUND_IsPlaying(0))
		{
			bool pause = FSOUND_GetPaused(0);

			if (pause)
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/pause.png"), wxBITMAP_TYPE_PNG)));
			else
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/play.png"), wxBITMAP_TYPE_PNG)));

			FSOUND_SetPaused(0, !pause);
		}
		else
		{
			FSOUND_SetPaused(0, 0);
			FSOUND_PlaySound(0, sound);
			FSOUND_SetCurrentPosition(0, sld_position->GetValue());
			btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/pause.png"), wxBITMAP_TYPE_PNG)));
		}

		return;
	}

	if (music)
	{
		if (FMUSIC_IsPlaying(music))
		{
			bool pause = FMUSIC_GetPaused(music);

			if (pause)
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/pause.png"), wxBITMAP_TYPE_PNG)));
			else
				btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/play.png"), wxBITMAP_TYPE_PNG)));

			FMUSIC_SetPaused(music, !pause);
		}
		else
		{
			FMUSIC_SetPaused(music, 0);
			FMUSIC_PlaySong(music);

			if (sld_position->IsEnabled())
				FMUSIC_SetOrder(music, sld_position->GetValue());

			btn_playpause->SetBitmapLabel(wxBitmap(wxImage(_T("res/icons/pause.png"), wxBITMAP_TYPE_PNG)));
		}

		return;
	}
}

void MediaLumpArea::onStop(wxCommandEvent &event)
{
	if (sound)
	{
		FSOUND_StopSound(0);
		icon_change = true;
		return;
	}

	if (music)
	{
		FMUSIC_StopSong(music);
		icon_change = true;
		return;
	}
}

void MediaLumpArea::onPositionChange(wxScrollEvent &event)
{
	if (sound)
		FSOUND_SetCurrentPosition(0, sld_position->GetValue());

	if (music)
		FMUSIC_SetOrder(music, sld_position->GetValue());
}
