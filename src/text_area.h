
#include "lump_area.h"
#include <wx/wxscintilla.h>

#define CASE_UPPER	0x01	// UPPER CASE
#define CASE_LOWER	0x02	// lower case
#define CASE_CAPS	0x04	// Capitalised Case

struct funcdef_t
{
	string	func_name;
	string	func_args;
	BYTE	cases;
};

class ScriptLanguage
{
private:
	string	lang_name;
	//string	lump_name;
	bool	case_sensitive;

	wxArrayString		lump_names;
	wxArrayString		keywords;
	wxArrayString		constants;
	vector<funcdef_t>	functions;
	BYTE				kw_cases;
	BYTE				cs_cases;

public:
	ScriptLanguage(string name = _T(""));
	~ScriptLanguage();

	//string	lumpName() { return lump_name; }
	string	langName() { return lang_name; }
	bool	caseSensitive() { return case_sensitive; }

	bool loadFile(string filename);
	string getKeywords(bool include_case = true);
	string getConstants(bool include_case = true);
	string getFunctions(bool include_case = true);
	string getLumpName(int index);

	bool lumpTitleMatch(string lump);
	funcdef_t* searchFunction(string func);
};

class TextArea : public wxScintilla
{
private:
	ScriptLanguage *cur_lang;

public:
	void setMarginWidths();
	void setLanguage(ScriptLanguage *lang);
	void setupStyle(int style, string name);

	TextArea(wxWindow *parent, int id);
	~TextArea();

	void onTextChanged(wxScintillaEvent &event);
	void onKeyDown(wxKeyEvent &event);
	void onMouseDwell(wxScintillaEvent &event);
	void onMouseDwellEnd(wxScintillaEvent &event);

	DECLARE_EVENT_TABLE()
};

class TextLumpArea : public LumpArea
{
private:
	//wxTextCtrl	*text;
	TextArea	*text;
	wxButton	*btn_save;
	wxChoice	*combo_format;
	bool		change;

public:
	enum
	{
		TLA_TEXT,
		TLA_BTN_SAVE,
		TLA_COMBO_FORMAT,
	};

	TextLumpArea(WadPanel *parent);
	~TextLumpArea();

	virtual void loadLump(Lump* lump);
	virtual void checkSave();

	// Events
	void onTextChanged(wxScintillaEvent &event);
	void onSaveClicked(wxCommandEvent &event);
	void onFormatChanged(wxCommandEvent &event);
	void onMouseDwell(wxScintillaEvent &event);
	void onMouseDwellEnd(wxScintillaEvent &event);

	DECLARE_EVENT_TABLE()
};

struct style_t
{
	string name;
	BYTE fred, fgreen, fblue;
	BYTE bred, bgreen, bblue;
	BYTE flags;
};

#define STYLE_BOLD		0x01
#define STYLE_ITALIC	0x02
#define STYLE_UNDERLINE	0x04
#define STYLE_UPPERCASE	0x08
#define STYLE_LOWERCASE	0x10

void load_languages();
