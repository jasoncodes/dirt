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
