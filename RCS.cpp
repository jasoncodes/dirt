#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"

#include "RCS.h"
RCS_ID($Id: RCS.cpp,v 1.6 2003-02-13 14:09:04 jason Exp $)

static wxString m_latest_rcs_date = wxEmptyString;
static wxString m_latest_rcs_author = wxEmptyString;

#define VERSION "3.0.0 Alpha 0"

static wxString extract_date(const char *x, const char *y)
{
	wxASSERT(x[0] == '$' && x[1] == 'I' && x[2] == 'd');
	wxASSERT(y[0] == 'v' && y[1] == ' ');
	wxString tmp(y+2);
	size_t i = tmp.Index(' ');
	wxASSERT(i > 0);
	tmp = tmp.Mid(i + 1, 19);
	return tmp;
}

static wxString extract_author(const char *x, const char *y)
{
	wxASSERT(x[0] == '$' && x[1] == 'I' && x[2] == 'd');
	wxASSERT(y[0] == 'v' && y[1] == ' ');
	wxASSERT(wxString(y).Right(5) == "Exp $");
	wxString tmp(y, strlen(y)-6);
	return tmp.AfterLast(' ');
}

int add_rcs_id(const char *x, const char *y)
{
	static wxString s_latest_rcs_date = wxEmptyString;
	static wxString s_latest_rcs_author = wxEmptyString;
	if (!x || !y)
	{
		m_latest_rcs_date = s_latest_rcs_date;
		m_latest_rcs_author = s_latest_rcs_author;
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
