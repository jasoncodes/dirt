#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogReader.cpp,v 1.11 2003-03-19 10:45:16 jason Exp $)

#include "LogReader.h"
#include <wx/filename.h>

LogReader::LogReader(const wxString &filename)
{
	if (m_file.Open(filename))
	{
		m_first_pass = true;
		Reset();
		while (!Eof())
		{
			LogEntryType let = GetNext();
			wxASSERT(let >= letInvalid);
			switch (let)
			{
				case letPublicKey:
					if (m_public_key.Length() > 0 && m_public_key != m_entry)
					{
						m_file.Close();
						return;
					}
					m_public_key = m_entry;
					break;
				case letProperty:
					if (m_public_key.Length() == 0)
					{
						ParsePropertyEntry(m_entry);
					}
					break;
			}
		}
		m_first_pass = false;
		Reset();
	}
}

LogReader::~LogReader()
{
	m_file.Close();
}

wxString LogReader::GetDefaultLogDirectory()
{
	wxFileName fn(wxGetHomeDir(), wxT(""));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

#include <wx/dynarray.h>
#include <wx/arrimpl.cpp>
WX_DECLARE_OBJARRAY(bool, BoolArray);
WX_DEFINE_OBJARRAY(BoolArray);
WX_DECLARE_OBJARRAY(size_t, SizeTArray);
WX_DEFINE_OBJARRAY(SizeTArray);

bool LogReader::ParseFilename(const wxString &filename, wxString &prefix, wxDateTime &date, wxString &suffix)
{
	wxFileName fn(filename);
	wxString name = fn.GetName();
	BoolArray is_digit;
	is_digit.Alloc(name.Length());
	is_digit.Add(false, name.Length());
	for (size_t i = 0; i < name.Length(); ++i)
	{
		is_digit[i] = wxIsdigit(name[i]) != 0;
	}
	SizeTArray date_positions;
	size_t date_len = 14;
	for (size_t i = 0; i < name.Length(); ++i)
	{
		if (is_digit[i])
		{
			size_t count = 0;
			for (size_t j = i; j < i + date_len && j < name.Length(); ++j)
			{
				if (is_digit[j])
				{
					count++;
				}
			}
			if (count == date_len)
			{
				date_positions.Add(i);
			}
		}
	}
	if (date_positions.GetCount() == 1)
	{
		size_t pos = date_positions[0];
		prefix = name.Left(pos).Trim(false).Trim(true);
		wxString date_str = name.Mid(pos, date_len);
		suffix = name.Mid(pos+date_len).Trim(false).Trim(true);
		const wxChar *start = date_str.c_str();
		const wxChar *end = date.ParseFormat(start, wxT("%Y%m%d%H%M%S"));
		
		return (end != NULL) && ((size_t)(end-start) == date_len);
	}
	return false;
}

bool LogReader::Ok() const
{
	return m_file.IsOpened() && !m_file.Error();
}

ByteBuffer LogReader::GetPublicKey()
{
	return m_public_key;
}

bool LogReader::SetPrivateKey(const wxString &private_key)
{
	try
	{
		ByteBuffer src = Crypt::Random(32);
		ByteBuffer enc = Crypt::RSAEncrypt(m_public_key, src);
		ByteBuffer dec = Crypt::RSADecrypt(private_key, enc);
		bool success = (src == dec);
		if (success)
		{
			m_private_key = private_key;
			m_first_pass = true;
			off_t saved_pos = m_file.Tell();
			Reset();
			m_public_key = ByteBuffer();
			while (!Eof())
			{
				LogEntryType let = GetNext();
				wxASSERT(let >= letInvalid);
				switch (let)
				{
					case letPublicKey:
						m_public_key = m_entry;
						break;
					case letProperty:
						{
							ByteBuffer data = m_entry;
							if (m_public_key.Length())
							{
								data = m_crypt.AESDecrypt(data);
							}
							ParsePropertyEntry(data);
						}
						break;
				}
			}
			m_first_pass = false;
			m_file.Seek(saved_pos);
		}
		return success;
	}
	catch (...)
	{
		return false;
	}
}

ByteBufferHashMap LogReader::GetProperties() const
{
	return m_properties;
}

ByteBuffer LogReader::GetProperty(const wxString &name) const
{
	ByteBufferHashMap::const_iterator i = m_properties.find(name);
	return (i != m_properties.end()) ? i->second : ByteBuffer();
}

bool LogReader::Eof() const
{
	wxASSERT(Ok());
	return !Ok() || m_file.Eof();
}

void LogReader::Reset()
{
	wxASSERT(Ok());
	m_file.Seek(0);
}

off_t LogReader::GetPosition() const
{
	wxASSERT(Ok());
	return m_file.Tell();
}

off_t LogReader::GetLength() const
{
	wxASSERT(Ok());
	return m_file.Length();
}

LogEntryType LogReader::GetNext()
{
	do
	{
		ByteBuffer data = Read();
		if (data.Length() >= 2)
		{
			const byte *ptr = data.LockRead();
			m_entry_type = (LogEntryType)BytesToUint16(ptr, 2);
			m_entry = ByteBuffer(ptr+2, data.Length() - 2);
			data.Unlock();
			if (m_entry_type == letBlockKey)
			{
				if (m_public_key.Length() && m_private_key.Length())
				{
					try
					{
						data = Crypt::RSADecrypt(m_private_key, m_entry);
						m_crypt.SetAESDecryptKey(data);
					}
					catch (...)
					{
					}
				}
			}
		}
		else
		{
			m_entry_type = letInvalid;
			m_entry = ByteBuffer();
			break;
		}
	}
	while (m_entry_type == letBlockKey ||
		(!m_first_pass && m_entry_type != letText && m_entry_type != letSeparator));
	return m_entry_type;
}

ByteBuffer LogReader::GetTextHelper()
{
	wxASSERT(m_entry_type == letText);
	ByteBuffer data = m_entry;
	if (m_public_key.Length() && m_private_key.Length())
	{
		data = m_crypt.AESDecrypt(data);
	}
	if (data.Length() < 4)
	{
		wxFAIL_MSG(wxT("Text entry must be at least 4 bytes"));
		return ByteBuffer();
	}
	return data;
}

wxString LogReader::GetText()
{
	ByteBuffer buff = GetTextHelper();
	if (buff.Length() > 4)
	{
		const byte *ptr = buff.LockRead();
		size_t data_len = BytesToUint16(ptr, 2);
		if (data_len+4 > buff.Length())
		{
			buff.Unlock();
			return wxEmptyString;
		}
		size_t text_len = BytesToUint16(ptr + 2 + data_len, 2);
		if (data_len+text_len+4 > buff.Length())
		{
			buff.Unlock();
			return wxEmptyString;
		}
		ByteBuffer text(ptr + 2 + data_len + 2, text_len);
		buff.Unlock();
		return text;
	}
	return wxEmptyString;
}

wxColour LogReader::GetTextColour()
{
	ByteBuffer buff = GetTextHelper();
	if (buff.Length() > 4)
	{
		const byte *ptr = buff.LockRead();
		size_t data_len = BytesToUint16(ptr, 2);
		if (data_len+4 > buff.Length())
		{
			buff.Unlock();
			return wxColour();
		}
		ByteBuffer data(ptr + 2, data_len);
		buff.Unlock();
		if (data_len >= 3)
		{
			ptr = data.LockRead();
			wxColour colour(ptr[0], ptr[1], ptr[2]);
			data.Unlock();
			return colour;
		}
	}
	return wxColour();
}

bool LogReader::GetTextConvertURLs()
{
	ByteBuffer buff = GetTextHelper();
	if (buff.Length() > 4)
	{
		const byte *ptr = buff.LockRead();
		size_t data_len = BytesToUint16(ptr, 2);
		if (data_len+4 > buff.Length())
		{
			buff.Unlock();
			return false;
		}
		ByteBuffer data(ptr + 2, data_len);
		buff.Unlock();
		if (data_len >= 4)
		{
			ptr = data.LockRead();
			byte flags = ptr[3];
			data.Unlock();
			return (flags & 1) != 0;
		}
	}
	return false;
}

ByteBuffer LogReader::Read()
{
	wxASSERT(Ok() && !Eof());
	ByteBuffer buff(2);
	off_t bytes_read = m_file.Read(buff.LockReadWrite(), buff.Length());
	buff.Unlock();
	if (bytes_read != (off_t)buff.Length())
	{
		return ByteBuffer();
	}
	wxUint16 len = BytesToUint16(buff.LockRead(), buff.Length());
	buff.Unlock();
	buff = ByteBuffer(len);
	bytes_read = m_file.Read(buff.LockReadWrite(), buff.Length());
	buff.Unlock();
	if (bytes_read != (off_t)buff.Length())
	{
		return ByteBuffer();
	}
	return buff;
}

void LogReader::ParsePropertyEntry(const ByteBuffer &data)
{
	if (data.Length() > 4)
	{
		const byte *ptr = data.LockRead();
		size_t name_len = BytesToUint16(ptr, 2);
		if (name_len+4 > data.Length())
		{
			data.Unlock();
			return;
		}
		size_t value_len = BytesToUint16(ptr + 2 + name_len, 2);
		if (name_len+value_len+4 > data.Length())
		{
			data.Unlock();
			return;
		}
		ByteBuffer name = ByteBuffer(ptr+2, name_len);
		ByteBuffer value = ByteBuffer(ptr + 2 + name_len + 2, value_len);
		m_properties[name] = value;
		data.Unlock();
	}
}
