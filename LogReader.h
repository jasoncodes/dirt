#ifndef LogReader_H_
#define LogReader_H_

#include "Log.h"
#include "ByteBuffer.h"
#include "Crypt.h"
#include "Util.h"

class LogReader
{

public:
	LogReader(const wxString &filename);
	virtual ~LogReader();

	virtual bool Ok() const;

	virtual ByteBuffer GetPublicKey();
	virtual void SetPrivateKey(const wxString &private_key);

	virtual ByteBufferHashMap GetProperties() const;
	virtual ByteBuffer GetProperty(const wxString &name) const;

	virtual bool IsEof() const;
	virtual void Reset();
	virtual off_t GetPosition() const;
	virtual off_t GetLength() const;

	inline bool HasNext() const { return !IsEof(); }
	virtual LogEntryType GetNext();

	virtual wxString GetText();

protected:
	virtual void Init(const wxString &filename);
	virtual ByteBuffer Read();

protected:
	wxFile m_file;
	Crypt m_crypt;
	ByteBuffer m_private_key;
	ByteBufferHashMap m_properties;
	ByteBuffer m_entry;
	bool m_first_pass;

};

#endif
