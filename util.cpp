#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: util.cpp,v 1.81 2004-01-24 05:23:09 jason Exp $)

#include "util.h"
#include <wx/datetime.h>
#include <math.h>
#include "ByteBuffer.h"
#include <wx/mimetype.h>
#include <wx/confbase.h>
#include <wx/filename.h>

#if defined(__WXMSW__)
	#include <windows.h>
	#include <wx/msw/winundef.h>
#elif defined(__UNIX__)
	#include <unistd.h>
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(Uint16Array)

const wxString PUBLIC_LIST_URL = wxT("http://dirt.gslabs.com.au/cgi-bin/dirt.pl");

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

wxArrayString SplitQuotedString(const wxString &text, const wxString &sep)
{
	HeadTail ht;
	wxArrayString array;
	wxString buff(text);
	while (buff.Trim(false).Trim(true).Length())
	{
		ht = SplitQuotedHeadTail(buff, sep);
		array.Add(ht.head);
		buff = ht.tail;
	}
	return array;
}

wxString StripQuotes(const wxString &text)
{
	if (text.Length() > 1 && text[0u] == wxT('"') && text.Last() == wxT('"'))
	{
		return text.Mid(1, text.Length() - 2);
	}
	return text;
}

wxString JoinArray(const wxArrayString &array, const wxString &sep, const wxString &prefix, const wxString &postfix)
{
	wxString str;
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		if (i > 0)
		{
			str += sep;
		}
		str += prefix + array.Item(i) + postfix;
	}
	return str;
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
		head.Replace(wxT("\"\""), wxT("\""));

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

wxString AddCommas(wxLongLong_t size)
{
	wxASSERT(size >= 0);
	wxString buff;
	while (size >= 1000)
	{
		buff = wxString::Format(wxT("%03d"), (int)(size % 1000)) + buff;
		buff = wxT(',') + buff;
		size /= 1000;
	}
	return wxString() << (int)size << buff;
}

wxString AddCommas(double size)
{

	wxLongLong_t size2 = (wxLongLong_t)size;
	
	wxString remainder = wxString::Format(wxT("%04.2lf"), fmod(size, 1));
	wxASSERT(remainder.Left(2) == wxT("0.") || remainder.Left(2) == wxT("1."));
	if (remainder.Left(2) == wxT("1."))
	{
		size2++;
	}
	remainder = remainder.Mid(1);

	wxString quotient = AddCommas((wxLongLong_t)size2);

	return wxString() << quotient << remainder;

}

wxString SizeToString(wxLongLong_t size)
{
	wxASSERT(size >= 0);
	wxLongLong_t size2 = size;
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
	else if (size2 < wxLL(1099511627776))
	{
		return wxString() << AddCommas(size / pow(1024, 3)) << wxT(" GB");
	}
	else
	{
		return wxString() << AddCommas(size / pow(1024, 4)) << wxT(" TB");
	}
}

wxString SizeToLongString(wxLongLong_t size, wxString suffix)
{
	wxString result;
	result << SizeToString(size) << suffix;
	if (size >= 1000)
	{	
		result << wxT(" (") << AddCommas(size) << wxT(" bytes") << suffix << wxT(")");
	}
	return result;
}

inline const wxChar* plural(int x)
{
	return (x == 1 || x == -1) ? wxT("") : wxT("s");
}

wxString SecondsToMMSS(long seconds, bool milliseconds, bool verbose)
{

	bool neg = (seconds < 0);

	if (neg)
	{
		seconds = -seconds;
	}

	int ms = 0;

	if (milliseconds)
	{
		ms = seconds % 1000;
		seconds /= 1000;
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
		if (verbose)
		{
			result = wxString::Format(wxT("%d hour%s, "), hour, plural(hour));
		}
		else
		{
			result = wxString::Format(wxT("%02d:"), hour);
		}
	}

	if (verbose)
	{
		if (hour || min)
		{
			result += wxString::Format(wxT("%d minute%s, "), min, plural(min));
		}
		result += wxString::Format(wxT("%d second%s"), sec, plural(sec));
	}
	else
	{
		result += wxString::Format(wxT("%02d:%02d"), min, sec);
	}

	if (milliseconds)
	{
		if (verbose)
		{
			if (hour || min || sec)
			{
				result += wxT(", ");
			}
			else
			{
				result.Empty();
			}
			result += wxString::Format(wxT("%d millisecond%s"), ms, plural(ms));
		}
		else
		{
			result += wxString::Format(wxT(".%03d"), ms);
		}
	}

	if (neg)
	{
		result = wxT("-") + result;
	}

	return result;

}

ByteBuffer Uint64ToBytes(unsigned wxLongLong_t num)
{
	wxULongLong ull(num);
	return Uint32ToBytes(ull.GetHi()) + Uint32ToBytes(ull.GetLo());
}

unsigned wxLongLong_t BytesToUint64(const byte *data, int WXUNUSED_UNLESS_DEBUG(len))
{
	wxASSERT((len == 8) && (len == sizeof (wxUint64)) && (len == sizeof (wxULongLong)));
	wxUint32 hi = BytesToUint32(data + 0, 4);
	wxUint32 lo = BytesToUint32(data + 4, 4);
	return wxULongLong(hi, lo).GetValue();
}

ByteBuffer Uint32ToBytes(wxUint32 num)
{
	ByteBuffer bytes(4);
	byte *ptr = bytes.LockReadWrite();
	ptr[0] = ((num >> 24) & 0xff);
	ptr[1] = ((num >> 16) & 0xff);
	ptr[2] = ((num >> 8)  & 0xff);
	ptr[3] = ((num >> 0)  & 0xff);
	bytes.Unlock();
	return bytes;
}

wxUint32 BytesToUint32(const byte *data, int WXUNUSED_UNLESS_DEBUG(len))
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
	byte *ptr = bytes.LockReadWrite();
	ptr[0] = ((num >> 8)  & 0xff);
	ptr[1] = ((num >> 0)  & 0xff);
	bytes.Unlock();
	return bytes;
}

wxUint16 BytesToUint16(const byte *data, int WXUNUSED_UNLESS_DEBUG(len))
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
		<< wxT("Dirt Secure Chat ") << GetProductVersion() << wxT("\n")
		<< wxT("\n")
		<< wxT("Built with ") << wxVERSION_STRING << wxT("\n")
		<< wxT("\n")
		<< wxT("Last revision date: ") << GetRCSDate() << wxT(" UTC\n")
		<< wxT("Last revision author: ") << GetRCSAuthor() << wxT("\n")
		<< wxT("\n")
		<< wxT("http://dirt.gslabs.com.au/"),
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
	byte *ptr = buff.LockReadWrite();
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		ByteBuffer tmp(array.Item(i));
		const byte *tmpptr = tmp.LockRead();
		memcpy(ptr, tmpptr, tmp.Length());
		ptr += tmp.Length() + 1;
		tmp.Unlock();
	}
	buff.Unlock();
	return buff;
}

ByteBuffer Pack(const wxArrayString &array)
{
	ByteBufferArray buff_array;
	buff_array.Alloc(array.GetCount());
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		buff_array.Add(array.Item(i));
	}
	return Pack(buff_array);
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
		const byte *ptr = src.LockRead();
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

wxArrayString ByteBufferArrayToArrayString(const ByteBufferArray &array)
{
	wxArrayString str_array;
	str_array.Alloc(array.GetCount());
	for (size_t i = 0; i < array.GetCount(); ++i)
	{
		str_array.Add(array.Item(i));
	}
	return str_array;
}

ByteBuffer PackStringHashMap(const StringHashMap &hashmap)
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

StringHashMap UnpackStringHashMap(const ByteBuffer &packed_hashmap)
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

ByteBuffer PackByteBufferHashMap(const ByteBufferHashMap &hashmap)
{
	ByteBufferArray list;
	for (ByteBufferHashMap::const_iterator i = hashmap.begin(); i != hashmap.end(); ++i)
	{
		list.Add(i->first);
		list.Add(i->second);
	}
	wxASSERT(list.GetCount() == hashmap.size() * 2);
	return Pack(list);
}

ByteBufferHashMap UnpackByteBufferHashMap(const ByteBuffer &packed_hashmap)
{
	ByteBufferHashMap hashmap;
	ByteBufferArray list(Unpack(packed_hashmap));
	wxASSERT(list.GetCount() % 2 == 0);
	for (size_t i = 0; i < list.GetCount(); i += 2)
	{
		hashmap[list.Item(i)] = list.Item(i+1);
	}
	wxASSERT(list.GetCount() == hashmap.size() * 2);
	return hashmap;
}

const byte* findbytes(const byte *buff, size_t buff_len, const byte *lookfor, size_t lookfor_len)
{
	if (lookfor_len <= buff_len)
	{
		const byte *ptr = buff;
		size_t len = buff_len;
		while ((ptr = (const byte*)memchr(ptr, lookfor[0], len)) != NULL)
		{
			size_t len2 = ptr - buff;
			if (len2+lookfor_len > buff_len) break;
			
			const byte *buff1 = ptr;
			const byte *buff2 = lookfor;
			size_t x = lookfor_len;
			while (x && *buff1 == *buff2)
			{
				buff1++;
				buff2++;
				x--;
			}
			if (!x)
			{
				return ptr;
			}
			ptr++;
			len = buff_len - len2;
			if (!len) break;
			len--;
			if (!len) break;
		}
	}
	return NULL;
}

wxLongLong_t GetMillisecondTicks()
{
	#ifdef __WXMSW__
		return ::timeGetTime();
	#else
		return wxGetLocalTimeMillis().GetValue();
	#endif
}

wxString GetPublicListURL()
{
	return PUBLIC_LIST_URL;
}

bool OpenBrowser(wxWindow *parent, const wxString &URL, bool show_error)
{

	wxLogNull supress_log;

	#ifdef __WXMSW__

		return OpenFile(parent, URL, show_error);

	#else

		const wxChar *browsers[2] = { wxT("mozilla"), wxT("netscape") };
		const size_t num_browsers = ((sizeof browsers) / (sizeof browsers[0]));

		bool success = false;

		for (int i = 0; i < num_browsers && !success; ++i)
		{
			wxString cmdline;
			cmdline << browsers[i] << wxT(' ') << URL;
			::wxBeginBusyCursor();
			long pid = ::wxExecute(cmdline, wxEXEC_ASYNC);
			::wxEndBusyCursor();
			success = (pid != 0);
		}

		if (!success)
		{

			wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(wxT("html"));
			if ( !ft )
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to determine the file type for HTML."),
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}

			wxString cmd;
			bool ok = ft->GetOpenCommand(
				&cmd, wxFileType::MessageParameters(URL, wxT("")));
			delete ft;

			if (!ok)
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to determine the command for running your HTML browser."),
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}

			// GetOpenCommand can prepend file:// even if it already has http://
			if (cmd.Find(wxT("http://")) != -1)
			{
				cmd.Replace(wxT("file://"), wxT(""));
			}

			ok = (wxExecute(cmd, FALSE) != 0);

			if (!ok)
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to navigate to ") + URL,
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}
	
		}

		return true;

	#endif

}

void ForceForegroundWindow(wxFrame *frm)
{
	
	#if defined(__WXMSW__)
		
		HWND hWnd = (HWND)frm->GetHWND();
		
		if (hWnd != GetForegroundWindow())
		{
			
			frm->Show();
			
			DWORD ThreadID1 = GetWindowThreadProcessId(GetForegroundWindow(), 0);
			DWORD ThreadID2 = GetWindowThreadProcessId(hWnd, 0);
			bool flag = (ThreadID1 != ThreadID2);

			if (flag)
			{
				AttachThreadInput(ThreadID1, ThreadID2, TRUE);
			}
			
			SetForegroundWindow(hWnd);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			
			if (flag)
			{
				AttachThreadInput(ThreadID1, ThreadID2, FALSE);
			}

			if (IsIconic(hWnd))
			{
				ShowWindow(hWnd, SW_RESTORE);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOW);
			}

		}

	#else

		frm->Show(false);
		frm->Show(true);
		frm->SetFocus();

	#endif

}

void GetWindowState(const wxFrame *frm, wxRect& r, bool& maximized)
{
	
	maximized = frm->IsMaximized();

	#ifdef __WIN32__

		HWND hWnd = (HWND)frm->GetHandle();
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof (wp));
		wp.length = sizeof (WINDOWPLACEMENT);

		if (GetWindowPlacement(hWnd, &wp))
		{

			RECT& wr = wp.rcNormalPosition;

			r.x = wr.left;
			r.y = wr.top;
			r.width = wr.right - wr.left;
			r.height = wr.bottom - wr.top;

			maximized = (IsZoomed(hWnd) != FALSE);

		}

	#else

		r = frm->GetRect();

	#endif

}

void SetWindowState(wxFrame *frm, const wxRect &r, const bool maximized, bool show)
{

	#ifdef __WIN32__

		if (show && !frm->IsShown())
		{
			frm->Move(10000,10000);
			frm->Show();
		}

		HWND hwnd = (HWND) frm->GetHandle();
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof (wp));
		wp.length = sizeof (WINDOWPLACEMENT);
		wp.flags = 0;
		wp.showCmd = (show||frm->IsShown()) ? (maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL) : SW_HIDE;

		RECT& rc = wp.rcNormalPosition;

		rc.left = r.x;
		rc.top = r.y;
		rc.right = r.x + r.width;
		rc.bottom = r.y + r.height;

		SetWindowPlacement(hwnd, &wp);

	#else

		frm->SetSize(r);
		frm->Maximize(maximized);
		if (show)
		{
			frm->Show();
		}

	#endif

}

void SaveWindowState(const wxFrame *frm, wxConfigBase *cfg, const wxString &name)
{

	wxString path;
	if (name.Length())
	{
		path = ((name[0u] != wxT('/')) ? wxT("/") : wxT("")) + name;
	}
	path += wxT("/WindowState");

	wxString old_path = cfg->GetPath();
	cfg->SetPath(path);

	wxRect r;
	bool maximized;
	GetWindowState(frm, r, maximized);

	cfg->Write(wxT("X"), long(r.x));
	cfg->Write(wxT("Y"), long(r.y));
	cfg->Write(wxT("Width"), long(r.width));
	cfg->Write(wxT("Height"), long(r.height));
	cfg->Write(wxT("Maximized"), maximized);
	cfg->Flush();

	cfg->SetPath(old_path);

}

void RestoreWindowState(wxFrame *frm, wxConfigBase *cfg, const wxString &name, bool show, bool default_maximized)
{

	wxString path;
	if (name.Length())
	{
		path = ((name[0u] != wxT('/')) ? wxT("/") : wxT("")) + name;
	}
	path += wxT("/WindowState");

	wxString old_path = cfg->GetPath();
	cfg->SetPath(path);

	wxRect r;
	bool maximized;
	GetWindowState(frm, r, maximized);

	maximized |= default_maximized;

	cfg->Read(wxT("X"), &r.x, r.x);
	cfg->Read(wxT("Y"), &r.y, r.y);
	cfg->Read(wxT("Width"), &r.width, r.width);
	cfg->Read(wxT("Height"), &r.height, r.height);
	cfg->Read(wxT("Maximized"), &maximized, maximized);

	SetWindowState(frm, r, maximized, show);

	cfg->SetPath(old_path);

}

wxDateTime ParseDateTime(const wxString &str, bool okay_to_presume_future)
{
	wxDateTime dt;
	HeadTail ht = SplitHeadTail(str);
	bool has_colon[2] = { ht.head.Contains(wxT(':')), ht.tail.Contains(wxT(':')) };
	if (!has_colon[0] && !has_colon[1])
	{
		return wxInvalidDateTime;
	}
	if (has_colon[0] && !has_colon[1])
	{
		wxString tmp;
		tmp = ht.head;
		ht.head = ht.tail;
		ht.tail = tmp;
	}
	ht.head.Trim(true).Trim(false);
	ht.tail.Trim(true).Trim(false);
	if (ht.head.Length())
	{
		const wxChar *begin = ht.head.c_str();
		const wxChar *end = dt.ParseDate(begin);
		if ((size_t)(end - begin) != ht.head.Length())
		{
			return wxInvalidDateTime;
		}
	}
	if (ht.tail.Length())
	{
		const wxChar *begin = ht.tail.c_str();
		const wxChar *end = dt.ParseTime(begin);
		if ((size_t)(end - begin) != ht.tail.Length())
		{
			return wxInvalidDateTime;
		}
	}
	if (!ht.head.Length() && okay_to_presume_future)
	{
		if (dt < wxDateTime::Now())
		{
			dt += wxTimeSpan::Day();
		}
	}
	return dt;
}

wxString GetSelf()
{

	wxString first_arg =
		(wxTheApp->argc > 0) ? wxTheApp->argv[0] : wxEmptyString;

	#if defined(__WXMSW__)

		wxChar buff[4096];

		DWORD len = GetModuleFileName(NULL, buff, WXSIZEOF(buff));

		return len?wxString(buff,len):first_arg;

	#elif defined(__UNIX__)

		char buff[4096];

		int len = readlink("/proc/self/exe", buff, WXSIZEOF(buff));
		wxString filename(buff, wxConvLocal, wxMax(0, len));
		if (len > 0 && wxFileName(filename).FileExists())
		{
			return filename;
		}
		else
		{
			if (realpath(first_arg.mb_str(), buff))
			{
				return wxString(buff, wxConvLocal);
			}
			else
			{
				return first_arg;
			}
		}

	#else

		return first_arg;

	#endif

}

bool StringToLongLong(const wxString &str, wxLongLong_t *x)
{
	wxChar c;
	*x = 0;
	int retval = wxSscanf(str.c_str(), wxT("%") wxLongLongFmtSpec wxT("d%c"), x, &c);
	return (retval == 1);
}

bool StringToULongLong(const wxString &str, unsigned wxLongLong_t *x)
{
	wxChar c;
	*x = 0;
	int retval = wxSscanf(str.c_str(), wxT("%") wxLongLongFmtSpec wxT("u%c"), x, &c);
	return (retval == 1);
}

bool OpenFile(wxWindow *parent, const wxString &filename, bool show_error)
{

	wxLogNull supress_log;

	#ifdef __WXMSW__

		::wxBeginBusyCursor();
		HINSTANCE hInstance = ::ShellExecute((HWND)(parent->GetHandle()), wxT("open"), filename, NULL, NULL, SW_NORMAL);
		::wxEndBusyCursor();
		bool success = ((int)hInstance > 32);
		if (!success)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to open ") + filename,
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}
		return true;

	#else
		
		wxString ext = wxFileName(filename).GetExt();
		wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
		if ( !ft )
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to determine the file type for the extension \"") + ext + wxT("\"."),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}

		wxString cmd;
		bool ok = ft->GetOpenCommand(
			&cmd, wxFileType::MessageParameters(filename, wxT("")));
		delete ft;

		if (!ok)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to determine the command for executing files with extension \"") + ext + wxT("\"."),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}

		// GetOpenCommand can prepend file:// even if it already has http://
		if (cmd.Find(wxT("http://")) != -1)
		{
			cmd.Replace(wxT("file://"), wxT(""));
		}

		ok = (wxExecute(cmd, FALSE) != 0);

		if (!ok)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to execute \"") + cmd + wxT("\""),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}
				
		return true;

	#endif

}

bool OpenFolder(wxWindow *parent, const wxString &folder, bool show_error)
{
	#if defined(KDE_AVAILABLE)
		wxString cmdline;
		cmdline << wxT("konqueror ") << folder;
		::wxBeginBusyCursor();
		long pid = ::wxExecute(cmdline, wxEXEC_ASYNC);
		::wxEndBusyCursor();
		bool success = (pid != 0);
		if (!success && show_error)
		{
			wxMessageBox(
				wxT("Unable to open ") + folder,
				wxT("Error launching Konqueror"), wxOK | wxICON_ERROR, parent);
		}
		return success;
	#elif defined(__WXMSW__)
		return OpenFile(parent, folder, show_error);
	#else
		return OpenBrowser(parent, wxT("file://") + folder, show_error);
	#endif
}

bool OpenExternalResource(wxWindow *parent, const wxString &name, bool show_error)
{

	if (wxFileName(name, wxEmptyString).DirExists())
	{
		return OpenFolder(parent, name, show_error);
	}

	if (wxFileName(name).FileExists())
	{
		return OpenFile(parent, name, show_error);
	}

	return
		OpenFile(parent, name, false) ||
		OpenBrowser(parent, name, show_error);

}

bool SetDefaultMenuItem(wxMenu &mnu, int id)
{

	#ifdef __WXMSW__

		HMENU hMenu = (HMENU)mnu.GetHMenu();
		wxMenuItem *item = mnu.FindItem(id);
		if (item)
		{
			int real_id = item->GetRealId();
			MENUITEMINFO mii;
			mii.cbSize = sizeof (MENUITEMINFO);
			mii.fMask = MIIM_STATE;
			if (GetMenuItemInfo(hMenu, real_id, FALSE, &mii))
			{
				mii.fState |= MFS_DEFAULT;
				if (SetMenuItemInfo(hMenu, real_id, FALSE, &mii))
				{
					return true;
				}
			}
		}
		return false;

	#else

		return false;

	#endif

}

wxString CaseInsensitiveReplace(const wxString &text, const wxString &old_value, const wxString &new_value)
{
	const wxString text_upper = text.Upper();
	const wxString old_value_upper = old_value.Upper();
	wxASSERT(text.Length() == text_upper.Length());
	wxASSERT(old_value.Length() == old_value_upper.Length());
	wxString output = text;
	size_t start_pos = 0;
	int offset = 0;
	size_t find_pos = old_value_upper.Length()?text_upper.find(old_value_upper, start_pos):wxString::npos;
	while (find_pos != wxString::npos)
	{
		output.replace(find_pos+offset, old_value_upper.Length(), new_value);
		start_pos = find_pos + old_value_upper.Length();
		offset += new_value.Length() - old_value.Length();
		find_pos = old_value_upper.Length()?text_upper.find(old_value_upper, start_pos):wxString::npos;
	}
	return output;
}

#ifdef __UNIX__
#include <sys/utsname.h>
#endif

wxString GetOSDescription()
{
#ifdef __UNIX__
	struct utsname name;
	if (uname (&name) == -1)
	{
		return wxEmptyString;
	}
	wxString str;
	str << wxString(name.sysname, wxConvLocal)
		<< wxT(" ")
		<< wxString(name.release, wxConvLocal);
	return str;
#else
	return wxGetOsDescription();
#endif
}
