#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ByteBuffer.cpp,v 1.10 2003-03-25 08:17:46 jason Exp $)

#include "ByteBuffer.h"

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
		const wxWCharBuffer &buff_unicode = wxConvCurrent->cMB2WC(str.c_str());
		const wchar_t *bytes_unicode = buff_unicode.data();
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
		char *buff = new char[actual_len*2];
		size_t buff_len = wxConvLocal.WC2MB(buff, unicode, actual_len*2);
		if (buff_len == (size_t)-1)
		{
			buff_len = actual_len;
			for (size_t i = 0; i < actual_len; ++i)
			{
				buff[i] = (unicode[i] > 0xff) ? wxT('?') : (wxChar)unicode[i];
			}
		}
		wxString result(buff, buff_len);
		delete buff;
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
