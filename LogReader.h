#ifndef LogReader_H_
#define LogReader_H_

#include "Log.h"
#include "ByteBuffer.h"
#include "Crypt.h"
#include "util.h"
#include <wx/file.h>

class LogReader
{

public:
	LogReader(const wxString &filename);
	virtual ~LogReader();

	static wxString GetDefaultLogDirectory();

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

};

#endif
