#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogReader.cpp,v 1.3 2003-03-18 11:15:12 jason Exp $)

#include "LogReader.h"
#include <wx/filename.h>

LogReader::LogReader(const wxString &filename); // not implemented yet

LogReader::~LogReader(); // not implemented yet

wxString LogReader::GetDefaultLogDirectory()
{
	wxFileName fn(wxGetHomeDir(), wxT(""));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
}

bool LogReader::Ok() const; // not implemented yet

ByteBuffer LogReader::GetPublicKey(); // not implemented yet

void LogReader::SetPrivateKey(const wxString &private_key); // not implemented yet

ByteBufferHashMap LogReader::GetProperties() const; // not implemented yet

ByteBuffer LogReader::GetProperty(const wxString &name) const; // not implemented yet

bool LogReader::IsEof() const; // not implemented yet

void LogReader::Reset(); // not implemented yet

off_t LogReader::GetPosition() const; // not implemented yet

off_t LogReader::GetLength() const; // not implemented yet

LogEntryType LogReader::GetNext(); // not implemented yet

wxString LogReader::GetText(); // not implemented yet

ByteBuffer LogReader::Read(); // not implemented yet

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_private_key;
//	ByteBufferHashMap m_properties;
//	ByteBuffer m_entry;
//	bool m_first_pass;
