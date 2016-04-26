
#include "lump_area.h"

class HexLumpArea : public LumpArea
{
private:
	wxTextCtrl	*text;

public:
	enum
	{
		HLA_TEXT,
	};

	HexLumpArea(WadPanel *parent);
	~HexLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	// Events

	//DECLARE_EVENT_TABLE()
};
