#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: util.cpp,v 1.22 2003-02-18 13:31:00 jason Exp $)

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

	ByteBuffer context_buff(context);
	ByteBuffer cmd_buff(cmd.Upper());

	ByteBuffer msg(context_buff.Length() + cmd_buff.Length() + data.Length() + 2);
	
	byte *msgptr = msg.Lock();

	memcpy(msgptr, context_buff.Lock(), context_buff.Length());
	context_buff.Unlock();
	msgptr += context_buff.Length() + 1;

	memcpy(msgptr, cmd_buff.Lock(), cmd_buff.Length());
	cmd_buff.Unlock();
	msgptr += cmd_buff.Length() + 1;

	ByteBuffer tmp(data);
	memcpy(msgptr, tmp.Lock(), tmp.Length());
	tmp.Unlock();

	msg.Unlock();

	return msg;

}

bool DecodeMessage(const ByteBuffer &msg, wxString &context, wxString &cmd, ByteBuffer &data)
{

	ByteBuffer msg2(msg);
	byte *dataptr = msg2.Lock();
	size_t datalen = msg2.Length();
	byte *sep1 = (byte*)memchr(dataptr, 0, datalen);
	if (!sep1)
	{
		msg2.Unlock();
		return false;
	}
	byte *sep2 = (byte*)memchr(sep1+1, 0, datalen - (sep1-dataptr) - 1);
	if (!sep2)
	{
		msg2.Unlock();
		return false;
	}
	
	context = ByteBuffer(dataptr, sep1 - dataptr);
	cmd = ByteBuffer(sep1+1, sep2-sep1-1);
	data = ByteBuffer(sep2+1, datalen - (sep2-dataptr) - 1);

	msg2.Unlock();

	return true;

}

ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y)
{
	ByteBuffer x2(x), y2(y);
	ByteBuffer data(x.Length() + y.Length() + 1);
	byte *ptr = data.Lock();
	memcpy(ptr, x2.Lock(), x2.Length());
	x2.Unlock();
	memcpy(ptr+x2.Length()+1, y2.Lock(), y2.Length());
	y2.Unlock();
	data.Unlock();
	return data;
}

bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y)
{
	ByteBuffer data2(data);
	byte *ptr = data2.Lock();
	byte *sep = (byte*)memchr(ptr, 0, data2.Length());
	if (!sep)
	{
		data2.Unlock();
		return false;
	}
	x = ByteBuffer(ptr, sep - ptr);
	y = ByteBuffer(sep+1, data2.Length() - (sep-ptr) - 1);
	data2.Unlock();
	return true;
}

// faster implementation of this should be written
// use a mod of EncodeMessage then change DecodeMessage to use new func
ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y, const ByteBuffer &z)
{
	return Pack(Pack(x, y), z);
}

// faster implementation of this should be written
// use a mod of DecodeMessage then change DecodeMessage to use new func
bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y, ByteBuffer &z)
{
	ByteBuffer tmp;
	return Unpack(data, x, tmp) && Unpack(tmp, y, z);
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
		byte *tmpptr = tmp.Lock();
		memcpy(ptr, tmpptr, tmp.Length());
		ptr += tmp.Length() + 1;
		tmp.Unlock();
	}
	buff.Unlock();
	return buff;
}

ByteBufferArray Unpack(const ByteBuffer &packed_array)
{

	if (packed_array.Length() == 0)
	{
		return ByteBufferArray();
	}
	else
	{
	
		ByteBufferArray array;
		ByteBuffer src(packed_array);
		byte *ptr = src.Lock();
		size_t len = src.Length();

		byte *sep = (byte*)memchr(ptr, 0, len);
		while (sep)
		{
			size_t seglen = sep-ptr;
			ByteBuffer b(ptr, seglen);
			array.Add(b);
			ptr += (seglen + 1);
			len -= (seglen + 1);
			if (len<=0) break;
			sep = (byte*)memchr(ptr, 0, len);
		}

		if (len)
		{
			ByteBuffer b(ptr, len);
			array.Add(b);
		}
		
		src.Unlock();
		return array;

	}

}
