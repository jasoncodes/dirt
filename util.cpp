#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: util.cpp,v 1.28 2003-02-21 10:23:25 jason Exp $)

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
		tail = wxEmptyString;
	}
}

HeadTail SplitHeadTail(const wxString &text, const wxString &sep)
{
	HeadTail result;
	SplitHeadTail(text, result.head, result.tail, sep);
	return result;
}

void SplitQuotedHeadTail(const wxString &text, wxString &head, wxString &tail, const wxString &sep)
{
	if (text.Length() > 1 && text[0] == wxT('"'))
	{
		size_t i = text.find(wxT('"'), 1);
		while (i+1 < text.Length())
		{
			if (text[i+1] != wxT('"')) break;
			i = text.find(wxT('"'), i + 2);
		}
		if (i < text.Length())
		{
			head = text.Mid(1, i-1);
			if (i+1 < text.Length() && text[i+1] == ' ')
			{
				tail = text.Mid(i + 2);
			}
			else
			{
				tail = text.Mid(i + 1);
			}
		}
		else
		{
			head = text.Mid(1);
			tail = wxEmptyString;
		}
		head.Replace("\"\"", "\"");

	}
	else
	{
		SplitHeadTail(text, head, tail, sep);
	}
}

HeadTail SplitQuotedHeadTail(const wxString &text, const wxString &sep)
{
	HeadTail result;
	SplitQuotedHeadTail(text, result.head, result.tail, sep);
	return result;
}

wxString GetShortTimestamp()
{
	return wxDateTime::Now().Format(wxT("[%H:%M] "));
}

wxString GetLongTimestamp()
{
	return wxDateTime::Now().Format(wxT("%Y/%m/%d %H:%M:%S "));
}

wxString FormatISODateTime(const wxDateTime &datetime)
{
	return datetime.Format(wxT("%Y/%m/%d %H:%M:%S"));
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
		parser->SetFonts(wxEmptyString, wxT("Fixedsys"), default_sizes);
	#endif

}

wxString AddCommas(off_t size)
{
	wxASSERT(size >= 0);
	wxString buff;
	while (size >= 1000)
	{
		buff = wxString::Format(wxT("%03d"), size % 1000) + buff;
		buff = wxT(',') + buff;
		size /= 1000;
	}
	return wxString() << (int)size << buff;
}

wxString AddCommas(double size)
{

	off_t size2 = (off_t)size;
	
	wxString remainder = wxString::Format(wxT("%04.2lf"), fmod(size, 1));
	wxASSERT(remainder.Left(2) == wxT("0.") || remainder.Left(2) == wxT("1."));
	if (remainder.Left(2) == wxT("1."))
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
		return wxString() << (int)size << wxT(" bytes");
	}
	else if (size < 524288)
	{
		return wxString() << AddCommas(size / pow(1024, 1)) << wxT(" KB");
	}
	else if (size < 1073741824)
	{
		return wxString() << AddCommas(size / pow(1024, 2)) << wxT(" MB");
	}
	else if (size < 1099511627776)
	{
		return wxString() << AddCommas(size / pow(1024, 3)) << wxT(" GB");
	}
	else
	{
		return wxString() << AddCommas(size / pow(1024, 4)) << wxT(" TB");
	}
}

wxString SizeToLongString(off_t size, wxString suffix)
{
	wxString result;
	result << SizeToString(size) << suffix;
	if (size >= 1000)
	{	
		result << wxT(" (") << AddCommas(size) << wxT(" bytes") << suffix << wxT(")");
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
		result = wxString::Format(wxT("%02d:%02d:%02d"), hour, min, sec);
	}
	else
	{
		result = wxString::Format(wxT("%02d:%02d"), min, sec);
	}

	if (neg)
	{
		result = wxT("-") + result;
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

wxString AppTitle(const wxString &suffix)
{
	return
		wxT("Dirt Secure Chat") +
		(suffix.Length() ? (wxT(" ") + suffix + wxT(" ")) : wxT(" ")) +
		GetProductVersion();
}

void ShowAbout()
{
	wxMessageBox(wxString()
		<< wxT("Dirt Secure Chat 3.0.0 Alpha 0\n")
		<< wxT("\n")
		<< wxT("Last revision date: ") << GetRCSDate() << wxT(" UTC\n")
		<< wxT("Last revision author: ") << GetRCSAuthor() << wxT("\n")
		<< wxT("\n")
		<< wxT("http://dirtchat.sourceforge.net/"),
		wxT("About Dirt Secure Chat"), wxICON_INFORMATION);
}

ByteBuffer EncodeMessage(const wxString &context, const wxString &cmd, const ByteBuffer &data)
{
	ByteBufferArray tmp;
	tmp.Alloc(3);
	tmp.Add(context);
	tmp.Add(cmd);
	tmp.Add(data);
	return Pack(tmp);
}

bool DecodeMessage(const ByteBuffer &msg, wxString &context, wxString &cmd, ByteBuffer &data)
{
	ByteBufferArray tmp = Unpack(msg, 3);
	if (tmp.GetCount() == 3)
	{
		context = tmp.Item(0);
		cmd = tmp.Item(1);
		data = tmp.Item(2);
		return true;
	}
	return false;
}

ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y)
{
	ByteBufferArray tmp;
	tmp.Alloc(2);
	tmp.Add(x);
	tmp.Add(y);
	return Pack(tmp);
}

bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y)
{
	ByteBufferArray tmp = Unpack(data, 2);
	x = (tmp.GetCount() > 0) ? tmp.Item(0) : ByteBuffer();
	y = (tmp.GetCount() > 1) ? tmp.Item(1) : ByteBuffer();
	return (tmp.GetCount() == 2);
}

ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y, const ByteBuffer &z)
{
	ByteBufferArray tmp;
	tmp.Alloc(3);
	tmp.Add(x);
	tmp.Add(y);
	tmp.Add(z);
	return Pack(tmp);
}

bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y, ByteBuffer &z)
{
	ByteBufferArray tmp = Unpack(data, 3);
	x = (tmp.GetCount() > 0) ? tmp.Item(0) : ByteBuffer();
	y = (tmp.GetCount() > 1) ? tmp.Item(1) : ByteBuffer();
	z = (tmp.GetCount() > 2) ? tmp.Item(2) : ByteBuffer();
	return (tmp.GetCount() == 3);
}

ByteBuffer Pack(const ByteBufferArray &array)
{
	size_t total_len = 0;
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		if (i > 0)
		{
			total_len++;
		}
		total_len += array.Item(i).Length();
	}
	ByteBuffer buff(total_len);
	byte *ptr = buff.Lock();
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		ByteBuffer tmp(array.Item(i));
		const byte *tmpptr = tmp.Lock();
		memcpy(ptr, tmpptr, tmp.Length());
		ptr += tmp.Length() + 1;
		tmp.Unlock();
	}
	buff.Unlock();
	return buff;
}

ByteBufferArray Unpack(const ByteBuffer &packed_array, size_t max_segments)
{

	if (packed_array.Length() == 0)
	{
		return ByteBufferArray();
	}
	else
	{
	
		ByteBufferArray array;
		ByteBuffer src(packed_array);
		const byte *ptr = src.Lock();
		size_t len = src.Length();

		const byte *sep = (const byte*)memchr(ptr, 0, len);
		while (sep && ((max_segments == 0) || (array.GetCount() < max_segments-1)))
		{
			size_t seglen = sep-ptr;
			ByteBuffer b(ptr, seglen);
			array.Add(b);
			ptr += seglen;
			len -= seglen;
			if (!len)
			{
				break;
			}
			ptr++; len--;
			if (!len)
			{
				break;
			}
			sep = (const byte*)memchr(ptr, 0, len);
		}

		ByteBuffer b(ptr, len);
		array.Add(b);
		
		src.Unlock();
		return array;

	}

}

ByteBuffer PackHashMap(const StringHashMap &hashmap)
{
	ByteBufferArray list;
	for (StringHashMap::const_iterator i = hashmap.begin(); i != hashmap.end(); ++i)
	{
		list.Add(i->first);
		list.Add(i->second);
	}
	wxASSERT(list.GetCount() == hashmap.size() * 2);
	return Pack(list);
}

StringHashMap UnpackHashMap(const ByteBuffer &packed_hashmap)
{
	StringHashMap hashmap;
	ByteBufferArray list(Unpack(packed_hashmap));
	wxASSERT(list.GetCount() % 2 == 0);
	for (size_t i = 0; i < list.GetCount(); i += 2)
	{
		hashmap[list.Item(i)] = list.Item(i+1);
	}
	wxASSERT(list.GetCount() == hashmap.size() * 2);
	return hashmap;
}
