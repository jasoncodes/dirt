#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"

#include "RCS.h"
RCS_ID($Id: RCS.cpp,v 1.1 2003-02-13 12:51:35 jason Exp $)

int add_rcs_id(const char *id)
{
	wxString str;
	str << "\n" << id << "\n";
	OutputDebugString(str.c_str());
	return 0;
}
