#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: URL.cpp,v 1.5 2003-02-17 02:29:45 jason Exp $)

#include "URL.h"
#include "ByteBuffer.h"

URL::URL()
{
	m_port = 0;
}

URL::URL(const wxString &url)
{ // not implemented
	m_protocol = wxEmptyString;
	m_hostname = wxEmptyString;
	m_port = 0;
	m_username = wxEmptyString;
	m_password = wxEmptyString;
	m_path = wxEmptyString;
	m_query = wxEmptyString;
	m_reference = wxEmptyString;
}

URL::~URL()
{
}


URL::operator wxString() const
{
	wxString url;
	if (GetProtocol().Length())
	{
		url << Escape(GetProtocol()) << wxT("://");
	}
	if (GetAuthentication().Length())
	{
		url << Escape(GetAuthentication()) << wxT('@');
	}
	url << Escape(GetHostname());
	if (GetPort())
	{
		url << wxT(':') << GetPort();
	}
	if (GetPath() != wxT("/") || GetProtocol().Length() || GetQuery().Length() || GetReference().Length())
	{
		url << Escape(GetPath());
	}
	if (GetQuery().Length())
	{
		url << wxT('?') << Escape(GetQuery());
	}
	if (GetReference().Length())
	{
		url << wxT('#') << Escape(GetReference());
	}
	return url;
}

wxString URL::Escape(const wxString &text)
{
	wxString output;
	output.Alloc(text.Length() * 4);
	ByteBuffer src(text);
	byte *ptr = src.Lock();
	size_t len = src.Length();
	for (size_t i = 0; i < len; ++i)
	{
		byte b = ptr[i];
		if (b < 33 || b > 125 || b == 91 || b == 93 ||
			b == 43 || b == 63 || b == 38 || b == 43 ||
			b == 61 || b == 34 || b == 39 || b == 58 ||
			b == 37 || b == 13 || b == 10 || b == 9 ||
			b == 60 || b == 62 || b == 35 || b == 43)
		{
			output += wxString::Format(wxT("%%%02x"), (int)b);
		}
		else
		{
			output += (wxChar)b;
		}
	}
	src.Unlock();
	return output;
}

wxString URL::Unescape(const wxString &text)
{
	ByteBuffer src(text);
	ByteBuffer dst(text.Length());
	byte *srcptr = src.Lock();
	size_t srclen = src.Length();
	byte *dstptr = dst.Lock();
	for (size_t i = 0; i < srclen; ++i)
	{
		byte b = srcptr[i];
		if (b == '+')
		{
			*dstptr = ' ';
		}
		else if (b == '%' && (i+2 < srclen) && isxdigit(srcptr[i+1]) && isxdigit(srcptr[i+2]))
		{
			wxString hex;
			hex << (wxChar)srcptr[i+1] << (wxChar)srcptr[i+2];
			int c = 0;
			wxSscanf(hex, wxT("%x"), &c);
			i += 2;
			*dstptr = c;
		}
		else
		{
			*dstptr = b;
		}
		dstptr++;
	}
	*dstptr = 0;
	src.Unlock();
	dst.Unlock();
	return dst;
}

wxString URL::GetProtocol(const wxString &default_protocol) const
{
	return m_protocol.Length() ? m_protocol : default_protocol;
}

wxString URL::GetHostname() const
{
	return m_hostname;
}

int URL::GetPort(int default_port) const
{
	return m_port ? m_port : default_port;
}

wxString URL::GetAuthentication() const
{
	if (m_password.Length() > 0)
	{
		return m_username + wxT(':') + m_password;
	}
	else
	{
		return m_username;
	}
}

wxString URL::GetUsername() const
{
	return m_username;
}

wxString URL::GetPassword() const
{
	return m_password;
}

wxString URL::GetPath() const
{
	if (m_path.Length() == 0)
	{
		return wxT("/");
	}
	else
	{
		wxASSERT(m_path[0] == wxT('/'));
		return m_path;
	}
}

wxString URL::GetQuery() const
{
	return m_query;
}

wxString URL::GetReference() const
{
	return m_reference;
}


void URL::SetProtocol(const wxString &protocol)
{
	wxASSERT(protocol.Find(wxT(':')) == -1 && protocol.Find(wxT('/')) == -1);
	m_protocol = protocol.Lower();
}

void URL::SetHostname(const wxString &hostname)
{
	wxASSERT(hostname.Find(wxT(':')) == -1 && hostname.Find(wxT('/')) == -1);
	m_hostname = hostname;
}

void URL::SetPort(int port)
{
	wxASSERT(port >= 0);
	m_port = port;
}

void URL::SetAuthentication(const wxString &authentication)
{
	int index = authentication.Find(wxT(':'), true);
	if (index == wxNOT_FOUND)
	{
		m_username = authentication;
		m_password = wxEmptyString;
	}
	else
	{
		m_username = authentication.Left(index);
		m_password = authentication.Mid(index + 1);
	}
}

void URL::SetUsername(const wxString &username)
{
	m_username = username;
}

void URL::SetPassword(const wxString &password)
{
	m_password = password;
}

void URL::SetPath(const wxString &path)
{
	if (path.Length() > 0 && path[0] != wxT('/'))
	{
		m_path = wxT('/') + path;
	}
	else
	{
		m_path = path;
	}
}

void URL::SetQuery(const wxString &query)
{
	m_query = query;
}

void URL::SetReference(const wxString &reference)
{
	m_reference = reference;
}
