
#include <stdio.h>
#include <stdlib.h>

//#include <string>
//using std::string;

#include <vector>
using std::vector;

#include <algorithm>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

// For compilers that don't support precompilation, include "wx/wx.h"
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

typedef wxString string;

#define BYTE	unsigned char
#define WORD	unsigned short
#define DWORD	unsigned long

// A macro to check if a value exists in a vector
#define vector_exists(vec, val) find(vec.begin(), vec.end(), val) != vec.end()

// A macro to add a value to a vector if the value doesn't already exist in the vector
#define vector_add_nodup(vec, val) if (!(vector_exists(vec, val))) vec.push_back(val)

#include "wad.h"
#include "tokenizer.h"
#include "cvar.h"

class MainApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
};

//std::string str(wxString in);
const char* chr(string &str);
string c_path(string filename, BYTE dir);

#define DIR_APP	0
#define DIR_TMP	1
#define DIR_USR	2
