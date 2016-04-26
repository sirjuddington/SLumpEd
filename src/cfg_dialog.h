
class ConfigDialog : public wxDialog
{
private:
	wxTextCtrl	*text_iwadpath;
	wxButton	*btn_browseiwad;

	wxTextCtrl	*text_accpath;
	wxButton	*btn_browseacc;

	wxCheckBox	*cb_autosave_gfx;
	wxCheckBox	*cb_autosave_texturex;
	wxCheckBox	*cb_autosave_text;
	wxCheckBox	*cb_determine_type_load;
	wxCheckBox	*cb_save_logfile;
	wxCheckBox	*cb_media_autoplay;
	wxCheckBox	*cb_zip_askopenwad;
	wxCheckBox	*cb_force_uppercase;
	wxCheckBox	*cb_colour_lumps;
	wxCheckBox	*cb_lump_determine_gfx;

public:
	enum
	{
		CD_TEXT_IWADPATH,
		CD_BTN_BROWSEIWAD,
		CD_TEXT_ACCPATH,
		CD_BTN_BROWSEACC,

		CD_CB_START,
	};

	ConfigDialog();
	~ConfigDialog();

	void onCbClicked(wxCommandEvent &event);
	void onIwadPathChanged(wxCommandEvent &event);
	void onAccPathChanged(wxCommandEvent &event);
	void onIwadBrowse(wxCommandEvent &event);
	void onAccBrowse(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
