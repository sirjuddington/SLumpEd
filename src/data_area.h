
#include "lump_area.h"
#include <wx/listctrl.h>

class DataLumpArea : public LumpArea
{
private:
	wxListCtrl	*list_data;
	bool		allow_delete;
	bool		*columns_editable;
	Wad			*wadfile;

public:
	enum
	{
		DLA_LIST_DATA = 1,

		DLA_CM_DELETE,
		DLA_CM_END,
	};

	DataLumpArea(WadPanel *parent, Wad* wadfile);
	~DataLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	void deleteItem();

	void onDataListRightClick(wxListEvent &event);
	void onContextMenuSelect(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
