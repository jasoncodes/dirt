#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogWriter.cpp,v 1.4 2003-03-18 11:57:34 jason Exp $)

#include "LogWriter.h"
#include <wx/confbase.h>
#include <wx/filename.h>

LogWriter::LogWriter(const wxString &filename); // not implemented yet

LogWriter::~LogWriter()
{
}

wxString LogWriter::GenerateFilename(const wxString &prefix, const wxDateTime &date, const wxString &suffix)
{
	wxString filename;
	filename
		<< prefix
		<< (prefix.Length() ? wxT(" ") : wxT(""))
		<< date.Format(wxT("%Y%m%d%H%M%S"))
		<< (suffix.Length() ? wxT(" ") : wxT(""))
		<< suffix;
	wxFileName fn(wxGetHomeDir(), filename);
	fn.SetExt(wxT("dirtlog"));
	fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
	return fn.GetFullPath();
}

bool LogWriter::Ok() const; // not implemented yet

void LogWriter::SetPublicKey(const ByteBuffer &public_key)
{
	m_public_key = public_key;
	Write(Uint16ToBytes(letPublicKey) + public_key);
}

ByteBufferHashMap LogWriter::GetProperties() const
{
	return m_properties;
}

ByteBuffer LogWriter::GetProperty(const wxString &name) const
{
	ByteBufferHashMap::const_iterator i = m_properties.find(name);
	return (i != m_properties.end()) ? i->second : ByteBuffer();
}

void LogWriter::SetProperty(const wxString &name, const ByteBuffer &value)
{
	if (GetProperty(name) != value)
	{
		Write(Uint16ToBytes(letProperty) + Uint16ToBytes(name.Length()) + ByteBuffer(name) + value);
		m_properties[name] = value;
	}
}

void LogWriter::AddText(const wxString &line, const wxColour &line_colour, bool convert_urls)
{
	ByteBuffer data(4);
	byte *ptr = data.LockReadWrite();
	ptr[0] = line_colour.Red();
	ptr[1] = line_colour.Green();
	ptr[2] = line_colour.Blue();
	ptr[3] = (convert_urls ? 1 : 0);
	data.Unlock();
	Write(Uint16ToBytes(letText) + data + ByteBuffer(line));
}

void LogWriter::AddSeparator()
{
	Write(Uint16ToBytes(letSeparator));
}

void LogWriter::Write(const ByteBuffer &data); // not implemented yet

//	wxFile m_file;
//	Crypt m_crypt;
//	ByteBuffer m_public_key;
//	ByteBufferHashMap m_properties;
