#ifndef ClientDefault_H_
#define ClientDefault_H_

#include "Client.h"

class ClientDefault : public Client
{

public:
	ClientDefault(ClientEventHandler *event_handler);
	virtual ~ClientDefault();

	virtual void SendMessage(const wxString &nick, const wxString &message);
	virtual wxString GetNickname();
	virtual void Connect(const wxString &url);

protected:
	void OnTestTimer(wxTimerEvent &event);
	
protected:
	wxTimer *tmrTest;

private:
	DECLARE_EVENT_TABLE()

};

#endif
