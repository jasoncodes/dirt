#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "util.h"
#include <wx/datetime.h>

wxArrayString SplitString(const wxString &str, const wxString &sep)
{

	wxArrayString lines;
	
	wxString text = str;

	if (text.Length() > 0)
	{

		int i;
		while ((i = text.Find(sep)) > -1)
		{
			lines.Add(text.Mid(0, i));
			text = text.Mid(i + sep.Length());
		}

		if (text.Length() > 0)
		{
			lines.Add(text);
		}

	}

	return lines;

}

bool LeftEq(const wxString &text, const wxString &to_match)
{
	return text.Left( to_match.Length() ) == to_match;
}

bool RightEq(const wxString &text, const wxString &to_match)
{
	return (text.Right(to_match.Length()) == to_match);
}

void SplitHeadTail(const wxString &text, wxString &head, wxString &tail)
{
	int i = text.Find(" ");
	if (i > -1)
	{
		head = text.Mid(0, i);
		tail = text.Mid(i + 1);
	}
	else
	{
		head = text;
		tail = "";
	}
}

wxString Timestamp()
{
	return wxDateTime::Now().Format("[%H:%M] ");
}