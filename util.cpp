#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "util.h"
#include <wx/datetime.h>
#include <wx/html/htmlwin.h>

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

HeadTail SplitHeadTail(const wxString &text)
{
	HeadTail result;
	SplitHeadTail(text, result.head, result.tail);
	return result;
}

wxString Timestamp()
{
	return wxDateTime::Now().Format("[%H:%M] ");
}

void FixBorder(wxWindow *wnd)
{
	#ifdef __WXMSW__
		HWND hWnd = reinterpret_cast<HWND>(wnd->GetHandle());
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		dwStyle &= ~WS_BORDER;
		::SetWindowLong(hWnd, GWL_STYLE, dwStyle);
		::InvalidateRect(hWnd, NULL, FALSE);
		::SetWindowPos(hWnd, NULL, 0,0,0,0,
			SWP_FRAMECHANGED | SWP_NOACTIVATE |
			SWP_NOMOVE | SWP_NOOWNERZORDER |
			SWP_NOSIZE | SWP_NOZORDER);
	#endif
}

void SetHtmlParserFonts(wxHtmlWinParser *parser)
{

	// The following is copied from wxHtmlWinParser::wxHtmlWinParser()
	// as it is not exposed anywhere and NULL isn't allowed
	#ifdef __WXMSW__
			static int default_sizes[7] = {7, 8, 10, 12, 16, 22, 30};
	#elif defined(__WXMAC__)
			static int default_sizes[7] = {9, 12, 14, 18, 24, 30, 36};
	#else
			static int default_sizes[7] = {10, 12, 14, 16, 19, 24, 32};
	#endif

	#ifdef __WXMSW__
		parser->SetFonts(wxEmptyString, "Fixedsys", default_sizes);
	#endif

}
