#ifndef Server_H_
#define Server_H_

class ServerEventHandler
{

public:
	virtual void OnServerLog(const wxString &line) = 0;

};

class Server : public wxEvtHandler
{

public:
	Server(ServerEventHandler *event_handler);
	virtual ~Server();

	virtual void ProcessInput(const wxString &input);

protected:
	ServerEventHandler *m_event_handler;

private:
	DECLARE_EVENT_TABLE()

};

#endif
