/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: DirtLogsCGI.cpp,v 1.2 2004-07-19 18:27:51 jason Exp $)

#include <stdio.h>
#include <stdlib.h>

#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/dir.h>

#include "util.h"

#ifdef wxPuts
#undef wxPuts
#endif
#define wxPuts ConsoleOutput

wxString GetConfigFilename()
{
	wxFileName fn(GetSelf());
	fn.SetFullName(wxT("dirt.ini"));
	if (fn.FileExists())
	{
		return fn.GetFullPath();
	}
	return wxFileConfig::GetLocalFileName(wxT("dirt"));
}

wxString GetLogDirectory()
{

	wxFileConfig cfg(GetConfigFilename());

	// todo: check ini file for custom location
	// ConfigFile.cpp has source

	wxFileName fn(GetConfigFilename());
	fn.SetName(wxT("dirtlogs/"));
	return fn.GetFullPath();

}

int main(int argc, char **argv)
{

	puts("Content-Type: text/html; charset=utf-8");
	puts("");

	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

	wxInitializer initializer;
	if ( !initializer )
	{
		fprintf(stderr, "Error initializing wxWidgets.");
		return EXIT_FAILURE;
	}

	wxString logdir = GetLogDirectory();

	if (logdir.Length() == 0)
	{
		fprintf(stderr, "Logging is disabled.");
		return EXIT_FAILURE;
	}

	wxPuts(wxString() << wxT("Log directory: ") << logdir);

	wxArrayString files;
	wxDir::GetAllFiles(logdir, &files, wxT("*.dirtlog"));

	if (files.GetCount() == 0)
	{
		fprintf(stderr, "No log files found.");
		return EXIT_FAILURE;
	}

	wxPuts(wxString() << files.GetCount());

	return EXIT_SUCCESS;
	
}
