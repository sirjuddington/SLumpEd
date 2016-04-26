////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006/07                                             //
// ------------------------------------------------------------------ //
// text_area.cpp - TextArea class functions                           //
////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "text_area.h"
#include "wad_panel.h"
#include <wx/dir.h>

vector<ScriptLanguage*> languages;

vector<style_t> styles;
style_t style_default =
{
	"default",		// Name
	100, 100, 100,	// Foreground
	255, 255, 255,	// Background
	0				// Flags
};

CVAR(Bool, autosave_text, false, CVAR_SAVE)

void load_styles()
{
	styles.clear();

	Tokenizer tz;
	tz.open_file(_T("res/scripts/styles.cfg"), 0, 0);

	string token = tz.get_token();

	while (token != _T("!END"))
	{
		if (token == _T("style"))
		{
			style_t newstyle = style_default;
			newstyle.name = tz.get_token();

			tz.check_token(_T("{"));
			token = tz.get_token();

			while (token != _T("}"))
			{
				if (token == _T("foreground"))
				{
					tz.check_token(_T("{"));
					newstyle.fred = tz.get_integer();
					newstyle.fgreen = tz.get_integer();
					newstyle.fblue = tz.get_integer();
					tz.check_token(_T("}"));
				}

				if (token == _T("background"))
				{
					tz.check_token(_T("{"));
					newstyle.bred = tz.get_integer();
					newstyle.bgreen = tz.get_integer();
					newstyle.bblue = tz.get_integer();
					tz.check_token(_T("}"));
				}

				if (token == _T("bold"))
					newstyle.flags |= STYLE_BOLD;

				if (token == _T("italic"))
					newstyle.flags |= STYLE_ITALIC;

				if (token == _T("underlined"))
					newstyle.flags |= STYLE_UNDERLINE;

				if (token == _T("lowercase"))
					newstyle.flags |= STYLE_LOWERCASE;

				if (token == _T("uppercase"))
					newstyle.flags |= STYLE_UPPERCASE;

				token = tz.get_token();
			}

			if (newstyle.name == _T("default"))
				memcpy(&style_default, &newstyle, sizeof(style_t));

			styles.push_back(newstyle);
		}

		token = tz.get_token();
	}
}

style_t *get_style(string name)
{
	for (int a = 0; a < styles.size(); a++)
	{
		if (styles[a].name == name)
			return &styles[a];
	}

	return &style_default;
}

void load_languages()
{
	// Get all valid game configuration filenames
	wxDir dir;

	ScriptLanguage *tmp = new ScriptLanguage("None");
	languages.push_back(tmp);

	if (dir.Open(_T("./res/scripts/")))
	{
		wxString filename;
		bool cont = dir.GetFirst(&filename, _T("*.cfg"));

		while (cont)
		{
			string path = _T("res/scripts/");
			path += filename;
			ScriptLanguage *lang = new ScriptLanguage();

			if (lang->loadFile(path))
				languages.push_back(lang);
			else
				delete lang;

			cont = dir.GetNext(&filename);
		}
	}

	load_styles();
}

ScriptLanguage::ScriptLanguage(string name)
{
	this->lang_name = name;
}

ScriptLanguage::~ScriptLanguage()
{
}

bool ScriptLanguage::loadFile(string filename)
{
	Tokenizer tz;
	tz.open_file(filename, 0, 0);

	string token = tz.get_token();

	if (token != _T("scriptlang"))
		return false;

	while (token != "!END")
	{
		// Read main info
		if (token == _T("scriptlang"))
		{
			// Read name
			lang_name = tz.get_token();

			if (!tz.check_token(_T("{")))
				return false;

			token = tz.get_token();
			while (token != _T("}"))
			{
				if (token == _T("lumpname"))
				{
					if (tz.peek_token() != _T("{"))
						lump_names.Add(tz.get_token());
					else
					{
						tz.get_token();
						token = tz.get_token();
						while (token != _T("}"))
						{
							lump_names.Add(token);
							token = tz.get_token();
						}
					}
				}

				if (token == _T("case_sensitive"))
					case_sensitive = tz.get_bool();

				token = tz.get_token();
			}
		}

		// Read keywords
		if (token == _T("keywords"))
		{
			if (!tz.check_token(_T("{")))
				return false;

			kw_cases = 0;

			token = tz.get_token();
			while (token != _T("}"))
			{
				if (token == _T("$case_upper"))
					kw_cases |= CASE_UPPER;
				else if (token == _T("$case_lower"))
					kw_cases |= CASE_LOWER;
				else if (token == _T("$case_caps"))
					kw_cases |= CASE_CAPS;
				else
					keywords.Add(token);

				token = tz.get_token();
			}
		}

		// Read constants
		if (token == _T("constants"))
		{
			if (!tz.check_token(_T("{")))
				return false;

			cs_cases = 0;

			token = tz.get_token();
			while (token != _T("}"))
			{
				if (token == _T("$case_upper"))
					cs_cases |= CASE_UPPER;
				else if (token == _T("$case_lower"))
					cs_cases |= CASE_LOWER;
				else if (token == _T("$case_caps"))
					cs_cases |= CASE_CAPS;
				else
					constants.Add(token);
				token = tz.get_token();
			}
		}

		// Read functions
		if (token == _T("functions"))
		{
			if (!tz.check_token(_T("{")))
				return false;

			token = tz.get_token();
			BYTE cases = 0;
			while (token != _T("}"))
			{
				if (token == _T("$case_upper"))
					cases |= CASE_UPPER;
				else if (token == _T("$case_lower"))
					cases |= CASE_LOWER;
				else if (token == _T("$case_caps"))
					cases |= CASE_CAPS;
				else
				{
					funcdef_t fdef;
					fdef.func_name = token;
					fdef.func_args = token + _T("()");
					fdef.cases = cases;
					if (tz.peek_token() == _T("="))
					{
						tz.get_token(); // skip "="
						fdef.func_args = tz.get_token();
					}
					functions.push_back(fdef);
				}

				token = tz.get_token();
			}
		}

		token = tz.get_token();
	}

	return true;
}

string ScriptLanguage::getLumpName(int index)
{
	if (index < 0 || index >= lump_names.size())
		return _T("");

	return lump_names[index];
}

string ScriptLanguage::getKeywords(bool include_case)
{
	string ret = _T("");

	for (int a = 0; a < keywords.size(); a++)
	{
		if (case_sensitive)
		{
			ret += keywords[a];
			ret += _T(" ");
			
			if (include_case)
			{
				if (kw_cases & CASE_UPPER)
				{
					ret += keywords[a].Upper();
					ret += _T(" ");
				}

				if (kw_cases & CASE_LOWER)
				{
					ret += keywords[a].Lower();
					ret += _T(" ");
				}
			}
		}
		else
		{
			if (include_case)
				ret += keywords[a].Lower();
			else
				ret += keywords[a];

			ret += _T(" ");
		}
	}

	return ret;
}

string ScriptLanguage::getConstants(bool include_case)
{
	string ret = _T("");

	for (int a = 0; a < constants.size(); a++)
	{
		if (case_sensitive)
		{
			ret += constants[a];
			ret += _T(" ");

			if (include_case)
			{
				if (cs_cases & CASE_UPPER)
				{
					ret += constants[a].Upper();
					ret += _T(" ");
				}

				if (cs_cases & CASE_LOWER)
				{
					ret += constants[a].Lower();
					ret += _T(" ");
				}
			}
		}
		else
		{
			if (include_case)
				ret += constants[a].Lower();
			else
				ret += constants[a];

			ret += _T(" ");
		}
	}

	return ret;
}

string ScriptLanguage::getFunctions(bool include_case)
{
	string ret = _T("");

	for (int a = 0; a < functions.size(); a++)
	{
		if (case_sensitive)
		{
			ret += functions[a].func_name;
			ret += _T(" ");

			if (include_case)
			{
				if (functions[a].cases & CASE_LOWER)
				{
					ret += functions[a].func_name.Lower();
					ret += _T(" ");
				}

				if (functions[a].cases & CASE_UPPER)
				{
					ret += functions[a].func_name.Upper();
					ret += _T(" ");
				}
			}
		}
		else
		{
			if (include_case)
				ret += functions[a].func_name.Lower();
			else
				ret += functions[a].func_name;

			ret += _T(" ");
		}
	}

	return ret;
}

bool ScriptLanguage::lumpTitleMatch(string lump)
{
	for (int a = 0; a < lump_names.size(); a++)
	{
		int l = lump_names[a].size();
		if (lump.Upper().Truncate(l) == lump_names[a].Upper())
			return true;
	}

	return false;
}


funcdef_t* ScriptLanguage::searchFunction(string func)
{
	for (int a = 0; a < functions.size(); a++)
	{
		if (functions[a].func_name.Upper() == func.Upper())
			return &functions[a];
	}

	return NULL;
}

TextArea::TextArea(wxWindow *parent, int id)
:	wxScintilla(parent, id)
{
	// Set monospace font
	wxFont tmpFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	StyleSetFont(wxSCI_STYLE_DEFAULT, tmpFont);

	// Set tab stuff
	SetTabWidth(4);
	SetUseTabs(true);
    SetTabIndents(false);
    SetBackSpaceUnIndents(true);
	SetIndent(0);

	StyleClearAll();
	SetLexer(wxSCI_LEX_CPP);

	setupStyle(0, "default");
	setupStyle(1, "comment");
	setupStyle(2, "comment");
	setupStyle(3, "comment");
	setupStyle(4, "number");
	setupStyle(5, "keyword");
	setupStyle(6, "string");
	setupStyle(7, "string");
	setupStyle(8, "default");
	setupStyle(9, "preprocessor");
	setupStyle(10, "operator");
	setupStyle(11, "default");
	setupStyle(12, "default");
	setupStyle(13, "default");
	setupStyle(14, "default");
	setupStyle(15, "comment");
	setupStyle(16, "function");
	setupStyle(17, "keyword");
	setupStyle(18, "default");
	setupStyle(19, "constant");
	setupStyle(32, "default");
	setupStyle(wxSCI_STYLE_CALLTIP, "number");
	setupStyle(wxSCI_STYLE_BRACEBAD, "function");
	setupStyle(wxSCI_STYLE_BRACELIGHT, "keyword");

	wxFont tmpFont2(8, 0, 0, 0);
	StyleSetFont(wxSCI_STYLE_CALLTIP, tmpFont2); 

	// Show line numbers in margin
	SetMarginType(1, 1);
	setMarginWidths();

	cur_lang = NULL;

	AutoCompSetIgnoreCase(true);
	SetMouseDwellTime(1000);

	//StyleSetFont(wxSCI_STYLE_CALLTIP, wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	//StyleSetForeground(wxSCI_STYLE_CALLTIP,	wxColour(0,	0, 0));
}

TextArea::~TextArea()
{
}

void TextArea::setupStyle(int style, string name)
{
	style_t *s = get_style(name);
	StyleSetForeground(style, wxColor(s->fred, s->fgreen, s->fblue));
	StyleSetBackground(style, wxColor(s->bred, s->bgreen, s->bblue));
	StyleSetBold(style, s->flags & STYLE_BOLD);
	StyleSetItalic(style, s->flags & STYLE_ITALIC);
	StyleSetUnderline(style, s->flags & STYLE_UNDERLINE);
	
	if (s->flags & STYLE_UPPERCASE)
		StyleSetCase(style, 1);

	if (s->flags & STYLE_LOWERCASE)
		StyleSetCase(style, 2);
}

void TextArea::setMarginWidths()
{
	SetMarginWidth(1, TextWidth(wxSCI_STYLE_LINENUMBER, wxString::Format(_T("_%d"), GetLineCount())));
}

void TextArea::setLanguage(ScriptLanguage *lang)
{
	cur_lang = lang;

	if (lang)
	{
		if (lang->caseSensitive())
			SetLexer(wxSCI_LEX_CPP);
		else
			SetLexer(wxSCI_LEX_CPPNOCASE);

		SetKeyWords(0, lang->getKeywords());
		SetKeyWords(1, lang->getFunctions());
		SetKeyWords(3, lang->getConstants());
	}
	else
	{
		SetLexer(wxSCI_LEX_NULL);
		SetKeyWords(0, _T(""));
		SetKeyWords(1, _T(""));
		SetKeyWords(3, _T(""));
	}

	Colourise(0, -1);
}

void TextArea::onTextChanged(wxScintillaEvent &event)
{
	char chr = event.GetKey();

	// Auto indent
    int currentLine = GetCurrentLine();
    if (chr == '\n')
	{
        int lineInd = 0;

        if (currentLine > 0)
            lineInd = GetLineIndentation(currentLine - 1);

        if (lineInd != 0)
		{
			SetLineIndentation(currentLine, lineInd);
			GotoPos(GetLineEndPosition(currentLine));
		}
    }

	// Calltips
	if (chr == '(')
	{
		int pos1 = WordStartPosition(GetCurrentPos()-1, false);
		string token = GetTextRange(WordStartPosition(pos1, true), 
									WordEndPosition(pos1, true));

		funcdef_t *f = cur_lang->searchFunction(token);
		if (f)
			CallTipShow(GetCurrentPos(), f->func_args);
	}

	if (chr == ')')
		CallTipCancel();
}

BEGIN_EVENT_TABLE(TextArea, wxScintilla)
	EVT_KEY_DOWN(TextArea::onKeyDown)
END_EVENT_TABLE()

void TextArea::onKeyDown(wxKeyEvent &event)
{
	if (event.ControlDown() && event.GetKeyCode() == WXK_SPACE)
	{
		// Get token previous to cursor
		wxString token = GetTextRange(WordStartPosition(GetCurrentPos(), true), GetCurrentPos());

		if (cur_lang->langName() != _T("None"))
			AutoCompShow(token.size(), cur_lang->getFunctions(false) + cur_lang->getKeywords(false) + cur_lang->getConstants(false));
	}

	event.Skip();
}

void TextArea::onMouseDwell(wxScintillaEvent &event)
{
	int i = event.GetPosition();

	if (i == -1 || CallTipActive())
		return;

	// Get token under cursor
	int p1 = WordStartPosition(i, true);
	int p2 = WordEndPosition(i, true);

	wxString token = GetTextRange(p1, p2);

	funcdef_t *f = cur_lang->searchFunction(token);
	if (f)
		CallTipShow(i, f->func_args);
}

void TextArea::onMouseDwellEnd(wxScintillaEvent &event)
{
	CallTipCancel();
}

TextLumpArea::TextLumpArea(WadPanel *parent)
:	LumpArea(parent)
{
	type = LAREA_TEXT;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	btn_save = new wxButton(this, TLA_BTN_SAVE, _T("Save Changes"));
	hbox->Add(btn_save, 0, 0);

	wxArrayString formats;
	for (int a = 0; a < languages.size(); a++)
		formats.Add(languages[a]->langName());

	combo_format = new wxChoice(this, TLA_COMBO_FORMAT, wxDefaultPosition, wxDefaultSize, formats);
	combo_format->Select(0);

	hbox->Add(new wxStaticText(this, -1, _T("Hilight Format:")), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(combo_format, 0, wxLEFT, 4);

	text = new TextArea(this, TLA_TEXT);
	vbox->Add(text, 1, wxEXPAND);

	Hide();

	change = false;
}

TextLumpArea::~TextLumpArea()
{
}

void TextLumpArea::checkSave()
{
	lump->setEx(_T("TextPos"), wxString::Format(_T("%d"), text->GetCurrentPos()));

	int p = text->PositionFromLine(text->GetFirstVisibleLine() + text->LinesOnScreen());
	lump->setEx(_T("ScrollPos"), wxString::Format(_T("%d"), p-1));

	if (!change)
		return;

	if (!autosave_text)
	{
		if (wxMessageBox(wxString::Format(_T("Save changes to %s?"), lump->getName()), _T("Text Changed"), wxICON_QUESTION|wxYES_NO) == wxNO)
			return;
	}

	lump->loadData((BYTE*)chr(text->GetText()), text->GetText().size());
	change = false;
	lump->setType(LUMP_TEXT);
	parent->updateList(lump);
}

void TextLumpArea::loadLump(Lump* lump)
{
	this->lump = lump;
	wxString istr = wxString::FromAscii((char*)lump->getData());
	istr.Truncate(lump->getSize());
	text->SetText(istr);
	text->setMarginWidths();
	change = false;

	ScriptLanguage *lang = languages[0];
	int index = 0;
	for (int a = 0; a < languages.size(); a++)
	{
		if (languages[a]->lumpTitleMatch(lump->getName()) ||
			languages[a]->lumpTitleMatch(lump->getEx(_T("TextFormat"))))
		{
			lang = languages[a];
			index = a;
		}
	}

	text->setLanguage(lang);
	combo_format->Select(index);

	// A bit of a hack, but setting the scroll position directly doesn't actually scroll the text :/
	string pos = lump->getEx(_T("ScrollPos"));
	if (pos != _T(""))
	{
		int p = atoi(chr(pos));
		text->GotoPos(p);
	}

	pos = lump->getEx(_T("TextPos"));
	if (pos != _T(""))
	{
		int p = atoi((char*)pos.ToAscii());
		text->SetAnchor(p);
		text->SetCurrentPos(p);
	}
}


BEGIN_EVENT_TABLE(TextLumpArea, wxPanel)
	//EVT_TEXT(TLA_TEXT, TextLumpArea::text_changed)
	EVT_CHOICE(TLA_COMBO_FORMAT, TextLumpArea::onFormatChanged)
	EVT_SCI_CHARADDED(TLA_TEXT, TextLumpArea::onTextChanged)
	EVT_SCI_DWELLSTART(TLA_TEXT, TextLumpArea::onMouseDwell)
	EVT_SCI_DWELLEND(TLA_TEXT, TextLumpArea::onMouseDwellEnd)
	EVT_BUTTON(TLA_BTN_SAVE, TextLumpArea::onSaveClicked)
END_EVENT_TABLE()

void TextLumpArea::onTextChanged(wxScintillaEvent &event)
{
	change = true;
	//text->Colourise(0, wxSCI_INVALID_POSITION);
	text->setMarginWidths();
	text->onTextChanged(event);
}

void TextLumpArea::onSaveClicked(wxCommandEvent &event)
{
	//if (!change)
	//	return;

	lump->loadData((BYTE*)chr(text->GetText()), text->GetText().size());
	change = false;
	lump->setType(LUMP_TEXT);
	parent->updateList(lump);
}

void TextLumpArea::onFormatChanged(wxCommandEvent &event)
{
	text->setLanguage(languages[combo_format->GetSelection()]);
	lump->setEx(_T("TextFormat"), languages[combo_format->GetSelection()]->getLumpName(0));
}

void TextLumpArea::onMouseDwell(wxScintillaEvent &event)
{
	text->onMouseDwell(event);
}

void TextLumpArea::onMouseDwellEnd(wxScintillaEvent &event)
{
	text->onMouseDwellEnd(event);
}
