////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006                                                //
// ------------------------------------------------------------------ //
// gfx_area.cpp - GfxArea class functions                             //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "gfx_area.h"
#include "misc.h"
#include "wad_panel.h"

wxColour*	global_palette;

CVAR(Bool, autosave_gfx, false, CVAR_SAVE)

GfxLumpArea::GfxLumpArea(WadPanel *parent, wxColour *pal)
:	LumpArea(parent)
{
	type = LAREA_GFX;

	if (pal)
		palette = pal;
	else
		palette = global_palette;

	image.setPalette(palette);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxBOTTOM, 4);

	btn_save = new wxButton(this, GA_BTN_SAVE, _T("Save Changes"));
	hbox->Add(btn_save, 0, wxEXPAND|wxRIGHT, 4);

	btn_zoomout = new wxButton(this, GA_BTN_ZOOMOUT, _T("-"), wxDefaultPosition, wxSize(24, -1), wxBU_EXACTFIT);
	btn_zoomin = new wxButton(this, GA_BTN_ZOOMIN, _T("+"), wxDefaultPosition, wxSize(24, -1), wxBU_EXACTFIT);

	hbox->Add(new wxStaticText(this, -1, _T("Zoom")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(btn_zoomout, 0, wxEXPAND, 0);
	hbox->Add(btn_zoomin, 0, wxEXPAND, 0);

	hbox->AddSpacer(4);

	entry_xoff = new wxTextCtrl(this, GA_ENTRY_XOFF, _T(""), wxDefaultPosition, wxSize(64, -1));
	entry_yoff = new wxTextCtrl(this, GA_ENTRY_YOFF, _T(""), wxDefaultPosition, wxSize(64, -1));

	hbox->Add(new wxStaticText(this, -1, _T("Offsets")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_xoff, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(entry_yoff, 0, wxEXPAND|wxRIGHT, 4);

	string offtypes[] =
	{
		_T("None"),
		_T("Normal"),
		_T("Hud/Weapon")
	};

	combo_offtype = new wxChoice(this, GA_COMBO_OFFTYPE, wxDefaultPosition, wxDefaultSize, 3, offtypes);
	hbox->Add(new wxStaticText(this, -1, _T("Offset Type")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(combo_offtype, 0, wxEXPAND);


	draw_area = new GfxBox(this);
	vbox->Add(draw_area, 1, wxEXPAND);
	draw_area->Refresh();
	draw_area->Update();

	change = false;

	Refresh();
	Layout();
}

GfxLumpArea::~GfxLumpArea()
{
}

void GfxLumpArea::loadLump(Lump* lump)
{
	change = false;
	bool allowchange = true;

	this->lump = lump;
	draw_area->setImage(NULL);
	image.setOffsets(0, 0);
	image.setPalette(palette);

	lump->checkGfxFormat();
	image.loadLump(lump);
	draw_area->setOffsets(image.xOff(), image.yOff());
	draw_area->setImage(image.toWxImage());

	draw_area->allowOffsetChange(false);
	entry_xoff->SetValue(wxString::Format(_T("%d"), image.xOff()));
	entry_yoff->SetValue(wxString::Format(_T("%d"), image.yOff()));
	draw_area->setEntries(entry_xoff, entry_yoff);
	draw_area->allowOffsetChange(allowchange);

	string otypestr = lump->getEx(_T("GfxOffsetType"));
	if (otypestr != _T(""))
	{
		int otype = atoi((char*)otypestr.ToAscii());
		draw_area->setOffType(otype);
	}

	combo_offtype->Select(draw_area->getOffType());

	draw_area->Refresh();
	draw_area->Update();
}

void GfxLumpArea::checkSave()
{
	if (!draw_area->changed())
		return;

	if (!autosave_gfx)
	{
		if (wxMessageBox(wxString::Format(_T("Save changes to %s?"), lump->getName()), _T("Text Changed"), wxICON_QUESTION|wxYES_NO) == wxNO)
			return;
	}

	if (lump->getType() == LUMP_PATCH || lump->getType() == LUMP_SPRITE || lump->getType() == LUMP_GFX)
		savePatch();

	if (lump->getType() == LUMP_PNG)
		savePNG();

	lump->setChanged(1, true);
	parent->updateList(lump);
}

void GfxLumpArea::readFlat()
{
	/*
	if (lump->getSize() == 64*64)
		image.Create(64, 64, true);
	else if (lump->getSize() == 320*200)
		image.Create(320, 200, true);
	else if (lump->getSize() == 64*128)
		image.Create(64, 128, true);
	else
		return;

	for (int a = 0; a < lump->getSize(); a++)
	{
		BYTE* data = image.GetData();
		BYTE col = lump->getData()[a];
		data[a*3] = palette[col].Red();
		data[(a*3)+1] = palette[col].Green();
		data[(a*3)+2] = palette[col].Blue();
	}

	draw_area->setOffsets(0, 0);
	draw_area->setImage(&image);
	*/
}

void GfxLumpArea::readPatch()
{
	/*
	// Get header & offsets
	patch_header_t *header = (patch_header_t *)lump->getData();
	long *col_offsets= (long *)((BYTE *)lump->getData() + sizeof(patch_header_t));

	image.Create(header->width, header->height, true);
	image.SetAlpha(NULL);
	memset(image.GetAlpha(), 0, header->width * header->height);

	// Read data
	for (int c = 0; c < header->width; c++)
	{
		// Go to start of column
		BYTE* data = lump->getData();
		data += col_offsets[c];

		// Read posts
		while (1)
		{
			// Get row offset
			BYTE row = *data;

			if (row == 255) // End of column?
				break;

			// Get no. of pixels
			data++;
			BYTE n_pix = *data;

			data++; // Skip buffer
			for (BYTE p = 0; p < n_pix; p++)
			{
				data++;
				image.SetRGB(c, row+p, palette[*data].Red(), palette[*data].Green(), palette[*data].Blue());
				image.SetAlpha(c, row+p, 255);
			}
			data += 2; // Skip buffer & go to next row offset
		}
	}

	draw_area->setOffsets(header->left, header->top);
	draw_area->setImage(&image);
	*/
}

void GfxLumpArea::savePatch()
{
	// At the moment it's only possible to change the offsets
	patch_header_t *header = (patch_header_t *)lump->getData();
	header->left = draw_area->getXOff();
	header->top = draw_area->getYOff();
}

void GfxLumpArea::savePNG()
{
	// Find offsets if present
	BYTE* data = lump->getData();
	int xoff = 0;
	int yoff = 0;
	for (int a = 0; a < lump->getSize(); a++)
	{
		if (data[a] == 'g' &&
			data[a+1] == 'r' &&
			data[a+2] == 'A' &&
			data[a+3] == 'b')
		{
			long xoff = wxINT32_SWAP_ON_LE(draw_area->getXOff());
			long yoff = wxINT32_SWAP_ON_LE(draw_area->getYOff());
			memcpy(data+a+4, &xoff, 4);
			memcpy(data+a+8, &yoff, 4);

			DWORD dcrc = wxUINT32_SWAP_ON_LE(crc(data+a, 12));
			memcpy(data+a+12, &dcrc, 4);

			return;
		}

		if (data[a] == 'I' &&
			data[a+1] == 'D' &&
			data[a+2] == 'A' &&
			data[a+3] == 'T')
			break;
	}

	FILE* fp = fopen("slumptemp", "wb");

	// Write PNG header and IHDR chunk
	fwrite(data, 33, 1, fp);

	struct grab_chunk_t
	{
		char name[4];
		long xoff;
		long yoff;
	};

	DWORD size = wxUINT32_SWAP_ON_LE(8);
	grab_chunk_t gc = { 'g', 'r', 'A', 'b', wxINT32_SWAP_ON_LE(draw_area->getXOff()), wxINT32_SWAP_ON_LE(draw_area->getYOff()) };
	DWORD dcrc = wxUINT32_SWAP_ON_LE(crc((BYTE*)&gc, 12));

	// Write grAb chunk
	fwrite(&size, 4, 1, fp);
	fwrite(&gc, 12, 1, fp);
	fwrite(&dcrc, 4, 1, fp);

	// Write the rest of the file
	fwrite(data + 33, lump->getSize() - 33, 1, fp);

	fclose(fp);

	lump->loadFile("slumptemp");
	remove("slumptemp");
}

BEGIN_EVENT_TABLE(GfxLumpArea, wxPanel)
	EVT_BUTTON(GA_BTN_ZOOMIN, GfxLumpArea::onZoomIn)
	EVT_BUTTON(GA_BTN_ZOOMOUT, GfxLumpArea::onZoomOut)
	EVT_BUTTON(GA_BTN_SAVE, GfxLumpArea::onSave)
	EVT_TEXT(GA_ENTRY_XOFF, GfxLumpArea::onXOffChanged)
	EVT_TEXT(GA_ENTRY_YOFF, GfxLumpArea::onYOffChanged)
	EVT_CHOICE(GA_COMBO_OFFTYPE, GfxLumpArea::onOffTypeChanged)
END_EVENT_TABLE()

void GfxLumpArea::onZoomIn(wxCommandEvent &event)
{
	draw_area->zoomIn();
	draw_area->Refresh();
	draw_area->Update();
}

void GfxLumpArea::onZoomOut(wxCommandEvent &event)
{
	draw_area->zoomOut();
	draw_area->Refresh();
	draw_area->Update();
}

void GfxLumpArea::onXOffChanged(wxCommandEvent &event)
{
	int i = 0;

	if (entry_xoff->GetValue() != _T(""))
		i = atoi(entry_xoff->GetValue());

	draw_area->setXOff(i);
	draw_area->Refresh();
	draw_area->Update();
}

void GfxLumpArea::onYOffChanged(wxCommandEvent &event)
{
	int i = 0;

	if (entry_yoff->GetValue() != _T(""))
		i = atoi(entry_yoff->GetValue());

	draw_area->setYOff(i);
	draw_area->Refresh();
	draw_area->Update();
}

void GfxLumpArea::onOffTypeChanged(wxCommandEvent &event)
{
	draw_area->setOffType(combo_offtype->GetSelection());
	draw_area->Refresh();
	draw_area->Update();

	string otype = wxString::Format(_T("%d"), draw_area->getOffType());
	lump->setEx(_T("GfxOffsetType"), otype);
}

void GfxLumpArea::onSave(wxCommandEvent &event)
{
	if (!draw_area->changed())
		return;

	if (lump->getType() == LUMP_PATCH || lump->getType() == LUMP_SPRITE || lump->getType() == LUMP_GFX)
		savePatch();

	if (lump->getType() == LUMP_PNG)
		savePNG();

	lump->setChanged(1);
	draw_area->setChanged(false);
	parent->updateList(lump);
}
