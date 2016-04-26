////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// misc.cpp - Misc functions                                          //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "misc.h"

void image_from_lump(Lump* lump, wxImage* image, wxColour* palette)
{
	if (lump->getType() == LUMP_FLAT || lump->getType() == LUMP_GFX2)
	{
		if (lump->getSize() == 64*64)
			image->Create(64, 64, true);
		else if (lump->getSize() == 320*200)
			image->Create(320, 200, true);
		else if (lump->getSize() == 64*128)
			image->Create(64, 128, true);
		else
			return;

		for (int a = 0; a < lump->getSize(); a++)
		{
			BYTE* data = image->GetData();
			BYTE col = lump->getData()[a];
			data[a*3] = palette[col].Red();
			data[(a*3)+1] = palette[col].Green();
			data[(a*3)+2] = palette[col].Blue();
		}
	}

	if (lump->getType() == LUMP_PATCH || lump->getType() == LUMP_SPRITE || lump->getType() == LUMP_GFX)
	{
		// Get header & offsets
		patch_header_t *header = (patch_header_t *)lump->getData();
		long *col_offsets= (long *)((BYTE *)lump->getData() + sizeof(patch_header_t));

		image->Create(header->width, header->height, true);
		image->SetAlpha(NULL);
		memset(image->GetAlpha(), 0, header->width * header->height);

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
					image->SetRGB(c, row+p, palette[*data].Red(), palette[*data].Green(), palette[*data].Blue());
					image->SetAlpha(c, row+p, 255);
				}
				data += 2; // Skip buffer & go to next row offset
			}
		}
	}

	if (lump->getType() == LUMP_PNG)
	{
		lump->dumpToFile(_T("slumptemp"));
		image->LoadFile(_T("slumptemp"), wxBITMAP_TYPE_PNG);
		remove("slumptemp");
	}
}

// CRC stuff for PNG

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++)
	{
		c = (unsigned long) n;

		for (k = 0; k < 8; k++)
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}

		crc_table[n] = c;
	}

	crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
{
	unsigned long c = crc;
	int n;

	if (!crc_table_computed)
		make_crc_table();

	for (n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}




struct dsnd_hdr_t
{
	WORD three;
	WORD samplerate;
	WORD samples;
	WORD zero;
};

struct wav_hdr_t
{
	char id[4];
	long size;
};

struct wav_fmtchunk_t
{
	wav_hdr_t header;
	short tag;
	short channels;
	long samplerate;
	long datarate;
	short blocksize;
	short bps;
};


bool dsnd_to_wav(char* filename, Lump* lump)
{
	dsnd_hdr_t *dhdr = (dsnd_hdr_t*)lump->getData();
	wav_hdr_t whdr, wdhdr;
	wav_fmtchunk_t fmtchunk;

	if (dhdr->three != 3)
		return false;

	char did[4] = { 'd', 'a', 't', 'a' };
	memcpy(&wdhdr.id, &did, 4);
	wdhdr.size = lump->getSize() - 8;

	char fid[4] = { 'f', 'm', 't', ' ' };
	memcpy(&fmtchunk.header.id, &fid, 4);
	fmtchunk.header.size = 16;
	fmtchunk.tag = 1;
	fmtchunk.samplerate = dhdr->samplerate;
	fmtchunk.datarate = dhdr->samplerate;
	fmtchunk.channels = 1;
	fmtchunk.blocksize = 1;
	fmtchunk.bps = 8;

	char wid[4] = { 'R', 'I', 'F', 'F' };
	memcpy(&whdr.id, &wid, 4);
	whdr.size = wdhdr.size + fmtchunk.header.size + 8;

	FILE *fp = fopen(filename, "wb");

	if (!fp)
		return false;

	fwrite(&whdr, 8, 1, fp);
	fwrite("WAVE", 4, 1, fp);
	fwrite(&fmtchunk, sizeof(wav_fmtchunk_t), 1, fp);
	fwrite(&wdhdr, 8, 1, fp);
	fwrite(lump->getData() + 8, lump->getSize() - 8, 1, fp);

	if ((lump->getSize() - 8) % 2 != 0)
	{
		BYTE temp = 0;
		fwrite(&temp, 1, 1, fp);
	}

	fclose(fp);

	return true;
}

bool load_texturex(Lump* lump, vector<texture_t> &texlist, vector<string> &pnames)
{
	if (!lump)
		return false;

	if (lump->getSize() == 0)
		return false;

	long n_tex = 0;
	long *offsets = NULL;
	long temp = 0;

	short flags = 0;
	BYTE  x_scale = 0;
	BYTE  y_scale = 0;
	short width = 0;
	short height = 0;
	short patches = 0;

	short xoff = 0;
	short yoff = 0;
	short patch = 0;

	BYTE* p = lump->getData();

	// Read number of textures
	memcpy(&n_tex, p, 4);
	p += 4;

	// Read offsets
	offsets = new long[n_tex];
	memcpy(offsets, p, n_tex * 4);

	for (int t = 0; t < n_tex; t++)
	{
		p = lump->getData();
		p += offsets[t];

		// Read texture name
		char texname[9] = "";
		memset(texname, 0, 9);
		memcpy(texname, p, 8);
		p += 8;

		// Read flags
		memcpy(&flags, p, 2);
		p += 2;

		// Read scale info
		memcpy(&x_scale, p, 1);
		p++;
		memcpy(&y_scale, p, 1);
		p++;

		// Read width & height
		memcpy(&width, p, 2);
		p += 2;
		memcpy(&height, p, 2);
		p += 2;

		// Add texture
		texture_t tex;
		tex.name = wxString::FromAscii(texname);
		tex.width = width;
		tex.height = height;
		tex.flags = flags;
		tex.x_scale = x_scale;
		tex.y_scale = y_scale;

		// Skip unused stuff
		p += 4;

		// Read no. of patches in texture
		memcpy(&patches, p, 2);
		p += 2;

		// Add patches
		for (int a = 0; a < patches; a++)
		{
			// Read patch info
			memcpy(&xoff, p, 2);
			p += 2;
			memcpy(&yoff, p, 2);
			p += 2;
			memcpy(&patch, p, 2);
			p += 2;

			// Skip unused
			p += 4;

			patch_info_t pi;
			if (pnames.size() > 0) pi.patch = pnames[patch];
			pi.patch_index = patch;
			pi.xoff = xoff;
			pi.yoff = yoff;
			tex.patches.push_back(pi);
		}

		texlist.push_back(tex);
	}

	delete[] offsets;

	return true;
}

void save_texturex(Lump* lump, vector<texture_t> &texlist)
{
	FILE *fp = fopen("slumptemp", "wb");

	long ntex = texlist.size();
	fwrite(&ntex, 4, 1, fp);

	// Skip offsets for now
	long *offsets = new long[ntex];
	fseek(fp, ntex * 4, SEEK_CUR);

	for (int a = 0; a < texlist.size(); a++)
	{
		offsets[a] = ftell(fp);

		// Write name
		const char* tname = texlist[a].name.Truncate(8).ToAscii();
		fwrite(tname, 1, 8, fp);

		// Write flags
		fwrite(&texlist[a].flags, 2, 1, fp);

		// Write scale info
		fwrite(&texlist[a].x_scale, 1, 1, fp);
		fwrite(&texlist[a].y_scale, 1, 1, fp);

		// Write dimensions
		fwrite(&texlist[a].width, 2, 1, fp);
		fwrite(&texlist[a].height, 2, 1, fp);

		// Write more unused
		long poo = 0;
		fwrite(&poo, 4, 1, fp);

		// Write no. of patches
		short np = texlist[a].patches.size();
		fwrite(&np, 2, 1, fp);

		// Write patch info
		for (int p = 0; p < np; p++)
		{
			fwrite(&texlist[a].patches[p].xoff, 2, 1, fp);
			fwrite(&texlist[a].patches[p].yoff, 2, 1, fp);
			fwrite(&texlist[a].patches[p].patch_index, 2, 1, fp);
			fwrite(&poo, 4, 1, fp);
		}
	}

	// Now write offsets
	fseek(fp, 4, SEEK_SET);
	fwrite(offsets, 4, ntex, fp);

	fclose(fp);

	lump->loadFile("slumptemp");
	remove("slumptemp");
}

struct doomside_t
{
	short	x_offset;
	short	y_offset;
	char	tex_upper[8];
	char	tex_lower[8];
	char	tex_middle[8];
	short	sector;
};

struct doomsector_t
{
	short	f_height;
	short	c_height;
	char	f_tex[8];
	char	c_tex[8];
	short	light;
	short	special;
	short	tag;
};

void get_used_textures(Wad* wad, vector<string> &textures, vector<string> &flats)
{
	for (DWORD a = 0; a < wad->numLumps(); a++)
	{
		Lump* lump = wad->lumpAt(a);

		// SIDEDEFS lump?
		if (lump->getType() == LUMP_SIDEDEFS)
		{
			doomside_t *sides = (doomside_t*)(lump->getData());
			int n_sides = lump->getSize() / 30;

			for (int s = 0; s < n_sides; s++)
			{
				char temp[9] = "";
				temp[8] = 0;

				memcpy(temp, sides[s].tex_lower, 8);
				string lo = wxString::FromAscii(temp).Upper();
				if (find(textures.begin(), textures.end(), lo) == textures.end())
					textures.push_back(lo);

				memcpy(temp, sides[s].tex_middle, 8);
				string mid = wxString::FromAscii(temp).Upper();
				if (find(textures.begin(), textures.end(), mid) == textures.end())
					textures.push_back(mid);

				memcpy(temp, sides[s].tex_upper, 8);
				string up = wxString::FromAscii(temp).Upper();
				if (find(textures.begin(), textures.end(), up) == textures.end())
					textures.push_back(up);
			}
		}
	}
}


void remove_unused_textures(Wad* wad)
{
	vector<string> textures;
	vector<string> flats;

	get_used_textures(wad, textures, flats);
}
