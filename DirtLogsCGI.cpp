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
RCS_ID($Id: DirtLogsCGI.cpp,v 1.3 2004-07-20 18:13:38 jason Exp $)

#include <stdio.h>
#include <stdlib.h>

#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/dir.h>

#include "util.h"
#include "LogReader.h"
#include "TextTools.h"

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

static int srtNewestFirst(const wxString& file1, const wxString& file2)
{
	wxDateTime mod1, mod2;
	wxFileName(file1).GetTimes(NULL, &mod1, NULL);
	wxFileName(file2).GetTimes(NULL, &mod2, NULL);
	if (mod1 == mod2)
	{
		return 0;
	}
	else
	{
		return (mod1 > mod2) ? -1 : 1;
	}
}

int main(int argc, char **argv)
{

	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

	wxInitializer initializer;
	if ( !initializer )
	{
		fprintf(stderr, "Error initializing wxWidgets.");
		puts("</body></html>");
		return EXIT_FAILURE;
	}

	wxString pathinfo = wxGetenv(wxT("PATH_INFO"));

	if (pathinfo.Length() == 0)
	{
		wxFprintf(stdout, wxString() << wxT("Location: ") << wxGetenv(wxT("SCRIPT_URI")) << wxT("/\r\n\r\n"));
		return EXIT_SUCCESS;
	}

	fprintf(stdout, "Content-Type: text/html; charset=utf-8\r\n");
	fprintf(stdout, "\r\n");
	puts("<html>");
	puts("<body>");

	wxString logdir = GetLogDirectory();

	if (logdir.Length() == 0)
	{
		fprintf(stderr, "Logging is disabled.");
		puts("</body></html>");
		return EXIT_FAILURE;
	}

	if (pathinfo == wxT("/"))
	{

		wxPuts(wxString() << wxT("Log directory: ") << logdir << wxT("<br />"));

		wxArrayString files;
		wxDir::GetAllFiles(logdir, &files, wxT("*.dirtlog"));
		files.Sort(srtNewestFirst);

		if (files.GetCount() == 0)
		{
			fprintf(stderr, "No log files found.");
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		for (size_t i = 0u; i < files.GetCount(); ++i)
		{
			wxString relpath = files[i].Mid(logdir.Length());
			wxString friendly_name = wxFileName(files[i]).GetName();
			wxPuts(wxString() << wxT("<tt><a href=\"") << relpath << wxT("\">") << friendly_name << wxT("</a></tt><br />"));
		}

	}
	else
	{

		if (pathinfo.Find(wxT("../")) > -1 || pathinfo[0u] != wxT('/'))
		{
			wxPuts(wxString() << wxT("File not found: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		wxFileName fn(logdir + pathinfo);

		if (!fn.FileExists())
		{
			wxPuts(wxString() << wxT("File not found: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		LogReader reader(fn.GetFullPath());

		if (!reader.Ok())
		{
			wxPuts(wxString() << wxT("Error reading file: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		while(reader.HasNext())
		{

			LogEntryType type = reader.GetNext();

			if (type == letText)
			{

				wxString line = reader.GetText();

				line = FormatTextAsHtml(line);

				if (reader.GetTextConvertURLs())
				{
					line = ConvertUrlsToLinks(line);
				}

				line = ConvertModifiersIntoHtml(line, false);

				line = wxString()
					<< wxT("<tt>")
					<< wxT("<font color=\"")
					<< ColourRGBToString(reader.GetTextColour())
					<< wxT("\">")
					<< line
					<< wxT("</font>")
					<< wxT("</tt><br />");

				ConsoleOutputUTF8(line);

			}

		}

	}

	puts("</body></html>");
	return EXIT_SUCCESS;

}
