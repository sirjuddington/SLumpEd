////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// misc_dialog.cpp - Misc dialog classes                              //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "misc_dialog.h"

GfxOffsetDialog::GfxOffsetDialog(wxWindow *parent)
:	wxDialog(parent, -1, _T("Modify Gfx Offset(s)"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	opt_set = new wxRadioButton(this, GOD_OPT_SET, _T("Set Offsets"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	hbox->Add(opt_set, 1, wxEXPAND|wxALL, 4);

	entry_xoff = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(40, -1));
	entry_yoff = new wxTextCtrl(this, -2, _T(""), wxDefaultPosition, wxSize(40, -1));
	cbox_relative = new wxCheckBox(this, -1, _T("Relative"));
	hbox->Add(entry_xoff, 0, wxEXPAND|wxALL, 4);
	hbox->Add(entry_yoff, 0, wxEXPAND|wxALL, 4);
	hbox->Add(cbox_relative, 0, wxEXPAND|wxALL, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	opt_auto = new wxRadioButton(this, GOD_OPT_AUTO, _T("Automatic Offsets"));
	hbox->Add(opt_auto, 1, wxEXPAND|wxALL, 4);

	string offtypes[] =
	{
		_T("Monster"),
		_T("Projectile"),
		_T("Hud/Weapon")
	};

	combo_aligntype = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, offtypes);
	combo_aligntype->Select(0);
	combo_aligntype->Enable(false);
	hbox->Add(combo_aligntype, 0, wxEXPAND|wxALL, 4);

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	Layout();
	SetBestFittingSize();
}

GfxOffsetDialog::~GfxOffsetDialog()
{
}

point2_t GfxOffsetDialog::getOffset()
{
	long x = 0;
	long y = 0;
	entry_xoff->GetValue().ToLong(&x);
	entry_yoff->GetValue().ToLong(&y);

	return point2_t(x, y);
}

int	GfxOffsetDialog::getAlignType()
{
	return combo_aligntype->GetSelection();
}

int	GfxOffsetDialog::getOption()
{
	if (opt_auto->GetValue())
		return 1;
	else
		return 0;
}

bool GfxOffsetDialog::relativeOffset()
{
	return cbox_relative->GetValue();
}

bool GfxOffsetDialog::xOffChange()
{
	if (entry_xoff->GetValue() == _T(""))
		return false;
	else
		return true;
}

bool GfxOffsetDialog::yOffChange()
{
	if (entry_yoff->GetValue() == _T(""))
		return false;
	else
		return true;
}

BEGIN_EVENT_TABLE(GfxOffsetDialog, wxDialog)
	EVT_RADIOBUTTON(GOD_OPT_SET, GfxOffsetDialog::onOptSet)
	EVT_RADIOBUTTON(GOD_OPT_AUTO, GfxOffsetDialog::onOptAuto)
END_EVENT_TABLE()

void GfxOffsetDialog::onOptSet(wxCommandEvent &event)
{
	entry_xoff->Enable(true);
	entry_yoff->Enable(true);
	cbox_relative->Enable(true);
	combo_aligntype->Enable(false);
}

void GfxOffsetDialog::onOptAuto(wxCommandEvent &event)
{
	entry_xoff->Enable(false);
	entry_yoff->Enable(false);
	cbox_relative->Enable(false);
	combo_aligntype->Enable(true);
}






GfxConvertDialog::GfxConvertDialog(wxWindow *parent, int type, Image &img)
:	wxDialog(parent, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Transparency frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Transparency"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cbox_trans = new wxCheckBox(this, GCD_CBOX_TRANS, _T("Enable"));
	box->Add(cbox_trans, 0, wxEXPAND|wxALL, 4);

	opt_trans_existing = new wxRadioButton(this, -1, _T("Use Existing"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	box->Add(opt_trans_existing, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);

	opt_trans_col = new wxRadioButton(this, -1, _T("Single Colour: "));
	hbox->Add(opt_trans_col, 0, wxEXPAND|wxRIGHT, 4);

	entry_trans_col = new wxTextCtrl(this, -1, _T("247"));
	hbox->Add(entry_trans_col, 0, wxEXPAND);


	// Colours frame
	frame = new wxStaticBox(this, -1, _T("Colour Depth"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cbox_colours = new wxCheckBox(this, GCD_CBOX_COLOURS, _T("Don't change"));
	box->Add(cbox_colours, 0, wxEXPAND|wxALL, 4);

	opt_32bpp = new wxRadioButton(this, -1, _T("32BPP (RGBA)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	box->Add(opt_32bpp, 0, wxEXPAND|wxALL, 4);

	opt_256col = new wxRadioButton(this, -1, _T("8BPP (256 Colours)"));
	box->Add(opt_256col, 0, wxEXPAND|wxALL, 4);

	opt_pal_current = new wxRadioButton(this, -1, _T("Current Palette"));
	box->Add(opt_pal_current, 0, wxEXPAND|wxALL, 4);


	// Buttons
	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);


	cbox_trans->SetValue(false);
	opt_trans_existing->Enable(false);
	opt_trans_col->Enable(false);
	entry_trans_col->Enable(false);

	cbox_colours->SetValue(true);
	opt_32bpp->Enable(false);
	opt_256col->Enable(false);
	opt_pal_current->Enable(false);

	if (type == 0)
	{
		cbox_colours->SetValue(false);
		cbox_colours->Enable(false);
		opt_pal_current->Enable(true);
		opt_pal_current->SetValue(true);
	}

	Layout();
	SetBestFittingSize();
}

GfxConvertDialog::~GfxConvertDialog()
{
}

int GfxConvertDialog::getTrans()
{
	if (!cbox_trans->GetValue())
		return -1;

	if (opt_trans_existing->GetValue())
		return 0;
	else if (opt_trans_col->GetValue())
		return 1;
}

int GfxConvertDialog::getTransCol()
{
	return atoi(chr(entry_trans_col->GetValue()));
}

int GfxConvertDialog::getColDepth()
{
	if (cbox_colours->GetValue())
		return -1;

	if (opt_32bpp->GetValue())
		return 0;
	else if (opt_256col->GetValue())
		return 1;
	else if (opt_pal_current->GetValue())
		return 2;
}


BEGIN_EVENT_TABLE(GfxConvertDialog, wxDialog)
	EVT_CHECKBOX(GCD_CBOX_TRANS, GfxConvertDialog::onCboxTrans)
	EVT_CHECKBOX(GCD_CBOX_COLOURS, GfxConvertDialog::onCboxColours)
END_EVENT_TABLE()

void GfxConvertDialog::onCboxTrans(wxCommandEvent &event)
{
	opt_trans_existing->Enable(cbox_trans->GetValue());
	opt_trans_col->Enable(cbox_trans->GetValue());
	entry_trans_col->Enable(cbox_trans->GetValue());
}

void GfxConvertDialog::onCboxColours(wxCommandEvent &event)
{
	opt_32bpp->Enable(!cbox_colours->GetValue());
	opt_256col->Enable(!cbox_colours->GetValue());
	opt_pal_current->Enable(!cbox_colours->GetValue());
}
