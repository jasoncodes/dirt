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
RCS_ID($Id: DirtLogsCGI.cpp,v 1.10 2004-09-10 00:16:59 jason Exp $)

#include <stdio.h>
#include <stdlib.h>

#ifdef __WXMSW__
	#include <io.h>
	#include <fcntl.h>
#endif

#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/dir.h>

#include "util.h"
#include "LogReader.h"
#include "TextTools.h"
#include "URL.h"

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

	wxFileConfig config(GetConfigFilename());

	// todo: check ini file for custom location
	// ConfigFile.cpp has source

	wxFileName cfg(GetConfigFilename());
	wxFileName fn(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR), wxT(""));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

}

static inline wxString GetFriendlyName(const wxString &filename)
{
	return wxFileName(filename).GetName();
}

static int srtNewestFirst(const wxString& file1, const wxString& file2)
{
	wxDateTime mod1, mod2;
	wxFileName(file1).GetTimes(NULL, &mod1, NULL);
	wxFileName(file2).GetTimes(NULL, &mod2, NULL);
	if (mod1 == mod2)
	{
		return -wxStrcmp(GetFriendlyName(file1), GetFriendlyName(file2));
	}
	else
	{
		return (mod1 > mod2) ? -1 : 1;
	}
}

static inline wxString make_link(const wxString &url, const wxString &caption,
                                 bool new_window = false, const wxString &css_class = wxEmptyString)
{
	wxString link;
	link << wxT("<a href=\"") << url  << wxT("\"");
	if (new_window)
	{
		link << wxT(" target=\"_blank\"");
	}
	if (css_class.Length())
	{
		link << wxT(" class=\"") << css_class << wxT("\"");
	}
	link << wxT(">") << caption << wxT("</a>");
	return link;
}

void output_line(const wxString &text, bool convert_urls, int colour)
{

	wxString line = FormatTextAsHtml(text);

	if (convert_urls)
	{
		line = ConvertUrlsToLinks(line);
	}

	line = ConvertModifiersIntoHtml(line, false);

	wxString output;

	output
		<< wxT("<div>");

	if (colour != 0)
	{
		output
			<< wxT("<font color=\"")
			<< ColourRGBToString(colour)
			<< wxT("\">");
	}

	output
		<< line;

	if (colour != 0)
	{
		output
			<< wxT("</font>");
	}

	output
		<< wxT("</div>");

	ConsoleOutputUTF8(output);

}

struct Line
{
	wxString text;
	bool convert_urls;
	int colour;
};

inline void output_line(const Line &line)
{
	output_line(line.text, line.convert_urls, line.colour);
}

#include <wx/list.h>
WX_DECLARE_LIST(Line, LineList);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(LineList);

StringHashMap ParseQueryString(const wxString &str)
{
	StringHashMap map;
	wxArrayString pairs = SplitString(str, wxT("&"));
	for (size_t i = 0; i < pairs.Count(); ++i)
	{
		HeadTail ht = SplitHeadTail(pairs[i], wxT("="));
		map[URL::Unescape(ht.head)] = URL::Unescape(ht.tail);
	}
	return map;
}

long GetQueryLong(const StringHashMap &query, const wxString &name, long default_val)
{
	long x = default_val;
	StringHashMap::const_iterator i = query.find(name);
	if (i != query.end())
	{
		if (!i->second.ToLong(&x))
		{
			x = default_val;
		}
	}
	return x;
}

Line ReadLine(const LogReader &reader)
{
	Line line;
	line.text = reader.GetText();
	line.convert_urls = reader.GetTextConvertURLs();
	line.colour = reader.GetTextColour();
	return line;
}

void send_header(const wxString &baseuri, const wxString &pathinfo)
{
	fprintf(stdout, "Content-Type: text/html; charset=utf-8\r\n");
	fprintf(stdout, "\r\n");
	puts("<html>");
	puts("<body text=\"#000000\" bgcolor=\"#ffffff\">");
	puts("<head>");
	puts("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
	if (pathinfo.Length() > 1)
	{
		wxPuts(wxString() << wxT("<title>") << GetFriendlyName(pathinfo) << wxT("</title>"));
	}
	else
	{
		puts("<title>dirtlogs</title>");
	}
	puts("<style type=\"text/css\">");
	puts("div");
	puts("{");
	puts("\tmargin-left: 32px;");
	puts("\ttext-indent: -32px;");
	puts("\tfont-family: monospace;");
	puts("}");
	puts("a.small_link");
	puts("{");
	puts("\tcolor: black;");
	puts("\tfont-size: smaller;");
	puts("\ttext-decoration: none;");
	puts("}");
	puts("a.small_link:hover");
	puts("{");
	puts("\ttext-decoration: underline;");
	puts("}");
	puts("</style>");
	if (pathinfo.Length() > 1)
	{
		wxPuts(wxString() << wxT("<link rel=\"index\" href=\"") << baseuri << wxT("/\" />"));
		//<link rel="prev" href="Chapter1.html" />
		//<link rel="next" href="Chapter3.html" />
		//<link rel="prefetch" href="Chapter3.html" />
	}
	puts("</head>");
}

int main(int argc, char **argv)
{

#ifdef __WXMSW__
	setmode(fileno(stdout), O_BINARY);
#endif

	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

	wxInitializer initializer;
	if (!initializer)
	{
		fprintf(stderr, "Error initializing wxWidgets.");
		puts("</body></html>");
		return EXIT_FAILURE;
	}

	wxString pathinfo = wxGetenv(wxT("PATH_INFO"));
	wxString scripturi = wxGetenv(wxT("SCRIPT_URI"));
	wxString scripturl = wxGetenv(wxT("SCRIPT_URL"));
	wxString baseuri = scripturi.Left(scripturi.Length() - pathinfo.Length());

	wxString redirect_base_url;
	redirect_base_url =
		scripturi.Left(scripturi.Length()-scripturl.Length()) +
		URL::Escape(scripturl);
	redirect_base_url.Replace(wxT("%7e"), wxT("~"));

	if (scripturi.Length() == 0 || scripturl.Length() == 0)
	{
		wxFputs(wxT("This is a CGI application for viewing Dirt log files.\n"), stderr);
		return EXIT_SUCCESS;
	}

	if (pathinfo.Length() == 0)
	{
		wxFputs(wxString() << wxT("Location: ") << redirect_base_url << wxT("/\r\n\r\n"), stdout);
		return EXIT_SUCCESS;
	}

	StringHashMap query = ParseQueryString(wxGetenv(wxT("QUERY_STRING")));

	const long last = GetQueryLong(query, wxT("last"), -1);
	const long start = GetQueryLong(query, wxT("start"), -1);
	const long count = GetQueryLong(query, wxT("count"), -1);

	bool no_header = (last > -1 && pathinfo.Length() > 1);

	if (!no_header)
	{
		send_header(baseuri, pathinfo);
	}

	wxString logdir = GetLogDirectory();

	if (logdir.Length() == 0)
	{
		if (no_header) send_header(baseuri, pathinfo);
		fprintf(stderr, "Logging is disabled.");
		puts("</body></html>");
		return EXIT_FAILURE;
	}

	if (pathinfo == wxT("/"))
	{

		if (no_header) send_header(baseuri, pathinfo);
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
			wxString friendly_name = GetFriendlyName(files[i]);
			wxPuts(wxString()
				<< wxT("<tt>") << make_link(relpath, friendly_name) << wxT("</tt>")
				<< wxT(" (") << make_link(relpath + wxT("?start=0&count=100"), wxT("first 100"), false, wxT("small_link")) << wxT(")")
				<< wxT(" (") << make_link(relpath + wxT("?last=100"), wxT("last 100"), false, wxT("small_link")) << wxT(")")
				<< wxT("<br />"));
		}

	}
	else
	{

		if (pathinfo.Find(wxT("../")) > -1 || pathinfo.Length() < 10 || pathinfo[0u] != wxT('/') || pathinfo.Right(8) != wxT(".dirtlog"))
		{
			if (no_header) send_header(baseuri, pathinfo);
			wxPuts(wxString() << wxT("File not found: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		wxFileName fn(logdir + pathinfo);

		if (!fn.FileExists())
		{
			if (no_header) send_header(baseuri, pathinfo);
			wxPuts(wxString() << wxT("File not found: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		LogReader reader(fn.GetFullPath());

		if (!reader.Ok())
		{
			if (no_header) send_header(baseuri, pathinfo);
			wxPuts(wxString() << wxT("Error reading file: ") << pathinfo);
			puts("</body></html>");
			return EXIT_FAILURE;
		}

		if (last > -1)
		{
			int total = 0;
			while (reader.HasNext())
			{
				LogEntryType type = reader.GetNext();
				if (type == letText)
				{
					total++;
				}
			}
			long count = wxMin(last, total);
			long start = total - count;
			wxFputs(wxString()
				<< wxT("Location: ") << redirect_base_url
				<< wxT("?start=") << start
				<< wxT("&count=") << count
				<< wxT("\r\n\r\n"), stdout);
			return EXIT_SUCCESS;
		}

		if (no_header) send_header(baseuri, pathinfo);

		int pos = 0;
		while (reader.HasNext() && pos < start)
		{
			LogEntryType type = reader.GetNext();
			if (type == letText)
			{
				pos++;
			}
		}

		if (pos > 0 && count > 0)
		{
			int x = wxMax(pos - count, 0);
			long before = pos-x;
			wxPuts(wxString()
				<< make_link(
					wxString() << redirect_base_url << wxT("?start=") << x << wxT("&count=") << count,
					wxString() << wxT("Previous ") << AddCommas((wxLongLong_t)before))
				<< wxT(" (") << AddCommas((wxLongLong_t)pos) << wxT(" more)"));
		}

		int num = 0;

		while(reader.HasNext() && (count < 0 || num < count))
		{
			LogEntryType type = reader.GetNext();
			if (type == letText)
			{
				pos++;
				num++;
				output_line(ReadLine(reader));
			}
		}

		if (reader.HasNext() && count > 0)
		{
			long left = 0;
			while (reader.HasNext())
			{
				LogEntryType type = reader.GetNext();
				if (type == letText)
				{
					left++;
				}
			}
			long next = wxMin(count, left);
			wxPuts(wxString()
				<< make_link(
					wxString() << redirect_base_url << wxT("?start=") << start+count << wxT("&count=") << count,
					wxString() << wxT("Next ") << AddCommas((wxLongLong_t)next))
				<< wxT(" (") << AddCommas((wxLongLong_t)left) << wxT(" more)"));
		}

	}

	puts("</body></html>");
	return EXIT_SUCCESS;

}
