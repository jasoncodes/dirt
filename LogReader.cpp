#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogReader.cpp,v 1.2 2003-03-18 10:25:17 jason Exp $)

#include "LogReader.h"

LogReader::LogReader(const wxString &filename);
LogReader::~LogReader();
bool LogReader::Ok() const;
ByteBuffer LogReader::GetPublicKey();
void LogReader::SetPrivateKey(const wxString &private_key);
ByteBufferHashMap LogReader::GetProperties() const;
ByteBuffer LogReader::GetProperty(const wxString &name) const;
bool LogReader::IsEof() const;
void LogReader::Reset();
off_t LogReader::GetPosition() const;
off_t LogReader::GetLength() const;
LogEntryType LogReader::GetNext();
wxString LogReader::GetText();
ByteBuffer LogReader::Read();

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_private_key;
//	ByteBufferHashMap m_properties;
//	ByteBuffer m_entry;
//	bool m_first_pass;
