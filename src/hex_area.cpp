
#include "main.h"
#include "hex_area.h"

HexLumpArea::HexLumpArea(WadPanel *parent)
:	LumpArea(parent)
{
	type = LAREA_HEX;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	text = new wxTextCtrl(this, HLA_TEXT, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_MULTILINE);
	vbox->Add(text, 1, wxEXPAND);
}

HexLumpArea::~HexLumpArea()
{
}

void HexLumpArea::checkSave()
{
}

void HexLumpArea::loadLump(Lump *lump)
{
	/*
	BYTE* data = lump->getData();
	text->Hide();

	int h = 0;
	for (int a = 0; a < lump->getSize(); a++)
	{
		text->AppendText(wxString::Format(_T("%3d "), data[a]));

		if (h == 16)
		{
			text->AppendText(_T("\n"));
			h = 0;
		}

		h++;
	}

	text->Show();
	*/
}
