/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ByteBuffer.cpp,v 1.18 2003-11-24 07:01:46 jason Exp $)

#include "ByteBuffer.h"
#include "util.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ByteBufferArray)

struct ByteBufferData
{
	byte *buffer;
	size_t length;
	int refcount;
	int locks;
};

ByteBuffer::ByteBuffer(size_t len, byte fill)
{
	m_data = new ByteBufferData;
	m_data->buffer = new byte[len?len:1];
	memset(m_data->buffer, fill, len);
	m_data->length = len;
	m_data->refcount = 1;
	m_data->locks = 0;
}

ByteBuffer::ByteBuffer(const ByteBuffer &buffer)
{
	wxASSERT(!buffer.m_data->locks);
	m_data = buffer.m_data;
	m_data->refcount++;
}

ByteBuffer::ByteBuffer(ByteBufferData *data)
{
	wxASSERT(!data->locks);
	m_data = data;
	m_data->refcount++;
}

ByteBuffer::ByteBuffer(const byte *buffer, size_t len)
{
	wxASSERT(len >= 0);
	m_data = new ByteBufferData;
	wxASSERT(m_data);
	m_data->buffer = new byte[len?len:1];
	wxASSERT(m_data->buffer);
	memcpy(m_data->buffer, buffer, len);
	m_data->length = len;
	m_data->refcount = 1;
	m_data->locks = 0;
}

ByteBuffer::ByteBuffer(const wxString &str)
{

	#if wxUSE_UNICODE
		const wchar_t *bytes_unicode = str.wc_str();
	#else
		wxWCharBuffer buff_unicode = wxConvCurrent->cMB2WC(str.c_str());
		const wchar_t *bytes_unicode = buff_unicode.data();
		if (!bytes_unicode)
		{
			buff_unicode = wxConvLocal.cMB2WC(str.c_str());
			bytes_unicode = buff_unicode.data();
		}
		if (!bytes_unicode)
		{
			buff_unicode = wxConvLibc.cMB2WC(str.c_str());
			bytes_unicode = buff_unicode.data();
		}
		if (!bytes_unicode)
		{
			buff_unicode = wxConvFile.cMB2WC(str.c_str());
			bytes_unicode = buff_unicode.data();
		}
		if (!bytes_unicode)
		{
			buff_unicode = wxWCharBuffer(str.Length()+1);
			wchar_t *buff = buff_unicode.data();
			for (size_t i = 0; i < str.Length(); ++i)
			{
				buff[i] = (((unsigned char)str[i]) < 0x80) ? str[i] : '?';
			}
			buff[str.Length()] = 0;
			bytes_unicode = buff_unicode.data();
		}
	#endif

	size_t nLen = wxConvUTF8.WC2MB((char *) NULL, bytes_unicode, 0);
	wxASSERT(nLen != (size_t)-1);
	wxCharBuffer buf(nLen);
	wxConvUTF8.WC2MB((char *)(const char *) buf, bytes_unicode, nLen+1);

	m_data = new ByteBufferData;
	m_data->length = nLen;
	m_data->buffer = new byte[m_data->length];
	memcpy(m_data->buffer, buf.data(), m_data->length);
	m_data->refcount = 1;
	m_data->locks = 0;
}

ByteBuffer::~ByteBuffer()
{
	wxASSERT(!m_data->locks);
	Release();
}

void ByteBuffer::Release()
{
	if (m_data)
	{
		wxASSERT(!m_data->locks);
		m_data->refcount--;
		if (m_data->refcount < 1)
		{
			delete m_data->buffer;
			delete m_data;
		}
		m_data = NULL;
	}
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer &other)
{
	wxASSERT(!m_data->locks);
	wxASSERT(!other.m_data->locks);
	if (m_data != other.m_data)
	{
		Release();
		m_data = other.m_data;
		m_data->refcount++;
	}
	return *this;
}

ByteBuffer ByteBuffer::operator+(const ByteBuffer &other) const
{
	wxASSERT(!m_data->locks);
	wxASSERT(!other.m_data->locks);
	if (m_data->length == 0)
	{
		return ByteBuffer(other);
	}
	else if (other.m_data->length == 0)
	{
		return ByteBuffer(m_data);
	}
	else
	{
		ByteBufferData *data = new ByteBufferData;
		data->length = m_data->length + other.m_data->length;
		data->buffer = new byte[data->length];
		memcpy(data->buffer, m_data->buffer, m_data->length);
		memcpy(data->buffer + m_data->length, other.m_data->buffer, other.m_data->length);
		data->refcount = 0;
		data->locks = 0;
		return ByteBuffer(data);
	}
}

bool ByteBuffer::operator==(const ByteBuffer &other) const
{
	wxASSERT(!m_data->locks);
	wxASSERT(!other.m_data->locks);
	return
		(m_data == other.m_data) ||
		((m_data->length == other.m_data->length) &&
		(memcmp(m_data->buffer, other.m_data->buffer, m_data->length) == 0));
}

const byte* ByteBuffer::LockRead() const
{
	m_data->locks++;
	return m_data->buffer;
}

byte* ByteBuffer::LockReadWrite()
{
	EnsureOwnCopy();
	m_data->locks++;
	return m_data->buffer;
}

void ByteBuffer::EnsureOwnCopy()
{
	if (m_data->refcount > 1)
	{
		wxASSERT(!m_data->locks);
		ByteBufferData *data = new ByteBufferData;
		data->buffer = new byte[m_data->length];
		memcpy(data->buffer, m_data->buffer, m_data->length);
		data->length = m_data->length;
		data->locks = 0;
		data->refcount = 1;
		m_data->refcount--;
		m_data = data;
	}
}

void ByteBuffer::Unlock() const
{
	wxASSERT(m_data->locks);
	m_data->locks--;
}

size_t ByteBuffer::Length() const
{
	return m_data->length;
}

ByteBuffer::operator wxString() const
{

	wxASSERT(!m_data->locks);

	// make a null terminated copy of the data 
	char *utf8 = new char[m_data->length + 1];
	memcpy(utf8, m_data->buffer, m_data->length);
	utf8[m_data->length] = 0;

	// unicode length will always be less than utf8 length * sizeof wchar_t
	int max_len = m_data->length * sizeof (wchar_t);
	wchar_t *unicode = new wchar_t[max_len];
	memset(unicode, 0, max_len * sizeof (wchar_t));

	size_t actual_len = wxConvUTF8.MB2WC(unicode, utf8, m_data->length);

	actual_len = wxMin(actual_len, wxSTRING_MAXLEN);

	#if wxUSE_UNICODE
		wxString result(unicode, *wxConvCurrent, actual_len);
	#else
		wxString result;
		if (actual_len > 0)
		{
			char *buff = new char[actual_len*2];
			size_t buff_len = wxConvLocal.WC2MB(buff, unicode, actual_len);
			if (buff_len == (size_t)-1)
			{
				buff_len = actual_len;
				for (size_t i = 0; i < buff_len; ++i)
				{
					buff[i] = (unicode[i] > 0xff) ? wxT('?') : (wxChar)unicode[i];
				}
			}
			result = wxString(buff, buff_len);
			delete buff;
		}
	#endif

	delete unicode;
	delete utf8;

	return result;

}

wxString ByteBuffer::GetHexDump(bool uppercase, bool space) const
{

	wxASSERT(!m_data->locks);

	wxString hex;
	hex.Alloc(Length() * 3);

	for (size_t i = 0; i < m_data->length; ++i)
	{
		if (space && i > 0)
		{
			hex += wxT(' ');
		}
		if (uppercase)
		{
			hex += wxString::Format(wxT("%02x"), m_data->buffer[i]).Upper();
		}
		else
		{
			hex += wxString::Format(wxT("%02x"), m_data->buffer[i]).Lower();
		}
	}

	return hex;

}
int ByteBuffer::Find(const ByteBuffer &to_find) const
{
	const byte *ptr1 = LockRead();
	const byte *ptr2 = to_find.LockRead();
	const byte *pos = findbytes(ptr1, Length(), ptr2, to_find.Length());
	int result = pos ? (pos-ptr1) : -1;
	Unlock();
	to_find.Unlock();
	return result;
}

ByteBuffer ByteBuffer::Left(int len) const
{
	len = wxMin(wxMax(len, 0), (int)Length());
	const byte *ptr = LockRead();
	ByteBuffer result(ptr, len);
	Unlock();
	return result;
}

ByteBuffer ByteBuffer::Right(int len) const
{
	len = wxMin(wxMax(len, 0), (int)Length());
	const byte *ptr = LockRead();
	ByteBuffer result(ptr + Length() - len, len);
	Unlock();
	return result;
}

ByteBuffer ByteBuffer::Mid(int pos, int len) const
{
	if (pos < 0)
	{
		len += pos;
	}
	if (len < 0)
	{
		len = Length();
	}
	pos = wxMin(wxMax(pos, 0), (int)Length());
	len = wxMin(wxMax(len, 0), (int)Length() - pos);
	const byte *ptr = LockRead();
	ByteBuffer result(ptr + pos, len);
	Unlock();
	return result;
}

byte ByteBuffer::operator[](size_t pos) const
{
	wxCHECK_MSG(pos >= 0 && pos < Length(), 0, wxT("Invalid index position"));
	const byte *ptr = LockRead();
	byte result = ptr[pos];
	Unlock();
	return result;
}
