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


#ifndef LogReader_H_
#define LogReader_H_

#include <wx/file.h>
#include <wx/datetime.h>

#include "Log.h"
#include "Crypt.h"
#include "util.h"

class LogReader
{

public:
	LogReader(const wxString &filename);
	virtual ~LogReader();

	static bool ParseFilename(const wxString &filename, wxString &prefix, wxDateTime &date, wxString &suffix);

	virtual bool Ok() const;

	virtual ByteBuffer GetPublicKey();
	virtual bool SetPrivateKey(const wxString &private_key);

	virtual ByteBufferHashMap GetProperties() const;
	virtual ByteBuffer GetProperty(const wxString &name) const;

	virtual bool Eof() const;
	virtual void Reset();
	virtual off_t GetPosition() const;
	virtual off_t GetLength() const;

	inline bool HasNext() const { return !Eof(); }
	virtual LogEntryType GetNext();

	virtual wxString GetText();
	virtual wxColour GetTextColour();
	virtual bool GetTextConvertURLs();

protected:
	virtual ByteBuffer Read();
	virtual ByteBuffer GetTextHelper();
	virtual void ParsePropertyEntry(const ByteBuffer &data);

protected:
	wxFile m_file;
	Crypt m_crypt;
	ByteBuffer m_public_key;
	ByteBuffer m_private_key;
	ByteBufferHashMap m_properties;
	LogEntryType m_entry_type;
	ByteBuffer m_entry;
	bool m_first_pass;

private:
	DECLARE_NO_COPY_CLASS(LogReader)

};

#endif
