#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"

#include "RCS.h"
RCS_ID($Id: RCS.cpp,v 1.5 2003-02-13 13:19:17 jason Exp $)

static wxString latest_ver;

static wxString extract_ver(const char *x, const char *y)
{
	wxASSERT(x[0] == '$' && x[1] == 'I' && x[2] == 'd');
	wxASSERT(y[0] == 'v' && y[1] == ' ');
	wxString tmp(y+2);
	size_t i = tmp.Index(' ');
	wxASSERT(i > 0);
	tmp = tmp.Mid(i + 1, 19);
	return tmp;
}

int add_rcs_id(const char *x, const char *y)
{
	wxString str;
	str << "\n" << extract_ver(x, y) << "\n";
	OutputDebugString(str.c_str());
	return 0;
}
