#ifndef ServerUIConsole_H_
#define ServerUIConsole_H_

#include "Server.h"
#include "Console.h"

class ServerUIConsole : public Console, public ServerEventHandler
{

public:
	ServerUIConsole(bool no_input = false, bool quit_on_stop  = false);
	virtual ~ServerUIConsole();

	virtual void Output(const wxString &line);

protected:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params);
	virtual wxArrayString OnServerSupportedCommands();
	virtual void OnServerStateChange();
	virtual void OnServerConnectionChange();
	virtual void OnServerInformation(const wxString &line);
	virtual void OnServerWarning(const wxString &line);

protected:
	virtual void OnInput(const wxString &line);
	virtual void OnEOF();

protected:
	Server *m_server;
	bool m_quit_on_stop;

};

#endif
