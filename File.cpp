#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: File.cpp,v 1.1 2003-05-10 04:15:33 jason Exp $)

#include "File.h"

#if !USE_WIN32_FILE
	#include <sys/stat.h>
#endif

wxLongLong_t File::Length(const wxString &filename)
{
	#if USE_WIN32_FILE
	#else
		wxStructStat st;
		if (wxStat(filename, &st) == 0)
		{
			return st.st_size;
		}
		return -1;
	#endif
}

