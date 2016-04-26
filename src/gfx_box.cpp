////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006                                                //
// ------------------------------------------------------------------ //
// gfx_box.cpp - GfxBox class functions                               //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "gfx_box.h"

int zoom_save = 1;

GfxBox::GfxBox(wxWindow *parent)
:	wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN)
{
	image = NULL;
	SetBackgroundColour(wxColour(0, 255, 255));
	zoom = zoom_save;
	x_offset = 0;
	y_offset = 0;
	allow_offset_change = true;
	moving = false;
	change = false;
	entry_xoff = NULL;
	entry_yoff = NULL;
}

GfxBox::~GfxBox()
{
	zoom_save = zoom;

	if (image)
		delete image;
}

void GfxBox::zoomIn()
{
	zoom++;
	if (zoom > 20)
		zoom = 20;
}

void GfxBox::zoomOut()
{
	zoom--;
	if (zoom < 1)
		zoom = 1;
}

void GfxBox::setImage(wxImage *image)
{
	if (this->image)
		delete this->image;

	this->image = image;
	Update();
	change = false;
}

void GfxBox::setOffsets(int x, int y)
{
	if (allow_offset_change)
	{
		x_offset = x;
		y_offset = y;
		change = true;

		if (!moving)
			detectOffsetType();
	}
}

void GfxBox::setXOff(int x)
{
	if (allow_offset_change)
	{
		x_offset = x;
		change = true;

		if (!moving)
			detectOffsetType();
	}
}

void GfxBox::setYOff(int y)
{
	if (allow_offset_change)
	{
		y_offset = y;
		change = true;

		if (!moving)
			detectOffsetType();
	}
}

void GfxBox::detectOffsetType()
{
	offset_type = OTYPE_NORMAL;

	if (x_offset == 0 && y_offset == 0)
		offset_type = OTYPE_NONE;

	if (x_offset <= 0 && y_offset < -32)
		offset_type = OTYPE_WEAPON;
}

void GfxBox::autoOffset(int type)
{
	if (type == -1)
		type = offset_type;

	if (type == OTYPE_NONE)
	{
		x_offset = 0;
		y_offset = 0;
	}

	if (type == OTYPE_NORMAL)
	{
		x_offset = -(this->image->GetWidth() / 2);
		y_offset = -(this->image->GetHeight() / 2);
	}

	if (type == OTYPE_MONSTER)
	{
		x_offset = -(this->image->GetWidth() / 2);
		y_offset = -image->GetHeight() + 4;
	}
}

BEGIN_EVENT_TABLE(GfxBox, wxPanel)
	EVT_PAINT(GfxBox::paint)
	EVT_MOUSE_EVENTS(GfxBox::mouseEvent)
END_EVENT_TABLE()

void GfxBox::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	int xoff = 0;
	int yoff = 0;
	double zoom = this->zoom;

	if (zoom == -1 && image)
	{
		double x_scale = 1.0;
		double y_scale = 1.0;

		if (image->GetWidth() > GetClientSize().x)
			x_scale = (double)GetClientSize().x / (double)image->GetWidth();

		if (image->GetHeight() > GetClientSize().y)
			y_scale = (double)GetClientSize().y / (double)image->GetHeight();

		if (x_scale < y_scale)
			zoom = x_scale;
		else
			zoom = y_scale;
	}

	if (offset_type == OTYPE_CENTERED && image)
	{
		int midx = GetClientSize().x / 2;
		int midy = GetClientSize().y / 2;
		xoff = midx - ((image->GetWidth() * zoom) / 2);
		yoff = midy - ((image->GetHeight() * zoom) / 2);
	}

	if (offset_type == OTYPE_NORMAL)
	{
		int midx = GetClientSize().x / 2;
		int midy = GetClientSize().y - (GetClientSize().y / 3);
		screen_origin.set(midx, midy);

		xoff = midx - (x_offset * zoom);
		yoff = midy - (y_offset * zoom);

		dc.DrawLine(0, midy, GetClientSize().x, midy);
		dc.DrawLine(midx, 0, midx, GetClientSize().y);
	}
	else if (offset_type == OTYPE_NONE)
	{
		xoff = 0;
		yoff = 0;
		screen_origin.set(0, 0);
	}
	else if (offset_type == OTYPE_WEAPON)
	{
		int midx = GetClientSize().x / 2;
		int midy = GetClientSize().y / 2;

		rect_t srect(midx, midy, 320*zoom, 200*zoom, 1);
		dc.DrawLine(srect.tl.x, srect.tl.y, srect.br.x, srect.tl.y);
		dc.DrawLine(srect.br.x, srect.tl.y, srect.br.x, srect.br.y);
		dc.DrawLine(srect.br.x, srect.br.y, srect.tl.x, srect.br.y);
		dc.DrawLine(srect.tl.x, srect.br.y, srect.tl.x, srect.tl.y);

		screen_origin.set(srect.tl);
		xoff = srect.tl.x - (x_offset * zoom);
		yoff = srect.tl.y - (y_offset * zoom);
	}

	if (image)
	{
		if (zoom == 1)
			dc.DrawBitmap(wxBitmap(*image), xoff, yoff, true);
		else
			dc.DrawBitmap(wxBitmap(image->Scale(image->GetWidth() * zoom, image->GetHeight() * zoom)), xoff, yoff, true);

		if (moving)
		{
			dc.SetBrush(wxBrush(_T("Black"), wxTRANSPARENT));
			dc.DrawRectangle(xoff, yoff, image->GetWidth() * zoom, image->GetHeight() * zoom);
		}
	}
}

void GfxBox::mouseEvent(wxMouseEvent &event)
{
	if (!allow_offset_change)
		return;

	if (event.ButtonDown(wxMOUSE_BTN_LEFT))
	{
		int xoff = screen_origin.x - (x_offset * zoom);
		int yoff = screen_origin.y - (y_offset * zoom);

		if (event.GetX() > xoff && event.GetX() < xoff + (image->GetWidth() * zoom) &&
			event.GetY() > yoff && event.GetY() < yoff + (image->GetHeight() * zoom))
		{
			move_origin.x = event.GetX();
			move_origin.y = event.GetY();
			orig_offset.x = x_offset;
			orig_offset.y = y_offset;
			moving = true;
			Refresh();
			Update();
		}

		return;
	}

	if (event.ButtonUp(wxMOUSE_BTN_LEFT))
	{
		moving = false;
		detectOffsetType();
		Refresh();
		Update();
		return;
	}

	if (moving && event.LeftIsDown())
	{
		x_offset = orig_offset.x - ((event.GetX() - move_origin.x) / zoom);
		y_offset = orig_offset.y - ((event.GetY() - move_origin.y) / zoom);
		change = true;

		if (entry_xoff)
			entry_xoff->SetValue(wxString::Format(_T("%d"), x_offset));

		if (entry_yoff)
			entry_yoff->SetValue(wxString::Format(_T("%d"), y_offset));

		Refresh();
		Update();
	}
}
