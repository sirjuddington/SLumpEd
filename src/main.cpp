////////////////////////////////////////////////////////////////////////
// SLump - A doom wad/lump manager                                    //
// By Simon Judd, 2006                                                //
// ------------------------------------------------------------------ //
// main.cpp - Main app stuff                                          //
////////////////////////////////////////////////////////////////////////

// INCLUDES ////////////////////////////////////////////////////////////
#include "main.h"
#include "main_window.h"
#include "text_area.h"
#include "fmod.h"
#include "progbar.h"

#include <wx/image.h>
#include <wx/sysopt.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

// VARIABLES ///////////////////////////////////////////////////////////
MainWindow	*main_window;
Wad			iwad;
Wad			clipboard;
FILE		*logfile;

string app_path;
string tmp_path;
string usr_path;

CVAR(Bool, save_logfile, false, CVAR_SAVE)

// EXTERNAL VARIABLES //////////////////////////////////////////////////
EXTERN_CVAR(String, iwad_path)
EXTERN_CVAR(Bool, determine_type_load)

IMPLEMENT_APP(MainApp)

/*
// str: Converts a wxString to a std::string, converting from unicode to ASCII if necessary
// ----------------------------------------------------------------------------------------
std::string str(wxString in)
{
	wxCharBuffer buffer = in.ToAscii();
	std::string ret = (const char*)buffer;
	return ret;
}
*/

// chr: Converts a wxString to an ascii character string
// -------------------------------------------------- >>
const char* chr(string &str)
{
#ifdef UNICODE
	wxCharBuffer buffer = str.ToAscii();
	const char* data = buffer;
	return data;
#else
	return (const char*)str.ToAscii();
#endif
}

// c_path: Adds a path to a filename
// ------------------------------ >>
string c_path(string filename, BYTE dir)
{
	if (dir == DIR_APP)
		return app_path+filename;
	else if (dir == DIR_TMP)
		return tmp_path+filename;
	else if (dir == DIR_USR)
		return usr_path+filename;
	else 
		return _T("");
}

// save_config: Saves the main configuration file
// ----------------------------------------------
void save_config()
{
	FILE *fp = fopen(chr(c_path("slumped.cfg", DIR_USR)), "wt");
	save_cvars(fp);
	save_recent_wads(fp);
	fclose(fp);
}

// load_config: Loads the main configuration file
// ----------------------------------------------
void load_config()
{
	Tokenizer tz;
	if (!tz.open_file(c_path("slumped.cfg", DIR_USR), 0, 0))
		return;

	string token = tz.get_token();

	while (token != _T("!END"))
	{
		if (token == _T("cvars"))
			load_cvars(&tz);

		if (token == _T("recent_wads"))
			load_recent_wads(&tz);

		token = tz.get_token();
	}
}

void setup_directories()
{
	// Temporary directory
#ifdef unix
	tmp_path = _T("/tmp/");
#elif defined(__APPLE__)
	tmp_path = string(getTempDir()) + _T("/");
#else
	tmp_path = app_path;
#endif

	// User directory
#ifdef unix
	app_path = wxString::FromAscii(SHARE_DIR);
	usr_path = string(wxFileName::GetHomeDir()) + string(_T("/.slumped/"));
#elif defined(__APPLE__)
	app_path = string(getBundleResourceDir()) + _T("/");
	usr_path = wxFileName::GetHomeDir() + string(_T("/Library/Application Support/SLumpEd/"));
#elif WIN32
	//usr_path = wxFileName::GetHomeDir() + string(_T("/Application Data/SLumpEd/"));
	usr_path = wxString::FromAscii(getenv("APPDATA")) + string(_T("/SLumpEd/"));
#else
	usr_path = app_path;
#endif
}

void create_dir(string dir)
{
#ifdef unix
	if (mkdir(chr(dir), S_IRUSR|S_IWUSR|S_IXUSR) == -1)
		wxLogMessage(_T("Could not create ") + dir);
#else
	if (mkdir(chr(dir)) == -1)
		wxLogMessage(_T("Could not create ") + dir);
#endif
}

// MainApp::OnInit: Called when the application begins
// ---------------------------------------------------
bool MainApp::OnInit()
{
#ifdef WIN32
	app_path = argv[0];
	for (int a = 0; a < 11; a++)
		app_path.erase(app_path.end() - 1);
#endif

	setup_directories();

	logfile = fopen(chr(c_path("slumped.log", DIR_APP)), "wt");
	wxLog::SetActiveTarget(new wxLogStderr(logfile));

	wxSystemOptions::SetOption(_T("msw.remap"), 0);
	wxImage::AddHandler(new wxPNGHandler);

	//wxStandardPaths sp;
	//wxString dataDir = sp.GetDataDir();
	//wxSetWorkingDirectory(dataDir);

	// Create User Directory if it doesn't already exist
#ifdef unix
	if(!wxFileName::DirExists(usr_path))
	{
		if(mkdir(chr(usr_path), S_IRUSR | S_IWUSR | S_IXUSR) == -1)
			wxLogMessage(_T("Could not create ") + usr_path);
	}
#elif WIN32
	if (!wxFileName::DirExists(c_path(_T(""), DIR_USR)))
		create_dir(c_path(_T(""), DIR_USR));
#endif

	load_config();
	load_languages();

	main_window = new MainWindow();
	SetTopWindow(main_window);

	if (iwad_path == "")
	{
		if (wxMessageBox(_T("To view graphics properly, you must set an IWAD to get the palette from\n(it can be changed later from the file menu)"), _T("SLumpEd"), wxOK|wxCANCEL) == wxOK)
		{
			string filename = wxFileSelector(_T("Choose file(s) to open"), _T(""), _T(""), _T(""),
											_T("Doom Wad files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST);

			if (filename != _T(""))
				iwad_path = filename;
		}
	}

	bool dtl = determine_type_load;
	determine_type_load = false;
	iwad.open(iwad_path);
	determine_type_load = dtl;

	if (argc > 0)
	{
		for (int a = 0; a < argc; a++)
			wxLogMessage(argv[a]);

		for (int a = 1; a < argc; a++)
		{
			wxString arg = argv[a];

			if (arg.Right(4).CmpNoCase(_T(".wad")) ||
				arg.Right(4).CmpNoCase(_T(".zip")) ||
				arg.Right(4).CmpNoCase(_T(".pk3")))
			{
				wxLogMessage(_T("Opening wad %s"), arg);
				main_window->openWad(arg);
			}
		}
	}

	FSOUND_Init(48000, 128, 0);
	setup_progbar();
	progbar_hide();

	return true;
}

// MainApp::OnExit: Called when the application exits
// --------------------------------------------------
int MainApp::OnExit()
{
	save_config();

	// Close and delete logfile if necessary
	fclose(logfile);
	if (!save_logfile)
		remove(chr(c_path(_T("slumped.log"), DIR_APP)));

	return 0;
}
