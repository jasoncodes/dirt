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

	virtual void SendMessage(const wxString &nick, const wxString &message);
	virtual wxString GetNickname();
	virtual bool Connect(const URL &url);
	virtual void Disconnect();
	virtual bool IsConnected();

protected:
	void OnSocket(CryptSocketEvent &event);
	
protected:
	CryptSocketClient *m_sck;

private:
	DECLARE_EVENT_TABLE()

};

#endif
