#ifndef Server_H_
#define Server_H_

class ServerEventHandler
{

public:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params) { return false; }
	virtual void OnServerInformation(const wxString &line) = 0;
	virtual void OnServerWarning(const wxString &line) = 0;

};

class Server : public wxEvtHandler
{

public:
	Server(ServerEventHandler *event_handler);
	virtual ~Server();

	virtual void ProcessInput(const wxString &input);
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() = 0;

protected:
	ServerEventHandler *m_event_handler;

private:
	DECLARE_EVENT_TABLE()

};

#endif
