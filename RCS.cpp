#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"

#include "RCS.h"
RCS_ID($Id: RCS.cpp,v 1.8 2003-02-16 05:09:03 jason Exp $)

static wxString m_latest_rcs_date = wxEmptyString;
static wxString m_latest_rcs_author = wxEmptyString;
static bool m_init_done = false;

#define VERSION wxT("3.0.0 Alpha 0")

static wxString extract_date(const wxChar *x, const wxChar *y)
{
	wxASSERT(x[0] == wxT('$') && x[1] == wxT('I') && x[2] == wxT('d'));
	wxASSERT(y[0] == wxT('v') && y[1] == wxT(' '));
	wxString tmp(y+2);
	size_t i = tmp.Index(wxT(' '));
	wxASSERT(i > 0);
	tmp = tmp.Mid(i + 1, 19);
	return tmp;
}

static wxString extract_author(const wxChar *x, const wxChar *y)
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
