////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// cfg_dialog.cpp - ConfigDialog class functions                      //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "cfg_dialog.h"

EXTERN_CVAR(String, iwad_path)
EXTERN_CVAR(String, path_acc)
EXTERN_CVAR(Bool, autosave_gfx)
EXTERN_CVAR(Bool, autosave_texturex)
EXTERN_CVAR(Bool, autosave_text)
EXTERN_CVAR(Bool, determine_type_load)
EXTERN_CVAR(Bool, save_logfile)
EXTERN_CVAR(Bool, media_autoplay)
EXTERN_CVAR(Bool, zip_askopenwad)
EXTERN_CVAR(Bool, force_uppercase)
EXTERN_CVAR(Bool, colour_lumps)
EXTERN_CVAR(Bool, lump_determine_gfx)
EXTERN_CVAR(Int, n_recent_wads)


ConfigDialog::ConfigDialog()
:	wxDialog(NULL, -1, _T("SLumpEd Options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Paths frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Paths"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// IWAD Path
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, _T("Game IWAD:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	text_iwadpath = new wxTextCtrl(this, CD_TEXT_IWADPATH);
	hbox->Add(text_iwadpath, 1, wxEXPAND|wxRIGHT, 4);
	btn_browseiwad = new wxButton(this, CD_BTN_BROWSEIWAD, _T("Browse"));
	hbox->Add(btn_browseiwad, 0, wxEXPAND);

	// ACC Path
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, _T("ACC:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	text_accpath = new wxTextCtrl(this, CD_TEXT_ACCPATH);
	hbox->Add(text_accpath, 1, wxEXPAND|wxRIGHT, 4);
	btn_browseacc = new wxButton(this, CD_BTN_BROWSEACC, _T("Browse"));
	hbox->Add(btn_browseacc, 0, wxEXPAND);

	// Options frame
	frame = new wxStaticBox(this, -1, _T("Misc Options"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Checkboxes
	int id = CD_CB_START;

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	cb_autosave_gfx = new wxCheckBox(this, id++, _T("Autosave gfx lumps"));
	hbox->Add(cb_autosave_gfx, 1, wxEXPAND|wxRIGHT, 4);
	cb_autosave_texturex = new wxCheckBox(this, id++, _T("Autosave TEXTUREx"));
	hbox->Add(cb_autosave_texturex, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	cb_autosave_text = new wxCheckBox(this, id++, _T("Autosave text lumps"));
	hbox->Add(cb_autosave_text, 1, wxEXPAND|wxRIGHT, 4);
	cb_determine_type_load = new wxCheckBox(this, id++, _T("Determine lump types on load"));
	hbox->Add(cb_determine_type_load, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	cb_save_logfile = new wxCheckBox(this, id++, _T("Save logfile"));
	hbox->Add(cb_save_logfile, 1, wxEXPAND|wxRIGHT, 4);
	cb_media_autoplay = new wxCheckBox(this, id++, _T("Autoplay media"));
	hbox->Add(cb_media_autoplay, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	cb_zip_askopenwad = new wxCheckBox(this, id++, _T("Ask when opening a wad from within a zip"));
	hbox->Add(cb_zip_askopenwad, 1, wxEXPAND|wxRIGHT, 4);
	cb_force_uppercase = new wxCheckBox(this, id++, _T("Force uppercase lump names"));
	hbox->Add(cb_force_uppercase, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);
	cb_colour_lumps = new wxCheckBox(this, id++, _T("Colour-coded lumps"));
	hbox->Add(cb_colour_lumps, 1, wxEXPAND|wxRIGHT, 4);
	cb_lump_determine_gfx = new wxCheckBox(this, id++, _T("Detect nonstandard gfx lumps"));
	hbox->Add(cb_lump_determine_gfx, 1, wxEXPAND);

	// OK Button
	m_vbox->Add(CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

	// Setup widgets
	text_iwadpath->SetValue(string(iwad_path));
	text_accpath->SetValue(string(path_acc));
	if (autosave_gfx) cb_autosave_gfx->SetValue(true);
	if (autosave_texturex) cb_autosave_texturex->SetValue(true);
	if (autosave_text) cb_autosave_text->SetValue(true);
	if (determine_type_load) cb_determine_type_load->SetValue(true);
	if (save_logfile) cb_save_logfile->SetValue(true);
	if (media_autoplay) cb_media_autoplay->SetValue(true);
	if (zip_askopenwad) cb_zip_askopenwad->SetValue(true);
	if (force_uppercase) cb_force_uppercase->SetValue(true);
	if (colour_lumps) cb_colour_lumps->SetValue(true);
	if (lump_determine_gfx) cb_lump_determine_gfx->SetValue(true);

	Layout();
	SetBestFittingSize();
}

ConfigDialog::~ConfigDialog()
{
}

BEGIN_EVENT_TABLE(ConfigDialog, wxDialog)
	EVT_TEXT(CD_TEXT_IWADPATH, ConfigDialog::onIwadPathChanged)
	EVT_TEXT(CD_TEXT_ACCPATH, ConfigDialog::onAccPathChanged)
	EVT_BUTTON(CD_BTN_BROWSEIWAD, ConfigDialog::onIwadBrowse)
	EVT_BUTTON(CD_BTN_BROWSEACC, ConfigDialog::onAccBrowse)

	// Why is there no EVT_CHECKBOX_RANGE? Irritating.
	EVT_CHECKBOX(CD_CB_START, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+1, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+2, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+3, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+4, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+5, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+6, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+7, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+8, ConfigDialog::onCbClicked)
	EVT_CHECKBOX(CD_CB_START+9, ConfigDialog::onCbClicked)
END_EVENT_TABLE()

void ConfigDialog::onIwadPathChanged(wxCommandEvent &event)
{
	iwad_path = chr(text_iwadpath->GetValue());
}

void ConfigDialog::onAccPathChanged(wxCommandEvent &event)
{
	path_acc = chr(text_accpath->GetValue());
}

void ConfigDialog::onIwadBrowse(wxCommandEvent &event)
{
	string path = wxFileSelector(_T("Browse For IWAD"), text_iwadpath->GetValue(),
									_T(""), _T("*.wad"), _T("WAD files (*.wad)|*.wad|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	text_iwadpath->SetValue(path);
}

void ConfigDialog::onAccBrowse(wxCommandEvent &event)
{
	string path = wxFileSelector(_T("Browse For ACC"), text_accpath->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	text_accpath->SetValue(path);
}

void ConfigDialog::onCbClicked(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case CD_CB_START:
		autosave_gfx = cb_autosave_gfx->IsChecked();
		break;
	case CD_CB_START+1:
		autosave_texturex = cb_autosave_texturex->IsChecked();
		break;
	case CD_CB_START+2:
		autosave_text = cb_autosave_text->IsChecked();
		break;
	case CD_CB_START+3:
		determine_type_load = cb_determine_type_load->IsChecked();
		break;
	case CD_CB_START+4:
		save_logfile = cb_save_logfile->IsChecked();
		break;
	case CD_CB_START+5:
		media_autoplay = cb_media_autoplay->IsChecked();
		break;
	case CD_CB_START+6:
		zip_askopenwad = cb_zip_askopenwad->IsChecked();
		break;
	case CD_CB_START+7:
		force_uppercase = cb_force_uppercase->IsChecked();
		break;
	case CD_CB_START+8:
		colour_lumps = cb_colour_lumps->IsChecked();
		break;
	case CD_CB_START+9:
		lump_determine_gfx = cb_lump_determine_gfx->IsChecked();
		break;
	}
}
