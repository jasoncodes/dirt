#ifndef URL_H_
#define URL_H_

class URL
{

public:
	URL();
	URL(const wxString &url);
	URL(const URL &url, const wxString &rel_url);
	virtual ~URL();

	operator wxString() const;

	static wxString Escape(const wxString &text);
	static wxString Unescape(const wxString &text);

	virtual wxString GetProtocol(const wxString &default_protocol = wxEmptyString) const;
	virtual wxString GetHostname() const;
	virtual int GetPort(int default_port = 0) const;
	virtual wxString GetAuthentication() const;
	virtual wxString GetUsername() const;
	virtual wxString GetPassword() const;
	virtual wxString GetPath() const;
	virtual wxString GetQuery() const;
	virtual wxString GetReference() const;

	virtual void SetProtocol(const wxString &protocol);
	virtual void SetHostname(const wxString &hostname);
	virtual void SetPort(int port);
	virtual void SetAuthentication(const wxString &authentication);
	virtual void SetUsername(const wxString &username);
	virtual void SetPassword(const wxString &password);
	virtual void SetPath(const wxString &path);
	virtual void SetPathWithQueryAndRef(const wxString &path_with_query_and_ref);
	virtual void SetQuery(const wxString &query);
	virtual void SetReference(const wxString &reference);

protected:
	wxString m_protocol; // all these fields are stored in a non-escaped format
	wxString m_hostname;
	int m_port;
	wxString m_username;
	wxString m_password;
	wxString m_path;
	wxString m_query;
	wxString m_reference;

};

#endif
