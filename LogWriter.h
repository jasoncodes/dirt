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
