
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "FreeImage.h"
#include "misc.h"

class Image
{
private:
	FIBITMAP	*bitmap;
	point2_t	offsets;
	wxColour	*palette;

public:
	Image(wxColour *pal = NULL);
	~Image();

	int		getWidth();
	int		getHeight();
	BYTE*	getRGBData();
	BYTE*	getAlpha();

	int		xOff() { return offsets.x; }
	int		yOff() { return offsets.y; }

	void	setPalette(wxColour *pal) { palette = pal; }
	void	setXOff(int x) { offsets.x = x; }
	void	setYOff(int y) { offsets.y = y; }
	void	setOffsets(int x, int y) { offsets.set(x, y); }

	bool	loadFile(string filename);
	bool	loadLump(Lump* lump);

	void	savePNG(string filename, WORD flags = 0);
	void	savePNGToLump(Lump* lump, WORD flags = 0);

	void	saveBMP(string filename, WORD flags = 0);
	void	saveBMPToLump(Lump* lump, WORD flags = 0);

	bool	saveDoomGfx(string filename, WORD flags = 0, int trans_index = 247);

	bool	saveDoomFlat(string filename, WORD flags = 0);

	void	colourToTransparent(wxColour col);

	FIBITMAP*	convertToPalette(wxColour* palette = NULL, int psize = 256);

	wxImage*	toWxImage();
};

#define ISV_OFFSETS		0x0001
#define ISV_PALETTE		0x0002
#define ISV_CYANTRANS	0x0004
#define ISV_NOTRANS		0x0008

#endif //__IMAGE_H__
