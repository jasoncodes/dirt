#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogWriter.cpp,v 1.3 2003-03-18 11:15:12 jason Exp $)

#include "LogWriter.h"
#include <wx/confbase.h>
#include <wx/filename.h>

LogWriter::LogWriter(const wxString &filename); // not implemented yet

LogWriter::~LogWriter(); // not implemented yet

wxString LogWriter::GenerateFilename(const wxString &prefix, const wxDateTime &date, const wxString &suffix)
{
	wxString filename;
	filename
		<< prefix
		<< (prefix.Length()?wxT(" "):wxT(""))
		<< date.Format(wxT("%Y%m%d%H%M%S"))
		<< (suffix.Length()?wxT(" "):wxT(""))
		<< suffix;
	wxFileName fn(wxGetHomeDir(), filename);
	fn.SetExt(wxT("dirtlog"));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetFullPath();
}

bool LogWriter::Ok() const; // not implemented yet

void LogWriter::SetPublicKey(const ByteBuffer &public_key); // not implemented yet

ByteBufferHashMap LogWriter::GetProperties() const; // not implemented yet

ByteBuffer LogWriter::GetProperty(const wxString &name) const; // not implemented yet

void LogWriter::SetProperty(const wxString &name, const ByteBuffer &value); // not implemented yet

void LogWriter::AddText(const wxString &line, const wxColour &line_colour, bool convert_urls); // not implemented yet

void LogWriter::AddSeparator(); // not implemented yet

void LogWriter::Write(const ByteBuffer &data); // not implemented yet

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_public_key;
//	ByteBufferHashMap m_properties;
