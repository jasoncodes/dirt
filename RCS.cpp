#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"

#include "RCS.h"
RCS_ID($Id: RCS.cpp,v 1.2 2003-02-13 13:00:27 jason Exp $)

int add_rcs_id(const char *x, const char *y)
{
	wxString str;
	str << "\n" << x << "\n" << y << "\n";
	OutputDebugString(str.c_str());
	return 0;
}
