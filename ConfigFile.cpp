#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ConfigFile.cpp,v 1.2 2003-04-28 12:44:48 jason Exp $)

#include "ConfigFile.h"
#include "Dirt.h"
#include "util.h"
#include <wx/filename.h>

DECLARE_APP(DirtApp)

static wxString GetConfigFilename()
{
	wxString filename = wxGetApp().GetConfigFilename();
	if (filename.Length() == 0)
	{
		wxFileName fn(GetSelf());
		fn.SetFullName(wxT("dirt.ini"));
		if (fn.FileExists())
		{
			return fn.GetFullPath();
		}
	}
	return filename;
}

ConfigFile::ConfigFile()
	: wxFileConfig(wxT("dirt"), wxT(""), GetConfigFilename())
{
	SetUmask(0077);
}

ConfigFile::~ConfigFile()
{
}
