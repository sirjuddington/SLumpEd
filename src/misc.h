
#ifndef __MISC_H__
#define __MISC_H__

#include <wx/image.h>

// rgba_t: A 32-bit colour definition
struct rgba_t
{
	BYTE r, g, b, a;
	char blend;

	// Constructors
	rgba_t() { r = 0; g = 0; b = 0; a = 0; blend = -1; }
	
	rgba_t(BYTE R, BYTE G, BYTE B, BYTE A, char BLEND = -1)
	{
		r = R;
		g = G;
		b = B;
		a = A;
		blend = BLEND;
	}

	// Functions
	void set(BYTE R, BYTE G, BYTE B, BYTE A, char BLEND = -1)
	{
		r = R;
		g = G;
		b = B;
		a = A;
		blend = BLEND;
	}

	void set(rgba_t colour)
	{
		r = colour.r;
		g = colour.g;
		b = colour.b;
		a = colour.a;
		blend = colour.blend;
	}

	float fr() { return (float)r / 255.0f; }
	float fg() { return (float)g / 255.0f; }
	float fb() { return (float)b / 255.0f; }
	float fa() { return (float)a / 255.0f; }

	wxColor to_wx_color()
	{
		return wxColour(r, g, b);
	}

	/*
	GdkColor to_gdk_color()
	{
		GdkColor ret;
		ret.red = r * 256;
		ret.green = g * 256;
		ret.blue = b * 256;

		return ret;
	}
	*/

	void set_blend()
	{
		if (blend == 0)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
};

#define COL_BLEND_DONTCARE	-1
#define COL_BLEND_NORMAL	0
#define COL_BLEND_ADD		1

// point2_t: A 2d point
struct point2_t
{
	int x, y;

	// Constructors
	point2_t() { x = 0; y = 0; }

	point2_t(int X, int Y)
	{
		x = X;
		y = Y;
	}

	// Functions
	void set(int X, int Y)
	{
		x = X;
		y = Y;
	}

	void set(point2_t p)
	{
		x = p.x;
		y = p.y;
	}

	bool equals(point2_t p)
	{
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}

	/*
	float magnitude()
	{
		return (float)sqrt((x * x) + (y * y));
	}
	*/

	point2_t perpendicular()
	{
		return point2_t(-y, x);
	}
};

// rect_t: A rectangle
struct rect_t
{
	point2_t tl, br;

	// Constructors
	rect_t() { tl.set(0, 0); br.set(0, 0); }

	rect_t(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	rect_t(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	rect_t(int x, int y, int width, int height, int align)
	{
		if (align == 0) // Top-left
		{
			tl.set(x, y);
			br.set(x + width, y + height);
		}

		if (align == 1) // Centered
		{
			tl.set(x - (width / 2), y - (height / 2));
			br.set(x + (width / 2), y + (height / 2));
		}
	}

	// Functions
	void set(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	void set(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	void set(rect_t rect)
	{
		tl.set(rect.tl);
		br.set(rect.br);
	}

	int x1() { return tl.x; }
	int y1() { return tl.y; }
	int x2() { return br.x; }
	int y2() { return br.y; }

	int left()		{ return min(tl.x, br.x); }
	int top()		{ return min(tl.y, br.y); }
	int right()		{ return max(br.x, tl.x); }
	int bottom()	{ return max(br.y, tl.y); }

	int width() { return br.x - tl.x; }
	int height() { return br.y - tl.y; }

	int awidth() { return max(br.x, tl.x) - min(tl.x, br.x); }
	int aheight() { return max(br.y, tl.y) - min(tl.y, br.y); }

	void resize(int x, int y)
	{
		if (x1() < x2())
		{
			tl.x -= x;
			br.x += x;
		}
		else
		{
			tl.x += x;
			br.x -= x;
		}

		if (y1() < y2())
		{
			tl.y -= y;
			br.y += y;
		}
		else
		{
			tl.y += y;
			br.y -= y;
		}
	}

	double length()
	{
		double dist_x = x2() - x1();
		double dist_y = y2() - y1();

		return sqrt(dist_x * dist_x + dist_y * dist_y);
	}
};

struct patch_info_t
{
	short	xoff;
	short	yoff;
	string	patch;
	short	patch_index;
};

struct texture_t
{
	short					width;
	short					height;
	string					name;
	short					flags;
	BYTE					x_scale;
	BYTE					y_scale;
	vector<patch_info_t>	patches;
};

void image_from_lump(Lump* lump, wxImage* image, wxColour* palette);
unsigned long crc(unsigned char *buf, int len);
bool dsnd_to_wav(char* filename, Lump* lump);
bool load_texturex(Lump* lump, vector<texture_t> &texlist, vector<string> &pnames);
void save_texturex(Lump* lump, vector<texture_t> &texlist);

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#endif
