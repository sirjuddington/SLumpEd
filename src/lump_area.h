
#ifndef __LUMP_AREA_H__
#define __LUMP_AREA_H__

class WadPanel;

enum
{
	LAREA_NONE,
	LAREA_TEXT,
	LAREA_GFX,
	LAREA_TEXTURES,
	LAREA_MAP,
	LAREA_HEX,
	LAREA_MEDIA,
	LAREA_DATA,
};

class LumpArea : public wxPanel
{
private:
	wxStaticText	*label_lumptype;

protected:
	Lump*		lump;
	WadPanel*	parent;

public:
	int		type;

	LumpArea(WadPanel *parent);
	~LumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();
};

#endif
