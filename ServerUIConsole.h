#ifndef ServerUIConsole_H_
#define ServerUIConsole_H_

#include "Server.h"
#include "Console.h"

class ServerUIConsole : public Console, public ServerEventHandler
{

public:
	ServerUIConsole();
	virtual ~ServerUIConsole();

	virtual void Output(const wxString &line);

protected:
	virtual void OnServerLog(const wxString &line);

protected:
	virtual void OnInput(const wxString &line);
	virtual void OnEOF();

protected:
	Server *m_server;

};

#endif
