#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogReader.cpp,v 1.5 2003-03-18 13:37:06 jason Exp $)

#include "LogReader.h"
#include <wx/filename.h>

LogReader::LogReader(const wxString &filename)
{
	if (m_file.Open(filename))
	{
		m_first_pass = true;
		Reset();
		// read public key + properties here // not implemented yet
	}
	m_first_pass = false;
	Reset();
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

bool LogReader::IsEof() const
{
	wxASSERT(Ok());
	return m_file.Eof();
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

LogEntryType LogReader::GetNext(); // not implemented yet

wxString LogReader::GetText(); // not implemented yet

ByteBuffer LogReader::Read(); // not implemented yet

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_public_key;
//	ByteBuffer m_private_key;
//	ByteBufferHashMap m_properties;
//	LogEntryType m_entry_type;
//	ByteBuffer m_entry;
//	bool m_first_pass;
