////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006                                                //
// ------------------------------------------------------------------ //
// lump_area.cpp - LumpArea class functions                           //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "lump_area.h"

LumpArea::LumpArea(WadPanel *parent)
:	wxPanel((wxWindow*)parent, -1)
{
	type = LAREA_NONE;
	this->parent = parent;
	//Hide();

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	label_lumptype = new wxStaticText(this, -1, _T(""));
	vbox->Add(label_lumptype, 1, wxALIGN_CENTER);
}

LumpArea::~LumpArea()
{
}

void LumpArea::loadLump(Lump* lump)
{
	this->lump = lump;
	label_lumptype->SetLabel(wxString::Format(_T("Lump Type: %s"), lump->getTypeString()));
}

void LumpArea::checkSave()
{
}
