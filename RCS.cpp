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
RCS_ID($Id: RCS.cpp,v 1.14 2004-06-14 01:46:47 jason Exp $)

#include "version.h"

#include <wx/datetime.h>

static wxString m_latest_rcs_date = wxEmptyString;
static wxString m_latest_rcs_author = wxEmptyString;
static wxDateTime m_latest_compile_date = wxInvalidDateTime;
static bool m_init_done = false;

static wxString extract_date(const wxChar *WXUNUSED_UNLESS_DEBUG(rcs_id_part1), const wxChar *rcs_id_part2)
{
	wxASSERT(rcs_id_part1[0] == wxT('$') && rcs_id_part1[1] == wxT('I') && rcs_id_part1[2] == wxT('d'));
	wxASSERT(rcs_id_part2[0] == wxT('v') && rcs_id_part2[1] == wxT(' '));
	wxString tmp(rcs_id_part2+2);
	size_t i = tmp.Index(wxT(' '));
	wxASSERT(i > 0);
	tmp = tmp.Mid(i + 1, 19);
	return tmp;
}

static wxString extract_author(const wxChar *WXUNUSED_UNLESS_DEBUG(rcs_id_part1), const wxChar *rcs_id_part2)
{
	wxASSERT(rcs_id_part1[0] == wxT('$') && rcs_id_part1[1] == wxT('I') && rcs_id_part1[2] == wxT('d'));
	wxASSERT(rcs_id_part2[0] == wxT('v') && rcs_id_part2[1] == wxT(' '));
	wxASSERT(wxString(rcs_id_part2).Right(5) == wxT("Exp $"));
	wxString tmp(rcs_id_part2, wxString(rcs_id_part2).Length()-6);
	return tmp.AfterLast(wxT(' '));
}

static wxDateTime extract_compile_date(const wxString &file_compile_date, const wxString &file_compile_time)
{

	//                     01234567890
	// file_compile_date = Jan  1 2000
	// file_compile_time = 00:00:00

	wxString year_str = file_compile_date.Mid(7, 4);

	wxString day_str = file_compile_date.Mid(4, 2);
	if (day_str[0u] == wxT(' '))
	{
		day_str[0u] = wxT('0');
	}

	const wxChar *months[] = {
		wxT("Jan"), wxT("Feb"), wxT("Mar"), wxT("Apr"), wxT("May"), wxT("Jun"),
		wxT("Jul"), wxT("Aug"), wxT("Sep"), wxT("Oct"), wxT("Nov"), wxT("Dec") };

	wxString month_str = file_compile_date.Mid(0, 3);
	int month = 0;
	for (int i = 0; i < 12; ++i)
	{
		if (month_str == months[i])
		{
			month = i + 1;
			break;
		}
	}
	month_str = wxString::Format(wxT("%02d"), month);

	// yyyy-mm-dd HH:MM:SS
	wxString str;

	str << year_str << wxT('-') << month_str << wxT('-') << day_str
		<< wxT(' ') << file_compile_time;

	wxDateTime result;
	result.ParseFormat(str, wxT("%Y-%m-%d %H:%M:%S"), wxInvalidDateTime);
	return result;

}

int add_rcs_id(
	const wxChar *rcs_id_part1, const wxChar *rcs_id_part2,
	const wxChar *file_compile_date, const wxChar *file_compile_time)
{

	static wxString s_latest_rcs_date = wxEmptyString;
	static wxString s_latest_rcs_author = wxEmptyString;
	static wxDateTime s_latest_compile_date = wxInvalidDateTime;

	if (!rcs_id_part1 || !rcs_id_part2 || !file_compile_date || !file_compile_time)
	{

		if (!m_init_done)
		{
			m_latest_rcs_date = s_latest_rcs_date;
			m_latest_rcs_author = s_latest_rcs_author;
			m_latest_compile_date = s_latest_compile_date;
			m_init_done = true;
		}
		return 1;

	}
	else
	{

		wxString date = extract_date(rcs_id_part1, rcs_id_part2);
		if (date.Length() == 0 || date > s_latest_rcs_date)
		{
			s_latest_rcs_date = date;
			s_latest_rcs_author = extract_author(rcs_id_part1, rcs_id_part2);
		}
		wxDateTime compile_date = extract_compile_date(file_compile_date, file_compile_time);
		if (compile_date.IsValid() &&
			(!s_latest_compile_date.IsValid() || compile_date > s_latest_compile_date))
		{
			s_latest_compile_date = compile_date;
		}
		return 0;

	}

}

wxString GetRCSDate()
{
	wxCHECK(add_rcs_id(NULL, NULL, NULL, NULL), wxEmptyString);
	return m_latest_rcs_date;
}

wxString GetRCSAuthor()
{
	wxCHECK(add_rcs_id(NULL, NULL, NULL, NULL), wxEmptyString);
	return m_latest_rcs_author;
}

wxString GetProductVersion()
{
	return VERSION;
}

wxDateTime GetCompileDate()
{
	wxCHECK(add_rcs_id(NULL, NULL, NULL, NULL), wxInvalidDateTime);
	return m_latest_compile_date;
}
