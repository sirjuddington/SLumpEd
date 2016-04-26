////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// data_area.cpp - DataArea class functions                           //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "data_area.h"
#include "misc.h"
#include "wad_panel.h"

DataLumpArea::DataLumpArea(WadPanel *parent, Wad* wadfile)
:	LumpArea(parent)
{
	type = LAREA_DATA;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	list_data = new wxListCtrl(this, DLA_LIST_DATA, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES);
	vbox->Add(list_data, 1, wxEXPAND|wxALL, 4);

	allow_delete = false;
	columns_editable = NULL;
	this->wadfile = wadfile;
}

DataLumpArea::~DataLumpArea()
{
}

void DataLumpArea::loadLump(Lump *lump)
{
	this->lump = lump;

	// PNAMES
	if (lump->getType() == LUMP_PNAMES)
	{
		list_data->ClearAll();
		list_data->InsertColumn(0, _T("Index"));
		list_data->InsertColumn(1, _T("Patch Name"));

		BYTE* data = lump->getData();
		DWORD n_pnames = *((DWORD*)data);

		DWORD a = 4;
		for (DWORD p = 0; p < n_pnames; p++)
		{
			char name[9] = "";
			memset(name, 0, 9);
			memcpy(name, (data + a), 8);
			a += 8;

			wxListItem li;

			// Index
			li.SetId(p);
			li.SetText(wxString::Format(_T("%d"), p));
			li.SetColumn(0);
			list_data->InsertItem(li);

			// Name
			li.SetText(wxString::FromAscii(name));
			li.SetColumn(1);
			list_data->SetItem(li);
		}

		columns_editable = new bool[2];
		columns_editable[0] = false;
		columns_editable[1] = true;
		allow_delete = true;
	}
}

void DataLumpArea::checkSave()
{
	parent->updateList(lump);
}

void DataLumpArea::deleteItem()
{
	// Get selection
	vector<int> selection;
	for (int a = 0; a < list_data->GetItemCount(); a++)
	{
		if (list_data->GetItemState(a, wxLIST_STATE_SELECTED))
			selection.push_back(a);
	}

	if (lump->getType() == LUMP_PNAMES)
	{
		vector<texture_t> texlist;
		vector<string> pnames;

		// Read existing pnames
		BYTE* data = lump->getData();
		DWORD n_pnames = *((DWORD*)data);

		DWORD a = 4;
		for (DWORD p = 0; p < n_pnames; p++)
		{
			char name[9] = "";
			memset(name, 0, 9);
			memcpy(name, (data + a), 8);
			a += 8;
			pnames.push_back(wxString::FromAscii(name));
		}

		// Get new pname indices (-1 for deleted
		vector<int> new_pname_indices;
		int p = 0;
		for (int a = 0; a < pnames.size(); a++)
		{
			if (vector_exists(selection, a))
				new_pname_indices.push_back(-1);
			else
				new_pname_indices.push_back(p++);
		}

		// Update TEXTURE1 if it exists
		Lump *texturex = wadfile->getLump(_T("TEXTURE1"));
		if (texturex)
		{
			load_texturex(texturex, texlist, pnames);

			for (int t = 0; t < texlist.size(); t++)
			{
				for (int p = 0; p < texlist[t].patches.size(); p++)
				{
					int i = new_pname_indices[texlist[t].patches[p].patch_index];

					if (i == -1)
					{
						texlist[t].patches.erase(texlist[t].patches.begin() + p);
						p--;
					}
					else
						texlist[t].patches[p].patch_index = i;
				}

				if (texlist[t].patches.size() == 0)
				{
					texlist.erase(texlist.begin() + t);
					t--;
				}
			}

			save_texturex(texturex, texlist);
		}

		// Update list
		for (int a = 0; a < list_data->GetItemCount(); a++)
		{
			if (list_data->GetItemState(a, wxLIST_STATE_SELECTED))
			{
				list_data->DeleteItem(a);
				pnames.erase(pnames.begin() + a);
				a--;
			}
		}

		// Update PNAMES lump
		FILE *fp = fopen("slumptemp", "wb");
		DWORD np = pnames.size();
		fwrite(&np, 4, 1, fp);

		for (int a = 0; a < pnames.size(); a++)
		{
			const char* name = pnames[a].Truncate(8).ToAscii();
			fwrite(name, 1, 8, fp);
		}

		fclose(fp);
		lump->loadFile(_T("slumptemp"));
		remove("slumptemp");
	}
}

BEGIN_EVENT_TABLE(DataLumpArea, wxPanel)
	EVT_LIST_ITEM_RIGHT_CLICK(DLA_LIST_DATA, DataLumpArea::onDataListRightClick)

	// Context menu
	EVT_MENU_RANGE(DLA_CM_DELETE, DLA_CM_END - 1, DataLumpArea::onContextMenuSelect)
	//EVT_MENU(DLA_CM_DELETE, DataLumpArea::onContextMenuSelect)
END_EVENT_TABLE()

void DataLumpArea::onDataListRightClick(wxListEvent &event)
{
	wxMenu *popup = new wxMenu(_T(""));

	if (allow_delete)
		popup->Append(DLA_CM_DELETE, _T("&Delete Item(s)"));

	PopupMenu(popup, event.GetPoint().x, event.GetPoint().y);
}

void DataLumpArea::onContextMenuSelect(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case DLA_CM_DELETE:
		deleteItem();
		break;
	};
}
