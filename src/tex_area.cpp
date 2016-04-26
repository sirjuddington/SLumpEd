////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// tex_area.cpp - TEXTUREx editor UI functions                        //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "tex_area.h"
#include "misc.h"
#include "image.h"
#include "wad_panel.h"

CVAR(Bool, autosave_texturex, false, CVAR_SAVE)

extern Wad iwad;
extern wxColour* palette;

TextureArea::TextureArea(TextureLumpArea *parent, Wad *wadfile, wxColour* pal)
:	wxPanel((wxWindow*)parent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxWANTS_CHARS)
{
	SetBackgroundColour(wxColour(50, 50, 50));
	this->wadfile = wadfile;
	texture = NULL;
	sel_patch = -1;
	palette = pal;
	moving = false;
	zoom = 1.0f;
	this->parent = parent;
}

TextureArea::~TextureArea()
{
}

void TextureArea::setTexture(texture_t *tex)
{
	if (texture == tex)
		return;

	for (int a = 0; a < images.size(); a++)
		delete images[a];

	images.clear();

	texture = tex;

	if (!tex)
		return;

	for (int a = 0; a < tex->patches.size(); a++)
	{
		Wad* wad = wadfile;
		wxImage *image = NULL;
		Lump* lump = wadfile->getLump(tex->patches[a].patch, 0);

		if (!lump)
		{
			lump = iwad.getLump(tex->patches[a].patch, 0);
			wad = &iwad;
		}

		//wad->getLump(tex->patches[a].patch, 0)->determineType();
		if (lump)
			lump->determineType();
		else
			wxMessageBox(wxString::Format(_T("Patch %s not found! Make sure you have the right IWAD selected"), tex->patches[a].patch));

		Image i;
		i.setPalette(palette);
		i.loadLump(lump);
		image = i.toWxImage();
		images.push_back(image);
	}
}

void TextureArea::changeZoom(float amount)
{
	zoom += amount;

	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 20.0f)
		zoom = 20.0f;

	Refresh();
	Update();
}

BEGIN_EVENT_TABLE(TextureArea, wxPanel)
	EVT_PAINT(TextureArea::paint)
	EVT_MOUSE_EVENTS(TextureArea::mouseEvent)
	EVT_KEY_DOWN(TextureArea::onKeyDown)
END_EVENT_TABLE()

void TextureArea::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	if (texture)
	{
		screen_origin.set((GetClientSize().x / 2) - (texture->width / 2) * zoom,
							(GetClientSize().y / 2) - (texture->height / 2) * zoom);

		for (int a = 0; a < texture->patches.size(); a++)
		{
			int xoff = screen_origin.x + (texture->patches[a].xoff) * zoom;
			int yoff = screen_origin.y + (texture->patches[a].yoff) * zoom;

			if (zoom == 1)
				dc.DrawBitmap(wxBitmap(*images[a]), xoff, yoff, true);
			else
				dc.DrawBitmap(wxBitmap(images[a]->Scale(images[a]->GetWidth() * zoom, images[a]->GetHeight() * zoom)), xoff, yoff, true);
		}

		dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawRectangle(screen_origin.x, screen_origin.y, texture->width * zoom, texture->height * zoom);

		if (sel_patch >= 0 && sel_patch < texture->patches.size())
		{
			dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
			dc.SetPen(*wxRED_PEN);
			dc.DrawRectangle(screen_origin.x + (texture->patches[sel_patch].xoff * zoom),
							screen_origin.y + (texture->patches[sel_patch].yoff * zoom),
				images[sel_patch]->GetWidth() * zoom, images[sel_patch]->GetHeight() * zoom);
		}
	}
}

void TextureArea::onKeyDown(wxKeyEvent &event)
{
	int key = event.GetKeyCode();

	if (!texture)
		return;

	if (sel_patch != -1)
	{
		// Delete selected patch
		if (key == WXK_DELETE)
			parent->removePatch(sel_patch);

		// Move selected patch (x8)
		if (key == WXK_UP)
		{
			texture->patches[sel_patch].yoff -= 8;
			parent->change = true;
		}

		if (key == WXK_DOWN)
		{
			texture->patches[sel_patch].yoff += 8;
			parent->change = true;
		}

		if (!event.ControlDown())
		{
			if (key == WXK_LEFT)
			{
				texture->patches[sel_patch].xoff -= 8;
				parent->change = true;
			}

			if (key == WXK_RIGHT)
			{
				texture->patches[sel_patch].xoff += 8;
				parent->change = true;
			}
		}
		else
		{
			if (key == 'B')
				parent->orderPatch(sel_patch);

			if (key == 'F')
				parent->orderPatch(sel_patch, false);
		}

		Refresh();
		Update();
	}

	// Cycle through patches
	if (event.ControlDown())
	{
		if (key == WXK_LEFT)
		{
			sel_patch++;

			if (sel_patch >= texture->patches.size())
				sel_patch = 0;

			Refresh();
			Update();
		}

		if (key == WXK_RIGHT)
		{
			sel_patch--;

			if (sel_patch < 0)
				sel_patch = texture->patches.size() - 1;

			Refresh();
			Update();
		}
	}
}

void TextureArea::mouseEvent(wxMouseEvent &event)
{
	if (!texture)
		return;

	if (event.ButtonDown(wxMOUSE_BTN_LEFT))
	{
		SetFocusFromKbd();

		if (sel_patch != -1)
		{
			int xoff = screen_origin.x + (texture->patches[sel_patch].xoff * zoom);
			int yoff = screen_origin.y + (texture->patches[sel_patch].yoff * zoom);

			if (event.GetX() > xoff && event.GetX() < xoff + (images[sel_patch]->GetWidth() * zoom) &&
				event.GetY() > yoff && event.GetY() < yoff + (images[sel_patch]->GetHeight() * zoom))
			{
				move_origin.x = event.GetX();
				move_origin.y = event.GetY();
				orig_offset.x = texture->patches[sel_patch].xoff;
				orig_offset.y = texture->patches[sel_patch].yoff;
				moving = true;
				Refresh();
				Update();
			}
			else
				sel_patch = -1;
		}

		if (sel_patch == -1)
		{
			for (int a = texture->patches.size()-1; a >= 0; a--)
			{
				int xoff = screen_origin.x + (texture->patches[a].xoff * zoom);
				int yoff = screen_origin.y + (texture->patches[a].yoff * zoom);

				if (event.GetX() > xoff && event.GetX() < xoff + (images[a]->GetWidth() * zoom) &&
					event.GetY() > yoff && event.GetY() < yoff + (images[a]->GetHeight() * zoom))
				{
					move_origin.x = event.GetX();
					move_origin.y = event.GetY();
					orig_offset.x = texture->patches[a].xoff;
					orig_offset.y = texture->patches[a].yoff;
					moving = true;
					sel_patch = a;
					Refresh();
					Update();
					break;
				}
			}
		}

		return;
	}

	if (event.ButtonUp(wxMOUSE_BTN_LEFT))
	{
		moving = false;
		parent->changePatch(sel_patch);
		Refresh();
		Update();
		return;
	}

	if (moving && event.LeftIsDown())
	{
		texture->patches[sel_patch].xoff = orig_offset.x + ((event.GetX() - move_origin.x) / zoom);
		texture->patches[sel_patch].yoff = orig_offset.y + ((event.GetY() - move_origin.y) / zoom);

		Refresh();
		Update();
		parent->change = true;
	}
}


TextureLumpArea::TextureLumpArea(WadPanel *parent, Wad* wadfile, wxColour* pal)
:	LumpArea(parent)
{
	type = LAREA_TEXTURES;
	this->wadfile = wadfile;
	this->palette = pal;
	change = false;

	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Textures frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Textures"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_hbox->Add(box, 0, wxEXPAND|wxALL, 4);

	list_textures = new wxListBox(this, TLA_LIST_TEXTURES, wxDefaultPosition, wxSize(96, -1));
	box->Add(list_textures, 1, wxEXPAND|wxALL, 4);

	btn_newtexture = new wxButton(this, TLA_BTN_NEWTEXTURE, _T("New Texture"));
	box->Add(btn_newtexture, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);

	btn_rentexture = new wxButton(this, TLA_BTN_RENTEXTURE, _T("Rename Texture"));
	box->Add(btn_rentexture, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);

	btn_deltexture = new wxButton(this, TLA_BTN_DELTEXTURE, _T("Delete Texture"));
	box->Add(btn_deltexture, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);

	// Texture draw area
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	m_hbox->Add(vbox, 1, wxEXPAND);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_zoomout = new wxButton(this, TLA_BTN_ZOOMOUT, _T("-"), wxDefaultPosition, wxSize(24, -1), wxBU_EXACTFIT);
	btn_zoomin = new wxButton(this, TLA_BTN_ZOOMIN, _T("+"), wxDefaultPosition, wxSize(24, -1), wxBU_EXACTFIT);

	hbox->Add(new wxStaticText(this, -1, _T("Zoom")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(btn_zoomout, 0, wxEXPAND, 0);
	hbox->Add(btn_zoomin, 0, wxEXPAND|wxRIGHT, 4);

	entry_texwidth = new wxTextCtrl(this, TLA_ENTRY_TEXWIDTH, _T(""), wxDefaultPosition, wxSize(32, -1));
	entry_texheight = new wxTextCtrl(this, TLA_ENTRY_TEXHEIGHT, _T(""), wxDefaultPosition, wxSize(32, -1));

	hbox->Add(new wxStaticText(this, -1, _T("Size: ")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_texwidth, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(entry_texheight, 0, wxEXPAND|wxRIGHT, 4);

	entry_xscale = new wxTextCtrl(this, TLA_ENTRY_XSCALE, _T(""), wxDefaultPosition, wxSize(32, -1));
	entry_yscale = new wxTextCtrl(this, TLA_ENTRY_YSCALE, _T(""), wxDefaultPosition, wxSize(32, -1));

	hbox->Add(new wxStaticText(this, -1, _T("Scale: ")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_xscale, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(entry_yscale, 0, wxEXPAND|wxRIGHT, 4);

	label_scalesize = new wxStaticText(this, -1, _T(""));
	hbox->Add(label_scalesize, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 4);

	ta_texture = new TextureArea(this, wadfile, pal);
	vbox->Add(ta_texture, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	m_hbox->Add(vbox, 0, wxEXPAND);

	// PNames frame
	frame = new wxStaticBox(this, -1, _T("PNames"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	vbox->Add(box, 2, wxEXPAND|wxALL, 4);

	list_pnames = new wxListBox(this, TLA_LIST_PNAMES, wxDefaultPosition, wxSize(96, -1));
	box->Add(list_pnames, 1, wxEXPAND|wxALL, 4);

	gbox_patch = new GfxBox(this);
	gbox_patch->Enable(false);
	gbox_patch->SetBackgroundColour(wxColour(50, 50, 50));
	gbox_patch->setZoom(-1);
	gbox_patch->setOffType(OTYPE_CENTERED);
	box->Add(gbox_patch, 0, wxEXPAND|wxALL, 4);

	btn_addpatch = new wxButton(this, TLA_BTN_ADDPATCH, _T("Add Patch"));
	box->Add(btn_addpatch, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);

	// Read PNames
	pnames.clear();
	DWORD n_pnames = 0;
	Wad* wad = wadfile;
	Lump *lump = wad->getLump(_T("PNAMES"), 0);

	if (!lump)
	{
		lump = iwad.getLump(_T("PNAMES"), 0);
		wad = &iwad;
	}

	if (lump)
	{
		BYTE* data = lump->getData();
		n_pnames = *((DWORD*)data);

		DWORD a = 4;
		for (DWORD p = 0; p < n_pnames; p++)
		{
			char name[9] = "";
			memset(name, 0, 9);
			memcpy(name, (data + a), 8);
			pnames.push_back(wxString::FromAscii(name));
			list_pnames->Append(pnames.back());
			a += 8;
		}
	}

	// Patches frame
	frame = new wxStaticBox(this, -1, _T("Patches"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	vbox->Add(box, 1, wxEXPAND|wxALL, 4);

	list_patches = new wxListBox(this, TLA_LIST_PATCHES, wxDefaultPosition, wxSize(96, -1));
	box->Add(list_patches, 1, wxEXPAND|wxALL, 4);

	entry_patchxoff = new wxTextCtrl(this, TLA_ENTRY_PATCHXOFF, _T(""), wxDefaultPosition, wxSize(32, -1));
	entry_patchyoff = new wxTextCtrl(this, TLA_ENTRY_PATCHYOFF, _T(""), wxDefaultPosition, wxSize(32, -1));

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	hbox->Add(new wxStaticText(this, -1, _T("X Pos:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_patchxoff, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	hbox->Add(new wxStaticText(this, -1, _T("Y Pos:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(entry_patchyoff, 1, wxEXPAND);

	btn_removepatch = new wxButton(this, TLA_BTN_REMOVEPATCH, _T("Remove Patch"));
	box->Add(btn_removepatch, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	entry_patchxoff->Enable(false);
	entry_patchyoff->Enable(false);

	Layout();
	gbox_patch->SetSizeHints(-1, gbox_patch->GetSize().x, -1, gbox_patch->GetSize().x);

	Layout();
	ta_texture->Refresh();
	ta_texture->Update();
}

TextureLumpArea::~TextureLumpArea()
{
}

void TextureLumpArea::changePatch(int index)
{
	list_patches->Select(index);

	if (index == -1)
	{
		entry_patchxoff->Enable(false);
		entry_patchyoff->Enable(false);
		entry_patchxoff->SetValue(_T(""));
		entry_patchyoff->SetValue(_T(""));
	}
	else
	{
		patch_info_t p = textures[list_textures->GetSelection()].patches[index];

		entry_patchxoff->Enable(true);
		entry_patchyoff->Enable(true);
		entry_patchxoff->SetValue(wxString::Format(_T("%d"), p.xoff));
		entry_patchyoff->SetValue(wxString::Format(_T("%d"), p.yoff));
	}
}

void TextureLumpArea::loadLump(Lump *lump)
{
	list_textures->Clear();
	list_patches->Clear();
	textures.clear();

	this->lump = lump;
	load_texturex(lump, textures, pnames);

	for (int a = 0; a < textures.size(); a++)
		list_textures->Append(textures[a].name);
}

void TextureLumpArea::checkSave()
{
	if (!change)
		return;

	if (!autosave_texturex)
	{
		if (wxMessageBox(wxString::Format(_T("Save changes to %s?"), lump->getName()), _T("Lump Changed"), wxICON_QUESTION|wxYES_NO) == wxNO)
			return;
	}

	save_texturex(lump, textures);
	parent->updateList(lump);
}

void TextureLumpArea::addPatch(int index)
{
	patch_info_t p;
	p.xoff = 0;
	p.yoff = 0;
	if (index < pnames.size()) p.patch = pnames[index];
	p.patch_index = index;
	textures[list_textures->GetSelection()].patches.push_back(p);

	ta_texture->setTexture(NULL);
	onTextureListChange(wxCommandEvent());

	change = true;
}

void TextureLumpArea::removePatch(int index)
{
	texture_t *tex = &textures[list_textures->GetSelection()];
	tex->patches.erase(tex->patches.begin() + index);

	ta_texture->setTexture(NULL);
	onTextureListChange(wxCommandEvent());

	change = true;
}

void TextureLumpArea::orderPatch(int index, bool up)
{
	texture_t *tex = &textures[list_textures->GetSelection()];
	int nindex = -1;

	if (up && index > 0)
		nindex = index - 1;
	else if (!up && index < tex->patches.size() - 1)
		nindex = index + 1;
	else
		return;

	patch_info_t p = tex->patches[index];
	tex->patches[index] = tex->patches[nindex];
	tex->patches[nindex] = p;

	ta_texture->setTexture(NULL);
	onTextureListChange(wxCommandEvent());
	ta_texture->selPatch(nindex);

	change = true;
}

void TextureLumpArea::updateScaleLabel()
{
	int sel = list_textures->GetSelection();
	if (sel == -1)
		return;

	int xs = textures[sel].width;
	if (textures[sel].x_scale != 0) xs = (int)(textures[sel].width / ((float)textures[sel].x_scale / 8.0f));
	int ys = textures[sel].height;
	if (textures[sel].y_scale != 0) ys = (int)(textures[sel].height / ((float)textures[sel].y_scale / 8.0f));

	label_scalesize->SetLabel(wxString::Format(_T("Scaled Size: %dx%d"), xs, ys));
}


BEGIN_EVENT_TABLE(TextureLumpArea, wxPanel)
	EVT_LISTBOX(TLA_LIST_TEXTURES, TextureLumpArea::onTextureListChange)
	EVT_LISTBOX(TLA_LIST_PATCHES, TextureLumpArea::onPatchesListChange)
	EVT_LISTBOX(TLA_LIST_PNAMES, TextureLumpArea::onPNamesListChange)
	EVT_BUTTON(TLA_BTN_ZOOMIN, TextureLumpArea::onZoomIn)
	EVT_BUTTON(TLA_BTN_ZOOMOUT, TextureLumpArea::onZoomOut)
	EVT_TEXT(TLA_ENTRY_TEXWIDTH, TextureLumpArea::onWidthChanged)
	EVT_TEXT(TLA_ENTRY_TEXHEIGHT, TextureLumpArea::onHeightChanged)
	EVT_TEXT(TLA_ENTRY_PATCHXOFF, TextureLumpArea::onXOffChanged)
	EVT_TEXT(TLA_ENTRY_PATCHYOFF, TextureLumpArea::onYOffChanged)
	EVT_BUTTON(TLA_BTN_ADDPATCH, TextureLumpArea::onAddPatch)
	EVT_BUTTON(TLA_BTN_REMOVEPATCH, TextureLumpArea::onRemovePatch)
	EVT_BUTTON(TLA_BTN_NEWTEXTURE, TextureLumpArea::onNewTexture)
	EVT_BUTTON(TLA_BTN_DELTEXTURE, TextureLumpArea::onDelTexture)
	EVT_BUTTON(TLA_BTN_RENTEXTURE, TextureLumpArea::onRenTexture)
	EVT_TEXT(TLA_ENTRY_XSCALE, TextureLumpArea::onXScaleChanged)
	EVT_TEXT(TLA_ENTRY_YSCALE, TextureLumpArea::onYScaleChanged)
END_EVENT_TABLE()

void TextureLumpArea::onTextureListChange(wxCommandEvent &event)
{
	list_patches->Clear();

	texture_t *tex = &textures[list_textures->GetSelection()];

	ta_texture->setTexture(tex);
	ta_texture->selPatch(-1);
	ta_texture->Refresh();
	ta_texture->Update();

	bool exchange = change;
	entry_texwidth->SetValue(wxString::Format(_T("%d"), tex->width));
	entry_texheight->SetValue(wxString::Format(_T("%d"), tex->height));

	for (int a = 0; a < tex->patches.size(); a++)
		list_patches->Append(tex->patches[a].patch);

	updateScaleLabel();
	entry_xscale->SetValue(wxString::Format(_T("%d"), tex->x_scale));
	entry_yscale->SetValue(wxString::Format(_T("%d"), tex->y_scale));

	change = exchange;
}

void TextureLumpArea::onPatchesListChange(wxCommandEvent &event)
{
	ta_texture->selPatch(list_patches->GetSelection());
	ta_texture->Refresh();
	ta_texture->Update();

	if (list_patches->GetSelection() == -1)
	{
		entry_patchxoff->Enable(false);
		entry_patchyoff->Enable(false);
		entry_patchxoff->SetValue(_T(""));
		entry_patchyoff->SetValue(_T(""));
	}
	else
	{
		patch_info_t p = textures[list_textures->GetSelection()].patches[list_patches->GetSelection()];

		entry_patchxoff->Enable(true);
		entry_patchyoff->Enable(true);
		entry_patchxoff->SetValue(wxString::Format(_T("%d"), p.xoff));
		entry_patchyoff->SetValue(wxString::Format(_T("%d"), p.yoff));
	}
}

void TextureLumpArea::onZoomIn(wxCommandEvent &event)
{
	ta_texture->changeZoom(1);
}

void TextureLumpArea::onZoomOut(wxCommandEvent &event)
{
	ta_texture->changeZoom(-1);
}

void TextureLumpArea::onWidthChanged(wxCommandEvent &event)
{
	if (entry_texwidth->GetValue() != _T(""))
	{
		textures[list_textures->GetSelection()].width = atoi(entry_texwidth->GetValue().c_str());
		ta_texture->Refresh();
		ta_texture->Update();
		updateScaleLabel();
		change = true;
	}
}

void TextureLumpArea::onHeightChanged(wxCommandEvent &event)
{
	if (entry_texheight->GetValue() != _T(""))
	{
		textures[list_textures->GetSelection()].height = atoi(entry_texheight->GetValue().c_str());
		ta_texture->Refresh();
		ta_texture->Update();
		updateScaleLabel();
		change = true;
	}
}

void TextureLumpArea::onXOffChanged(wxCommandEvent &event)
{
	if (list_patches->GetSelection() == -1)
		return;

	if (entry_patchxoff->GetValue() != _T(""))
	{
		textures[list_textures->GetSelection()].patches[list_patches->GetSelection()].xoff = atoi(entry_patchxoff->GetValue().c_str());
		ta_texture->Refresh();
		ta_texture->Update();
		change = true;
	}
}

void TextureLumpArea::onYOffChanged(wxCommandEvent &event)
{
	if (list_patches->GetSelection() == -1)
		return;

	if (entry_patchyoff->GetValue() != _T(""))
	{
		textures[list_textures->GetSelection()].patches[list_patches->GetSelection()].yoff = atoi(entry_patchyoff->GetValue().c_str());
		ta_texture->Refresh();
		ta_texture->Update();
		change = true;
	}
}

void TextureLumpArea::onAddPatch(wxCommandEvent &event)
{
	if (list_textures->GetSelection() == -1 ||
		list_pnames->GetSelection() == -1)
		return;

	addPatch(list_pnames->GetSelection());
}

void TextureLumpArea::onRemovePatch(wxCommandEvent &event)
{
	if (list_textures->GetSelection() == -1 ||
		list_patches->GetSelection() == -1)
		return;

	removePatch(list_patches->GetSelection());
}

void TextureLumpArea::onNewTexture(wxCommandEvent &event)
{
	string name = wxGetTextFromUser(_T("Enter name for new texture:"), _T("New Texture"));

	if (name == _T(""))
		return;

	name.UpperCase();
	name.Truncate(8);

	texture_t tex;
	tex.width = 64;
	tex.height = 128;
	tex.name = name;
	tex.flags = 0;
	tex.x_scale = 0;
	tex.y_scale = 0;

	textures.push_back(tex);
	list_textures->Append(name);
	list_textures->Select(list_textures->GetCount() - 1);

	ta_texture->setTexture(NULL);
	onTextureListChange(wxCommandEvent());
	change = true;
}

void TextureLumpArea::onDelTexture(wxCommandEvent &event)
{
	int sel = list_textures->GetSelection();

	if (sel == -1)
		return;

	textures.erase(textures.begin() + sel);
	list_textures->Delete(sel);
	list_textures->Select(sel);

	ta_texture->setTexture(NULL);
	onTextureListChange(wxCommandEvent());
	change = true;
}

void TextureLumpArea::onRenTexture(wxCommandEvent &event)
{
	int sel = list_textures->GetSelection();

	if (sel == -1)
		return;

	string name = wxGetTextFromUser(_T("Enter new name for texture:"), _T("Rename Texture"));

	if (name == _T(""))
		return;

	name.UpperCase();
	name.Truncate(8);

	textures[sel].name = name;
	list_textures->SetString(sel, name);
	change = true;
}

void TextureLumpArea::onXScaleChanged(wxCommandEvent &event)
{
	int sel = list_textures->GetSelection();
	if (sel == -1)
		return;

	if (entry_xscale->GetValue() == _T(""))
		textures[sel].x_scale = 0;
	else
	{
		int temp = atoi(entry_xscale->GetValue().ToAscii());

		if (temp > 255)
			temp = 255;
		if (temp < 0)
			temp = 0;

		textures[sel].x_scale = temp;
	}

	updateScaleLabel();
	change = true;
}

void TextureLumpArea::onYScaleChanged(wxCommandEvent &event)
{
	int sel = list_textures->GetSelection();
	if (sel == -1)
		return;

	if (entry_yscale->GetValue() == _T(""))
		textures[sel].y_scale = 0;
	else
	{
		int temp = atoi(entry_yscale->GetValue().ToAscii());

		if (temp > 255)
			temp = 255;
		if (temp < 0)
			temp = 0;

		textures[sel].y_scale = temp;
	}

	updateScaleLabel();
	change = true;
}

void TextureLumpArea::onPNamesListChange(wxCommandEvent &event)
{
	if (list_pnames->GetSelection() == -1 ||
		list_pnames->GetSelection() >= pnames.size())
		return;

	Lump* l = wadfile->getLump(pnames[list_pnames->GetSelection()]);

	if (!l)
		l = iwad.getLump(pnames[list_pnames->GetSelection()]);

	if (!l)
		gbox_patch->setImage(NULL);
	else
	{
		l->determineType();
		Image img;
		img.setPalette(palette);
		img.loadLump(l);
		gbox_patch->setImage(img.toWxImage());
	}

	gbox_patch->Refresh();
	gbox_patch->Update();
}
