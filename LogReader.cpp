#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogReader.cpp,v 1.4 2003-03-18 11:57:34 jason Exp $)

#include "LogReader.h"
#include <wx/filename.h>

LogReader::LogReader(const wxString &filename); // not implemented yet

LogReader::~LogReader()
{
}

wxString LogReader::GetDefaultLogDirectory()
{
	wxFileName fn(wxGetHomeDir(), wxT(""));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

bool LogReader::Ok() const; // not implemented yet

ByteBuffer LogReader::GetPublicKey(); // not implemented yet

bool LogReader::SetPrivateKey(const wxString &private_key); // not implemented yet

ByteBufferHashMap LogReader::GetProperties() const
{
	return m_properties;
}

ByteBuffer LogReader::GetProperty(const wxString &name) const
{
	ByteBufferHashMap::const_iterator i = m_properties.find(name);
	return (i != m_properties.end()) ? i->second : ByteBuffer();
}

bool LogReader::IsEof() const; // not implemented yet

void LogReader::Reset(); // not implemented yet

off_t LogReader::GetPosition() const; // not implemented yet

off_t LogReader::GetLength() const; // not implemented yet

LogEntryType LogReader::GetNext(); // not implemented yet

wxString LogReader::GetText(); // not implemented yet

ByteBuffer LogReader::Read(); // not implemented yet

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_public_key;
//	ByteBuffer m_private_key;
//	ByteBufferHashMap m_properties;
//	ByteBuffer m_entry;
//	bool m_first_pass;
