#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ConfigFile.cpp,v 1.1 2003-04-18 10:05:44 jason Exp $)

#include "ConfigFile.h"
#include "Dirt.h"

DECLARE_APP(DirtApp)

ConfigFile::ConfigFile()
	: wxFileConfig(wxT("dirt"), wxT(""), wxGetApp().GetConfigFilename())
{
	SetUmask(0077);
}

ConfigFile::~ConfigFile()
{
}
