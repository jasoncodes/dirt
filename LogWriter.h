#ifndef LogWriter_H_
#define LogWriter_H_

#include "Log.h"
#include "ByteBuffer.h"
#include "Crypt.h"
#include "Util.h"
#include <wx/datetime.h>

class LogWriter
{

public:
	LogWriter(const wxString &filename);
	LogWriter(const wxDateTime &date = wxDateTime::Now(), const wxString &name = wxEmptyString);
	virtual ~LogWriter();

	static wxString GenerateFilename(const wxDateTime &date = wxDateTime::Now(), const wxString &name = wxEmptyString);

	virtual bool Ok() const;

	virtual void SetPublicKey(const ByteBuffer &public_key);

	virtual ByteBufferHashMap GetProperties() const;
	virtual ByteBuffer GetProperty(const wxString &name) const;
	virtual void SetProperty(const wxString &name, const ByteBuffer &value);

	virtual void AddText(const wxString &line, const wxColour &line_colour = *wxBLACK, bool convert_urls = true);
	virtual void AddSeparator();

protected:
	virtual void Init(const wxString &filename);
	virtual void Write(const ByteBuffer &data);

protected:
	wxFile m_file;
	Crypt m_crypt;
	ByteBuffer m_public_key;
	ByteBufferHashMap m_properties;

};

#endif
