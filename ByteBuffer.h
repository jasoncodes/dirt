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


#ifndef ByteBuffer_H_
#define ByteBuffer_H_

typedef unsigned char byte;

// note: this class is not thread safe
// do not pass instances between multiple threads!

struct ByteBufferData;

class ByteBuffer
{

public:
	explicit ByteBuffer(size_t len = 0, byte fill = '\0');
	ByteBuffer(const ByteBuffer &buffer);
	ByteBuffer(const byte *buffer, size_t len);
	ByteBuffer(const wxString &str);

	~ByteBuffer();

	ByteBuffer& operator=(const ByteBuffer &other);
	ByteBuffer operator+(const ByteBuffer &other) const;
	bool operator==(const ByteBuffer &other) const;

	const byte* LockRead() const;
	byte* LockReadWrite();
	void Unlock() const;

	size_t Length() const;

	operator wxString() const;
	wxString GetHexDump(bool uppercase = true, bool space = true) const;

	int Find(const ByteBuffer &to_find) const;
	ByteBuffer Left(int len) const;
	ByteBuffer Right(int len) const;
	ByteBuffer Mid(int pos, int len = -1) const;

	byte operator[](size_t pos) const;

protected:
	ByteBuffer(ByteBufferData *data);
	void Release();
	void EnsureOwnCopy();

protected:
	ByteBufferData *m_data;

};

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(ByteBuffer, ByteBufferArray);

inline bool operator!=(const ByteBuffer &lhs, const ByteBuffer &rhs)
{
	return !(lhs == rhs);
}

inline ByteBuffer& operator+=(ByteBuffer &lhs, const ByteBuffer &rhs)
{
	lhs = lhs + rhs;
	return lhs;
}

#endif
