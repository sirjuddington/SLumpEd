
#include "misc.h"
#include "image.h"

class GfxOffsetDialog : public wxDialog
{
private:
	wxChoice	*combo_aligntype;

	wxTextCtrl	*entry_xoff;
	wxTextCtrl	*entry_yoff;
	wxCheckBox	*cbox_relative;

	wxRadioButton	*opt_set;
	wxRadioButton	*opt_auto;

public:
	enum
	{
		GOD_OPT_SET,
		GOD_OPT_AUTO,
	};

	GfxOffsetDialog(wxWindow *parent);
	~GfxOffsetDialog();

	point2_t	getOffset();
	int			getAlignType();
	int			getOption();
	bool		relativeOffset();
	bool		xOffChange();
	bool		yOffChange();

	void		onOptSet(wxCommandEvent &event);
	void		onOptAuto(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

class GfxConvertDialog : public wxDialog
{
private:
	wxCheckBox	*cbox_trans;
	wxCheckBox	*cbox_colours;

	wxRadioButton	*opt_trans_existing;
	wxRadioButton	*opt_trans_col;
	wxTextCtrl		*entry_trans_col;

	wxRadioButton	*opt_32bpp;
	wxRadioButton	*opt_256col;
	wxRadioButton	*opt_pal_current;

public:
	enum
	{
		GCD_CBOX_TRANS,
		GCD_CBOX_COLOURS,
	};

	GfxConvertDialog(wxWindow *parent, int type, Image &img);
	~GfxConvertDialog();

	int		getTrans();
	int		getTransCol();
	int		getColDepth();

	void	onCboxTrans(wxCommandEvent &event);
	void	onCboxColours(wxCommandEvent &event);
	
	DECLARE_EVENT_TABLE()
};
