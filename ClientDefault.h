#ifndef _CLIENTDEFAULT_H
#define _CLIENTDEFAULT_H

#include "Client.h"

class ClientDefault : public wxEvtHandler, public Client
{

public:
	ClientDefault(ClientEventHandler *event_handler);
	virtual ~ClientDefault();

	virtual void SendMessage(const wxString &nick, const wxString &message);

};

#endif
