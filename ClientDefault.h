#ifndef ClientDefault_H_
#define ClientDefault_H_

#include "Client.h"
class CryptSocketClient;
class CryptSocketEvent;

class ClientDefault : public Client
{

public:
	ClientDefault(ClientEventHandler *event_handler);
	virtual ~ClientDefault();

	virtual void SendMessage(const wxString &context, const wxString &nick, const wxString &message, bool is_action);
	virtual void SetNickname(const wxString &context, const wxString &nickname);
	virtual bool Connect(const URL &url);
	virtual void Disconnect();
	virtual bool IsConnected();
	virtual const URL& GetLastURL();
	virtual void Authenticate(const ByteBuffer &auth);
	virtual void Oper(const wxString &context, const wxString &pass);

protected:
	virtual void SendToServer(const ByteBuffer &msg);
	virtual bool ProcessServerInputExtra(bool preprocess, const wxString &context, const wxString &cmd, const ByteBuffer &data);

protected:
	void OnSocket(CryptSocketEvent &event);
	
protected:
	CryptSocketClient *m_sck;
	ByteBuffer m_authkey;
	URL m_url;

private:
	DECLARE_EVENT_TABLE()

};

#endif
