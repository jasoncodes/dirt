#ifndef ClientUIConsole_H_
#define ClientUIConsole_H_

#include "Client.h"

class ReadThread;

class ClientUIConsole : public wxEvtHandler, public ClientEventHandler
{

public:

	ClientUIConsole();
	virtual ~ClientUIConsole();

protected:

	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);
	virtual void OnClientTransferNew(const FileTransfer &transfer);
	virtual void OnClientTransferDelete(const FileTransfer &transfer);
	virtual void OnClientTransferState(const FileTransfer &transfer);
	virtual void OnClientTransferTimer(const FileTransfer &transfer);

protected:
	Client *m_client;
	ReadThread *m_read_thread;

protected:
	void OnTextEnter(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()

};

#endif
