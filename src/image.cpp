////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// image.cpp - Image class functions                                  //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "image.h"

#include <wx/filename.h>

CVAR(Bool, image_pal_cyantrans, true, CVAR_SAVE)

Image::Image(wxColour *pal)
{
	offsets.set(0, 0);
	bitmap = NULL;
	palette = pal;
}

Image::~Image()
{
	if (bitmap)
		FreeImage_Unload(bitmap);
}

int Image::getWidth()
{
	if (!bitmap)
		return 0;

	return FreeImage_GetWidth(bitmap);
}

int Image::getHeight()
{
	if (!bitmap)
		return 0;

	return FreeImage_GetHeight(bitmap);
}

BYTE* Image::getRGBData()
{
	if (!bitmap)
		return NULL;

	FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bitmap);

	BYTE* ret = new BYTE[getWidth()*getHeight()*3];
	FreeImage_ConvertToRawBits(ret, rgb, 3*getWidth(), 24, 0x000000FF, 0x0000FF00, 0x00FF0000, false);

	// Swap red and blue channels, since FreeImage seems to store data as BGR
	int size = getWidth()*getHeight();
	int a = 0;
	while (a < size)
	{
		BYTE temp = ret[a*3];
		ret[a*3] = ret[a*3+2];
		ret[a*3+2] = temp;
		a++;
	}

	FreeImage_Unload(rgb);

	return ret;
}

BYTE* Image::getAlpha()
{
	if (!bitmap)
		return NULL;

	FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bitmap);
	FIBITMAP *alpha = FreeImage_GetChannel(rgb, FICC_ALPHA);

	BYTE* ret = new BYTE[getWidth()*getHeight()];
	FreeImage_ConvertToRawBits(ret, alpha, getWidth(), 8, 0xFF000000, 0x00FF0000, 0x0000FF00, false);

	FreeImage_Unload(rgb);
	FreeImage_Unload(alpha);

	return ret;
}


bool Image::loadFile(string filename)
{
	int format = FIF_UNKNOWN;
	string ext = wxFileName(filename).GetExt();

	if (ext.CmpNoCase(_T("png")))
		format = FIF_PNG;
	else if (ext.CmpNoCase(_T("bmp")))
		format = FIF_BMP;

	FIBITMAP *bm = FreeImage_Load((FREE_IMAGE_FORMAT)format, filename, 0);

	if (bm)
	{
		if (bitmap)
			FreeImage_Unload(bitmap);

		bitmap = bm;

		return true;
	}
	else
		return false;
}

bool Image::loadLump(Lump *lump)
{
	if (!lump)
		return false;

	// Doom flat format
	if (lump->getType() == LUMP_FLAT || lump->getType() == LUMP_GFX2)
	{
		int w = 0;
		int h = 0;

		if (lump->getSize() == 64*64)
			w = h = 64;
		else if (lump->getSize() == 320*200)
		{
			w = 320;
			h = 200;
		}
		else if (lump->getSize() == 64*128)
		{
			w = 64;
			h = 128;
		}
		else
			return false;

		FIBITMAP *bm = FreeImage_Allocate(w, h, 8);

		// Load palette
		RGBQUAD *pal = FreeImage_GetPalette(bm);
		for (int a = 0; a < 256; a++)
		{
			if (palette)
			{
				pal[a].rgbRed = palette[a].Red();
				pal[a].rgbGreen = palette[a].Green();
				pal[a].rgbBlue = palette[a].Blue();
			}
			else
			{
				pal[a].rgbRed = a;
				pal[a].rgbGreen = a;
				pal[a].rgbBlue = a;
			}
		}

		// Load data
		BYTE* bits = (BYTE*)FreeImage_GetBits(bm);
		int p = 0;
		for (int y = h-1; y >= 0; y--)
		{
			for (int x = 0; x < w; x++)
			{
				BYTE col = lump->getData()[p++];
				bits[(y*w)+x] = col;
			}
		}

		setOffsets(0, 0);

		if (bitmap)
			FreeImage_Unload(bitmap);

		bitmap = bm;

		return true;
	}

	// Doom gfx format
	if (lump->getType() == LUMP_PATCH || lump->getType() == LUMP_SPRITE || lump->getType() == LUMP_GFX)
	{
		// Get header & offsets
		patch_header_t *header = (patch_header_t *)lump->getData();
		long *col_offsets= (long *)((BYTE *)lump->getData() + sizeof(patch_header_t));

		FIBITMAP *bm = FreeImage_Allocate(header->width, header->height, 8);
		
		// Load palette
		RGBQUAD *pal = FreeImage_GetPalette(bm);
		for (int a = 0; a < 256; a++)
		{
			if (palette)
			{
				pal[a].rgbRed = palette[a].Red();
				pal[a].rgbGreen = palette[a].Green();
				pal[a].rgbBlue = palette[a].Blue();
			}
			else
			{
				pal[a].rgbRed = a;
				pal[a].rgbGreen = a;
				pal[a].rgbBlue = a;
			}
		}

		if (image_pal_cyantrans)
		{
			pal[247].rgbRed = 0;
			pal[247].rgbGreen = 255;
			pal[247].rgbBlue = 255;
		}

		// Set transparency
		BYTE trans[256];
		memset(&trans, 255, 256);
		trans[247] = 0;
		FreeImage_SetTransparent(bm, true);
		FreeImage_SetTransparencyTable(bm, trans, 256);

		// Load data
		BYTE *bits = (BYTE*)FreeImage_GetBits(bm);
		int pitch = FreeImage_GetPitch(bm);
		memset(bits, 247, pitch*header->height);
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
					bits[((header->height - (row+p) - 1)*pitch) + c] = *data;
				}
				data += 2; // Skip buffer & go to next row offset
			}
		}

		setOffsets(header->left, header->top);

		if (bitmap)
			FreeImage_Unload(bitmap);

		bitmap = bm;

		return true;
	}

	// 'Normal' Gfx formats (PNG, BMP, JPEG, etc)
	if (lump->getType() == LUMP_PNG || lump->getType() == LUMP_IMAGE)
	{
		FIMEMORY *mem = FreeImage_OpenMemory(lump->getData(), lump->getSize());

		FREE_IMAGE_FORMAT fmt = FreeImage_GetFileTypeFromMemory(mem, lump->getSize());
		FIBITMAP *bm = FreeImage_LoadFromMemory(fmt, mem, 0);

		if (bm)
		{
			if (bitmap)
				FreeImage_Unload(bitmap);

			bitmap = bm;

			if (fmt == FIF_PNG)
			{
				// Find offsets if present
				BYTE* data = lump->getData();
				long xoff = 0;
				long yoff = 0;
				for (int a = 0; a < lump->getSize(); a++)
				{
					if (data[a] == 'g' &&
						data[a+1] == 'r' &&
						data[a+2] == 'A' &&
						data[a+3] == 'b')
					{
						memcpy(&xoff, data+a+4, 4);
						memcpy(&yoff, data+a+8, 4);
						xoff = wxINT32_SWAP_ON_LE(xoff);
						yoff = wxINT32_SWAP_ON_LE(yoff);
						break;
					}

					if (data[a] == 'I' &&
						data[a+1] == 'D' &&
						data[a+2] == 'A' &&
						data[a+3] == 'T')
						break;
				}

				offsets.set(xoff, yoff);
			}

			return true;
		}
		else
			return false;
	}

	return false;
}


wxImage* Image::toWxImage()
{
	return new wxImage(getWidth(), getHeight(), getRGBData(), getAlpha(), false);
}

void Image::savePNG(string filename, WORD flags)
{
	if (!bitmap)
		return;

	FreeImage_Save(FIF_PNG, bitmap, filename, 0);

	// If saving offsets, load the file back into memory and re-write with grAb chunk
	if (flags & ISV_OFFSETS)
	{
		FILE* fp = fopen(chr(filename), "rb");
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		BYTE* data = new BYTE[size];
		fseek(fp, 0, SEEK_SET);
		fread(data, size, 1, fp);
		fclose(fp);

		fp = fopen(chr(filename), "wb");

		// Write PNG header and IHDR chunk
		fwrite(data, 33, 1, fp);

		struct grab_chunk_t
		{
			char name[4];
			long xoff;
			long yoff;
		};

		DWORD csize = wxUINT32_SWAP_ON_LE(8);
		grab_chunk_t gc = { 'g', 'r', 'A', 'b', wxINT32_SWAP_ON_LE(offsets.x), wxINT32_SWAP_ON_LE(offsets.y) };
		DWORD dcrc = wxUINT32_SWAP_ON_LE(crc((BYTE*)&gc, 12));

		// Write grAb chunk
		fwrite(&csize, 4, 1, fp);
		fwrite(&gc, 12, 1, fp);
		fwrite(&dcrc, 4, 1, fp);

		// Write the rest of the file
		fwrite(data + 33, size - 33, 1, fp);

		fclose(fp);

		delete[] data;
	}
}

void Image::savePNGToLump(Lump* lump, WORD flags)
{
	if (!bitmap)
		return;

	// Save image to memory
	FIMEMORY *tmem = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_PNG, bitmap, tmem, 0);

	// Copy content to buffer and close memory
	int size = FreeImage_TellMemory(tmem);
	BYTE* data = new BYTE[size];
	FreeImage_SeekMemory(tmem, 0, SEEK_SET);
	FreeImage_ReadMemory(data, size, 1, tmem);
	FreeImage_CloseMemory(tmem);

	if (flags & ISV_OFFSETS)
	{
		tmem = FreeImage_OpenMemory();

		// Write PNG header and IHDR chunk
		FreeImage_WriteMemory(data, 33, 1, tmem);

		struct grab_chunk_t
		{
			char name[4];
			long xoff;
			long yoff;
		};

		DWORD csize = wxUINT32_SWAP_ON_LE(8);
		grab_chunk_t gc = { 'g', 'r', 'A', 'b', wxINT32_SWAP_ON_LE(offsets.x), wxINT32_SWAP_ON_LE(offsets.y) };
		DWORD dcrc = wxUINT32_SWAP_ON_LE(crc((BYTE*)&gc, 12));

		// Write grAb chunk
		FreeImage_WriteMemory(&csize, 4, 1, tmem);
		FreeImage_WriteMemory(&gc, 12, 1, tmem);
		FreeImage_WriteMemory(&dcrc, 4, 1, tmem);

		// Write the rest of the image
		FreeImage_WriteMemory(data + 33, size - 33, 1, tmem);

		// Write new image data back to buffer
		size = FreeImage_TellMemory(tmem);
		delete[] data;
		data = new BYTE[size];
		FreeImage_SeekMemory(tmem, 0, SEEK_SET);
		FreeImage_ReadMemory(data, size, 1, tmem);
		FreeImage_CloseMemory(tmem);
	}

	// Load buffer to lump
	lump->loadData(data, size);
	delete[] data;
}

void Image::saveBMP(string filename, WORD flags)
{
	if (!bitmap)
		return;

	FreeImage_Save(FIF_BMP, bitmap, filename, 0);
}

void Image::saveBMPToLump(Lump* lump, WORD flags)
{
	if (!bitmap)
		return;

	FIMEMORY *tmem = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_BMP, bitmap, tmem, 0);

	int size = FreeImage_TellMemory(tmem);
	BYTE* data = new BYTE[size];
	FreeImage_SeekMemory(tmem, 0, SEEK_SET);
	FreeImage_ReadMemory(data, size, 1, tmem);
	lump->loadData(data, size);

	delete[] data;
	FreeImage_CloseMemory(tmem);
}

FIBITMAP* Image::convertToPalette(wxColour* p, int psize)
{
	if (!bitmap)
		return NULL;

	if (!p)
	{
		p = palette;
		psize = 256;
	}

	RGBQUAD pal[256];
	for (int a = 0; a < psize; a++)
	{
		if (palette)
		{
			pal[a].rgbRed = p[a].Red();
			pal[a].rgbGreen = p[a].Green();
			pal[a].rgbBlue = p[a].Blue();
		}
		else
		{
			pal[a].rgbRed = a;
			pal[a].rgbGreen = a;
			pal[a].rgbBlue = a;
		}
	}

	FIBITMAP *bm24 = FreeImage_ConvertTo24Bits(bitmap);
	FIBITMAP *ret = FreeImage_ColorQuantizeEx(bm24, FIQ_NNQUANT, 256, psize, pal);
	FreeImage_Unload(bm24);

	return ret;
}

struct post_t
{
	BYTE			row_off;
	vector<BYTE>	pixels;
};

struct column_t
{
	vector<post_t> posts;
};

bool Image::saveDoomGfx(string filename, WORD flags, int trans_index)
{
	// Get the current palette minus index 247, since we don't want the quantize function
	// picking that colour for any pixels
	RGBQUAD pal[255];
	int p = 0;
	for (int a = 0; a < 256; a++)
	{
		if (a == 247)
			continue;

		if (palette)
		{
			pal[p].rgbRed = palette[a].Red();
			pal[p].rgbGreen = palette[a].Green();
			pal[p].rgbBlue = palette[a].Blue();
		}
		else
		{
			pal[p].rgbRed = a;
			pal[p].rgbGreen = a;
			pal[p].rgbBlue = a;
		}
		p++;
	}

	// Convert the image to the current palette
	FIBITMAP *bm24 = FreeImage_ConvertTo24Bits(bitmap);
	FIBITMAP *bm8bpp = FreeImage_ColorQuantizeEx(bm24, FIQ_NNQUANT, 255, 255, pal);
	FreeImage_Unload(bm24);

	// Get transparency info (if present) as an alpha channel
	bool notrans = flags & ISV_NOTRANS;

	if (notrans)
		trans_index = -1;

	FIBITMAP* alpha = NULL;
	if (FreeImage_GetBPP(bitmap) == 32 && !notrans)
		alpha = FreeImage_GetChannel(bitmap, FICC_ALPHA);

	if (!alpha && FreeImage_GetTransparencyCount(bitmap) > 0 && !notrans)
	{
		FIBITMAP *temp = FreeImage_ConvertTo32Bits(bitmap);
		alpha = FreeImage_GetChannel(temp, FICC_ALPHA);
		FreeImage_Unload(temp);
	}

	if (!bm8bpp)
		return false;

	// Flip image and alpha channel
	FreeImage_FlipVertical(bm8bpp);

	if (alpha)
		FreeImage_FlipVertical(alpha);

	// Get width & height (max height is 255 for doom format gfx)
	int width = getWidth();
	int height = getHeight();

	if (height > 255)
		height = 255;

	// Convert image to column/post structure
	vector<column_t> columns;
	for (int c = 0; c < width; c++)
	{
		column_t col;
		post_t post;
		bool ispost = false;

		for (int r = 0; r < height; r++)
		{
			BYTE index;
			FreeImage_GetPixelIndex(bm8bpp, c, r, &index);

			if (index >= 247)
				index++;

			bool trans = false;

			if (alpha)
			{
				BYTE a = 0;
				FreeImage_GetPixelIndex(alpha, c, r, &a);
				if (a < 128) trans = true;
			}
			else if (trans_index != -1 && FreeImage_GetPalette(bitmap))
			{
				BYTE i = 0;
				FreeImage_GetPixelIndex(bitmap, c, height - r - 1, &i);
				trans = (i == trans_index);
			}

			if (!trans)
			{
				if (!ispost)
				{
					post.row_off = r;
					ispost = true;
				}

				post.pixels.push_back(index);
			}
			else
			{
				if (ispost)
				{
					col.posts.push_back(post);
					post.pixels.clear();
					ispost = false;
				}
			}
		}

		if (post.pixels.size() > 0)
			col.posts.push_back(post);

		columns.push_back(col);
	}

	FreeImage_Unload(bm8bpp);

	FILE* fp = fopen(chr(filename), "wb");

	// Setup header
	patch_header_t header;
	header.top = offsets.y;
	header.left = offsets.x;
	header.width = getWidth();
	header.height = getHeight();

	// Write it
	fwrite(&header.width, 2, 1, fp);
	fwrite(&header.height, 2, 1, fp);
	fwrite(&header.left, 2, 1, fp);
	fwrite(&header.top, 2, 1, fp);

	// Skip column offsets for now
	fseek(fp, 8 + (columns.size() * 4), SEEK_SET);

	long *col_offsets = new long[columns.size()];

	// Write posts
	for (int c = 0; c < columns.size(); c++)
	{
		// Record column offset
		col_offsets[c] = ftell(fp);

		for (int p = 0; p < columns[c].posts.size(); p++)
		{
			// Write row offset
			fwrite(&columns[c].posts[p].row_off, 1, 1, fp);

			// Write no. of pixels
			BYTE npix = columns[c].posts[p].pixels.size();
			fwrite(&npix, 1, 1, fp);

			// Write unused byte
			BYTE temp = 0;
			fwrite(&temp, 1, 1, fp);

			// Write pixels
			for (int a = 0; a < columns[c].posts[p].pixels.size(); a++)
				fwrite(&columns[c].posts[p].pixels[a], 1, 1, fp);

			// Write unused byte
			fwrite(&temp, 1, 1, fp);
		}

		// Write '255' row to signal end of column
		BYTE temp = 255;
		fwrite(&temp, 1, 1, fp);
	}

	// Now we write column offsets
	fseek(fp, 8, SEEK_SET);
	fwrite(col_offsets, columns.size()*4, 1, fp);

	// Close the file
	fclose(fp);

	return true;
}

bool Image::saveDoomFlat(string filename, WORD flags)
{
	if (!(getWidth() == 64 && getHeight() == 64 ||
			getWidth() == 64 && getHeight() == 128 ||
			getWidth() == 128 && getHeight() == 128 ||
			getWidth() == 320 && getHeight() == 200))
		return false;

	FIBITMAP *bm8bpp = convertToPalette();
	FreeImage_FlipVertical(bm8bpp);

	FILE *fp = fopen(chr(filename), "wb");

	for (int y = 0; y < getHeight(); y++)
	{
		for (int x = 0; x < getWidth(); x++)
		{
			BYTE c = 0;
			FreeImage_GetPixelIndex(bm8bpp, x, y, &c);
			fwrite(&c, 1, 1, fp);
		}
	}

	fclose(fp);
}

void Image::colourToTransparent(wxColour col)
{
}
