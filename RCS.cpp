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
RCS_ID($Id: RCS.cpp,v 1.13 2004-05-16 04:42:47 jason Exp $)

#include "version.h"

static wxString m_latest_rcs_date = wxEmptyString;
static wxString m_latest_rcs_author = wxEmptyString;
static bool m_init_done = false;

static wxString extract_date(const wxChar *WXUNUSED_UNLESS_DEBUG(x), const wxChar *y)
{
	wxASSERT(x[0] == wxT('$') && x[1] == wxT('I') && x[2] == wxT('d'));
	wxASSERT(y[0] == wxT('v') && y[1] == wxT(' '));
	wxString tmp(y+2);
	size_t i = tmp.Index(wxT(' '));
	wxASSERT(i > 0);
	tmp = tmp.Mid(i + 1, 19);
	return tmp;
}

static wxString extract_author(const wxChar *WXUNUSED_UNLESS_DEBUG(x), const wxChar *y)
{
	wxASSERT(x[0] == wxT('$') && x[1] == wxT('I') && x[2] == wxT('d'));
	wxASSERT(y[0] == wxT('v') && y[1] == wxT(' '));
	wxASSERT(wxString(y).Right(5) == wxT("Exp $"));
	wxString tmp(y, wxString(y).Length()-6);
	return tmp.AfterLast(wxT(' '));
}

int add_rcs_id(const wxChar *x, const wxChar *y)
{
	static wxString s_latest_rcs_date = wxEmptyString;
	static wxString s_latest_rcs_author = wxEmptyString;
	if (!x || !y)
	{
		if (!m_init_done)
		{
			m_latest_rcs_date = s_latest_rcs_date;
			m_latest_rcs_author = s_latest_rcs_author;
			m_init_done = true;
		}
		return 1;
	}
	else
	{
		wxString date = extract_date(x, y);
		if (date.Length() == 0 || date > s_latest_rcs_date)
		{
			s_latest_rcs_date = date;
			s_latest_rcs_author = extract_author(x, y);
		}
		return 0;
	}
}

wxString GetRCSDate()
{
	wxCHECK(add_rcs_id(NULL, NULL), wxEmptyString);
	return m_latest_rcs_date;
}

wxString GetRCSAuthor()
{
	wxCHECK(add_rcs_id(NULL, NULL), wxEmptyString);
	return m_latest_rcs_author;
}

wxString GetProductVersion()
{
	return VERSION;
}
