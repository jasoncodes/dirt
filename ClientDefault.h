#ifndef _CLIENTDEFAULT_H
#define _CLIENTDEFAULT_H

#include "Client.h"

class ClientDefault : public Client
{

public:
	ClientDefault(ClientEventHandler *event_handler);
	virtual ~ClientDefault();

	virtual void SendMessage(const wxString &nick, const wxString &message);
	virtual wxString GetNickname();

protected:
	void OnTestTimer(wxTimerEvent &event);
	
protected:
	wxTimer *tmrTest;

private:
	DECLARE_EVENT_TABLE()

};

#endif
