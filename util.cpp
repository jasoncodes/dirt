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
#include <math.h>
#include "ByteBuffer.h"

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

void SplitHeadTail(const wxString &text, wxString &head, wxString &tail, const wxString &sep)
{
	int i = text.Find(sep);
	if (i > -1)
	{
		head = text.Mid(0, i);
		tail = text.Mid(i + sep.Length());
	}
	else
	{
		head = text;
		tail = "";
	}
}

HeadTail SplitHeadTail(const wxString &text, const wxString &sep)
{
	HeadTail result;
	SplitHeadTail(text, result.head, result.tail, sep);
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

wxString AddCommas(off_t size)
{
	wxASSERT(size >= 0);
	wxString buff;
	while (size >= 1000)
	{
		buff = wxString::Format("%03d", size % 1000) + buff;
		buff = ',' + buff;
		size /= 1000;
	}
	return wxString() << (int)size << buff;
}

wxString AddCommas(double size)
{

	off_t size2 = (off_t)size;
	
	wxString remainder = wxString::Format("%04.2lf", fmod(size, 1));
	wxASSERT(remainder.Left(2) == "0." || remainder.Left(2) == "1.");
	if (remainder.Left(2) == "1.")
	{
		size2++;
	}
	remainder = remainder.Mid(1);

	wxString quotient = AddCommas((off_t)size2);

	return wxString() << quotient << remainder;

}

wxString SizeToString(off_t size)
{
	wxASSERT(size >= 0);
	if (size < 1000)
	{
		return wxString() << (int)size << " bytes";
	}
	else if (size < 524288)
	{
		return wxString() << AddCommas(size / pow(1024, 1)) << " KB";
	}
	else if (size < 1073741824)
	{
		return wxString() << AddCommas(size / pow(1024, 2)) << " MB";
	}
	else if (size < 1099511627776)
	{
		return wxString() << AddCommas(size / pow(1024, 3)) << " GB";
	}
	else
	{
		return wxString() << AddCommas(size / pow(1024, 4)) << " TB";
	}
}

wxString SizeToLongString(off_t size, wxString suffix)
{
	wxString result;
	result << SizeToString(size) << suffix;
	if (size >= 1000)
	{	
		result << " (" << AddCommas(size) << " bytes" << suffix << ")";
	}
	return result;
}

wxString SecondsToMMSS(long seconds)
{

	bool neg = (seconds < 0);

	if (neg)
	{
		seconds = -seconds;
	}

	int hour, min, sec;

	sec = seconds % 60;
	min = seconds / 60;

	if (min > 59)
	{
		hour = min / 60;
		min = min % 60;
	}
	else
	{
		hour = 0;
	}
	
	wxString result;

	if (hour)
	{
		result = wxString::Format("%02d:%02d:%02d", hour, min, sec);
	}
	else
	{
		result = wxString::Format("%02d:%02d", min, sec);
	}

	if (neg)
	{
		result = "-" + result;
	}

	return result;

}

ByteBuffer Uint32ToBytes(wxUint32 num)
{
	ByteBuffer bytes(4);
	byte *ptr = bytes.Lock();
	ptr[0] = ((num >> 24) & 0xff);
	ptr[1] = ((num >> 16) & 0xff);
	ptr[2] = ((num >> 8)  & 0xff);
	ptr[3] = ((num >> 0)  & 0xff);
	bytes.Unlock();
	return bytes;
}

wxUint32 BytesToUint32(const byte *data, int len)
{
	wxASSERT((len == 4) && (len == sizeof (wxUint32)));
	return
		(data[0] << 24) +
		(data[1] << 16) +
		(data[2] << 8) +
		(data[3] << 0);
}

ByteBuffer Uint16ToBytes(wxUint16 num)
{
	ByteBuffer bytes(2);
	byte *ptr = bytes.Lock();
	ptr[0] = ((num >> 8)  & 0xff);
	ptr[1] = ((num >> 0)  & 0xff);
	bytes.Unlock();
	return bytes;
}

wxUint16 BytesToUint16(const byte *data, int len)
{
	wxASSERT((len == 2) && (len == sizeof (wxUint16)));
	return
		(data[0] << 8) +
		(data[1] << 0);
}
