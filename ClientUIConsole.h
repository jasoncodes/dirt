#ifndef ClientUIConsole_H_
#define ClientUIConsole_H_

#include "Client.h"
#include "Console.h"

class ClientUIConsole : public Console, public ClientEventHandler
{

public:
	ClientUIConsole(bool no_input = false);
	virtual ~ClientUIConsole();

	Client* GetClient() { return m_client; }

	virtual void Output(const wxString &line);

protected:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientError(const wxString &context, const wxString &type, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientStateChange();
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick);
	virtual void OnClientTransferNew(const FileTransfer &transfer);
	virtual void OnClientTransferDelete(const FileTransfer &transfer);
	virtual void OnClientTransferState(const FileTransfer &transfer);
	virtual void OnClientTransferTimer(const FileTransfer &transfer);

protected:
	virtual void OnInput(const wxString &line);
	virtual void OnEOF();

protected:
	Client *m_client;

};

#endif
