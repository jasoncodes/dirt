#ifndef ClientDefault_H_
#define ClientDefault_H_

#include "Client.h"
#include "URL.h"

class CryptSocketClient;
class CryptSocketEvent;
class DNS;
class DNSEvent;

class ClientDefault : public Client
{

public:
	ClientDefault(ClientEventHandler *event_handler);
	virtual ~ClientDefault();

	virtual void SendMessage(const wxString &context, const wxString &nick, const wxString &message, bool is_action);
	virtual void SetNickname(const wxString &context, const wxString &nickname);
	virtual bool Connect(const URL &url, bool is_reconnect);
	virtual void Disconnect(const wxString &msg);
	virtual bool IsConnected() const;
	virtual const URL& GetLastURL() const;
	virtual wxString GetLastURLString() const;
	virtual wxString GetLastHostname() const;
	virtual void Authenticate(const ByteBuffer &auth);
	virtual void Oper(const wxString &context, const wxString &pass);
	virtual ByteBuffer GetKeyLocalPublic() const;
	virtual ByteBuffer GetKeyLocalPrivate() const;
	virtual void NewProxySettings();
	virtual const CryptSocketProxySettings GetProxySettings() const;

protected:
	virtual void SendToServer(const ByteBuffer &msg);
	virtual bool ProcessServerInputExtra(bool preprocess, const wxString &context, const wxString &cmd, const ByteBuffer &data);

protected:
	void OnSocket(CryptSocketEvent &event);
	void OnDNS(DNSEvent &event);
	
protected:
	CryptSocketClient *m_sck;
	DNS *m_dns;
	ByteBuffer m_authkey;
	URL m_url;
	wxString m_last_server_hostname;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientDefault)

};

#endif
