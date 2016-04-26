
enum
{
	LUMP_UNKNOWN = 0,
	LUMP_MARKER,
	LUMP_TEXT,

	LUMP_PATCH,
	LUMP_SPRITE,
	LUMP_FLAT,
	LUMP_GFX,
	LUMP_GFX2,
	LUMP_PNG,
	LUMP_IMAGE,

	LUMP_SOUND,
	LUMP_WAV,
	LUMP_MP3,
	LUMP_FLAC,
	LUMP_MUS,
	LUMP_MIDI,
	LUMP_MOD,

	LUMP_TEXTURES,
	LUMP_PNAMES,

	LUMP_MAP,
	LUMP_LINEDEFS,
	LUMP_SIDEDEFS,
	LUMP_VERTEXES,
	LUMP_SECTORS,
	LUMP_THINGS,
	LUMP_NODES,
	LUMP_SEGS,
	LUMP_SSECTS,
	LUMP_REJECT,
	LUMP_BLOCKMAP,
	LUMP_BEHAVIOR,

	LUMP_PLAYPAL,
	LUMP_COLORMAP,

	LUMP_WAD,

	LUMP_FOLDER,


	LTYPE_COUNT,
};

class Wad;

struct key_value_t
{
	string key;
	string value;
};
typedef vector<key_value_t> exinfo_t;

class Lump
{
private:
	long		offset; // In a zip this is the index of the lump in the zip
	long		size;
	string		name;
	BYTE		*data;
	int			type;
	BYTE		changed; // 0 = unchanged, 1 = changed, 2 = newly created
	bool		loaded;
	Wad*		parent;
	exinfo_t	ex_info;
	bool		gfx_checked;

	vector<string>	directory;

public:
	Lump(long offset = 0, long size = 0, string name = _T(""), Wad* parent = NULL);
	~Lump();

	long	getOffset() { return offset;		}
	long	getSize()	{ return size;			}
	string	getName(bool full = true, bool ext = true);
	int		getType()	{ return type;			}
	bool	isChanged() { return (changed>0);	}
	bool	isNew()		{ return (changed==2);	}
	bool	isLoaded()	{ return loaded;		}
	Wad*	getParent() { return parent;		}
	BYTE*	getData();

	string	getDir(int index);
	string	getFullDir();
	int		dirLevel()	{ return (int)directory.size(); }
	void	addDir(string name, int index = -1);
	void	renameDir(string nname, int index);

	void	setOffset(DWORD o);
	void	reSize(DWORD s);
	void	setName(string n);
	void	setChanged(int c, bool offset = false);
	void	setParent(Wad* p)	{ parent = p;	}
	void	setType(int t)		{ type = t; gfx_checked = false;	}

	void	dumpToFile(string filename);
	bool	loadFile(string filename);
	void	loadStream(FILE *fp);
	void	loadData(BYTE* buf, DWORD size);
	void	determineType(bool force = false);
	void	checkGfxFormat();
	string	getTypeString();

	string	getEx(string key);
	void	setEx(string key, string val);
};

class Wad
{
private:
	vector<Lump*>	directory;
	vector<string>	dirs;

public:
	Lump	*parent;

	// Wad path
	string		path;

	// Header
	char	type[4];
	//DWORD	num_lumps;
	DWORD	dir_offset;

	// Stuff
	int		patches[2];
	int		flats[2];
	int		sprites[2];
	int		tx[2];

	vector<string>	available_maps;

	// Flags
	bool	locked;	// True if wad cannot be written to (for IWADs)
	bool	changed;
	bool	zip;

	// Constructor/Destructor
	Wad()
	{
		dir_offset = 0;
		locked = false;
		changed = false;
		type[0] = 'P';
		type[1] = 'W';
		type[2] = 'A';
		type[3] = 'D';
		path = _("");
		parent = NULL;

		patches[0] = patches[1] = -1;
		flats[0] = flats[1] = -1;
		sprites[0] = sprites[1] = -1;
		tx[0] = tx[1] = -1;
	}

	~Wad()
	{
		for (DWORD l = 0; l < directory.size(); l++)
			delete directory[l];

		directory.clear();
	}

	DWORD	numLumps() { return (DWORD)directory.size(); }

	// Member Functions
	bool	open(string filename, bool load_data = false);
	bool	openZip(string filename, bool load_data = false);
	void	save(string newname = _T(""));
	void	saveZip(string newname = _T(""));
	void	close();

	long	getLumpIndex(string name, DWORD offset = 0);
	long	lumpIndex(Lump* lump);
	Lump*	getLump(string name, DWORD offset = 0);
	Lump*	lumpAt(int index);
	Lump*	lastLump();

	void	addDir(string path);
	void	renameDir(Lump* dirlump, string newname);
	Lump*	addLump(string name, long index);
	void	replaceLump(string name, DWORD new_size, BYTE *data, DWORD offset);
	void	deleteLump(string name, DWORD offset = 0);
	void	deleteLump(long index, bool delfolder = true);
	void	swapLumps(long index1, long index2);
	void	swapLumps(Lump* lump1, Lump* lump2);

	void	loadLump(Lump* lump);

	void	deleteAllLumps();

	void	dumpDirectory();
	bool	needsSave();
	void	findMaps();
};

#define WAD_IWAD	0x0001
#define WAD_ZIP		0x0002
#define WAD_CHANGED	0x0004
#define WAD_LOCKED	0x0008

#define START	0
#define	END		1

struct patch_header_t
{
	short	width;
	short	height;
	short	left;
	short	top;
};

struct col_header_t
{
	BYTE	row_offset;
	BYTE	n_pixels;
};
