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


#ifndef File_H_
#define File_H_

#ifdef __WXMSW__
	#define USE_WIN32_FILE 1
#else
	#define USE_WIN32_FILE 0
#endif

#if USE_WIN32_FILE
	typedef void *HANDLE;
#else
	#include <wx/file.h>
#endif

typedef unsigned char byte;

class File
{

public:
	enum OpenMode { read, write, read_write, write_append, write_excl };

	File();
	~File();

	bool Create(const wxString &filename, bool overwrite = false);
	bool Open(const wxString &filename, OpenMode mode = read);
	bool Close();

	bool IsOpen();
	bool IsEof();

	off_t Read(const byte *ptr, const off_t len);
	off_t Write(const byte *ptr, const off_t len);

	bool Write(const wxString& s, wxMBConv& conv = wxConvUTF8)
	{
		const wxWX2MBbuf buf = s.mb_str(conv);
		off_t size = strlen(buf);
		return Write((const byte*)(const char*)buf, size) == size;
	}

	bool Flush();

	wxLongLong_t Seek(wxLongLong_t pos, wxSeekMode mode = wxFromStart);
	wxLongLong_t Tell();
	wxLongLong_t Length();

	static wxLongLong_t Length(const wxString &filename);

protected:
	#if USE_WIN32_FILE
		HANDLE m_hFile;
	#else
		wxFile m_file;
	#endif

private:
	DECLARE_NO_COPY_CLASS(File)

};

#endif
