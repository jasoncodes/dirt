#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogWriter.cpp,v 1.12 2003-05-14 07:53:15 jason Exp $)

#include "LogWriter.h"
#include <wx/confbase.h>
#include <wx/filename.h>
#include "Log.h"

LogWriter::LogWriter(const wxString &filename)
{
	wxLogNull supress_log;
	if (filename.Length())
	{
		wxFileName fn(filename);
		wxString dir = fn.GetPath(wxPATH_GET_VOLUME);
		wxFileName::Mkdir(dir, 0700, wxPATH_MKDIR_FULL);
		m_file.Open(filename, wxFile::write_append);
	}
}

LogWriter::~LogWriter()
{
	m_file.Close();
}

wxString LogWriter::GenerateFilename(const wxString &dir, const wxString &prefix, const wxDateTime &date, const wxString &suffix)
{
	if (dir.Length())
	{
		wxString filename;
		filename
			<< prefix
			<< (prefix.Length() ? wxT(" ") : wxT(""))
			<< date.Format(wxT("%Y%m%d%H%M%S"))
			<< (suffix.Length() ? wxT(" ") : wxT(""))
			<< suffix;
		wxFileName fn(dir, filename);
		fn.SetExt(wxT("dirtlog"));
		return fn.GetFullPath();
	}
	return wxEmptyString;
}

bool LogWriter::Ok() const
{
	return m_file.IsOpened();
}

void LogWriter::SetPublicKey(const ByteBuffer &public_key)
{
	m_public_key = public_key;
	Write(Uint16ToBytes(letPublicKey) + m_public_key);
	ByteBuffer block_key = Crypt::Random(32);
	m_crypt.SetAESEncryptKey(block_key);
	Write(Uint16ToBytes(letBlockKey) + m_crypt.RSAEncrypt(public_key, block_key));
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
		ByteBuffer name_buff(name);
		ByteBuffer value_buff(value);
		ByteBuffer data = Uint16ToBytes(name_buff.Length()) + name_buff + Uint16ToBytes(value_buff.Length()) + value_buff;
		if (m_public_key.Length())
		{
			data = m_crypt.AESEncrypt(data);
		}
		Write(Uint16ToBytes(letProperty) + data);
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
	ByteBuffer line_buff(line);
	data = Uint16ToBytes(data.Length()) + data + Uint16ToBytes(line_buff.Length()) + line_buff;
	if (m_public_key.Length())
	{
		data = m_crypt.AESEncrypt(data);
	}
	Write(Uint16ToBytes(letText) + data);
}

void LogWriter::AddSeparator()
{
	Write(Uint16ToBytes(letSeparator));
}

void LogWriter::Write(const ByteBuffer &data)
{
	wxASSERT(Ok());
	ByteBuffer tmp = Uint16ToBytes(data.Length()) + data;
	m_file.SeekEnd();
	m_file.Write(tmp.LockRead(), tmp.Length());
	m_file.Flush();
	tmp.Unlock();
}

wxDateTime LogWriter::GenerateNewLogDate(const wxString &dir, const wxString &prefix)
{
	wxDateTime date = wxDateTime::Now();
	wxFileName fn(GenerateFilename(dir, prefix, date));
	while (fn.FileExists())
	{
		date += wxTimeSpan::Seconds(1);
		fn = wxFileName(GenerateFilename(dir, prefix, date));
	}
	return date;
}
