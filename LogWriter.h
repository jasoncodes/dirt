/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef LogWriter_H_
#define LogWriter_H_

#include <wx/file.h>
#include <wx/datetime.h>

class ByteBuffer;

#include "Crypt.h"
#include "util.h"

class LogWriter
{

public:
	LogWriter(const wxString &filename);
	virtual ~LogWriter();

	static wxString GenerateFilename(const wxString &dir, const wxString &prefix = wxEmptyString, const wxDateTime &date = wxDateTime::Now(), const wxString &suffix = wxEmptyString);
	static wxDateTime GenerateNewLogDate(const wxString &dir, const wxString &prefix);

	virtual bool Ok() const;

	virtual void SetPublicKey(const ByteBuffer &public_key);

	virtual ByteBufferHashMap GetProperties() const;
	virtual ByteBuffer GetProperty(const wxString &name) const;
	virtual void SetProperty(const wxString &name, const ByteBuffer &value);

	virtual void AddText(const wxString &line, const wxColour &line_colour = *wxBLACK, bool convert_urls = true);
	virtual void AddSeparator();

protected:
	virtual void Write(const ByteBuffer &data);

protected:
	wxFile m_file;
	Crypt m_crypt;
	ByteBuffer m_public_key;
	ByteBufferHashMap m_properties;

private:
	DECLARE_NO_COPY_CLASS(LogWriter)

};

#endif
