#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogWriter.cpp,v 1.2 2003-03-18 10:25:17 jason Exp $)

#include "LogWriter.h"

LogWriter::LogWriter(const wxString &filename);
LogWriter::~LogWriter();
wxString LogWriter::GenerateFilename(const wxDateTime &date, const wxString &name);
bool LogWriter::Ok() const;
void LogWriter::SetPublicKey(const ByteBuffer &public_key);
ByteBufferHashMap LogWriter::GetProperties() const;
ByteBuffer LogWriter::GetProperty(const wxString &name) const;
void LogWriter::SetProperty(const wxString &name, const ByteBuffer &value);
void LogWriter::AddText(const wxString &line, const wxColour &line_colour, bool convert_urls);
void LogWriter::AddSeparator();
void LogWriter::Write(const ByteBuffer &data);

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_public_key;
//	ByteBufferHashMap m_properties;
