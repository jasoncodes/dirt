#ifndef _ClientUIConsole_H
#define _ClientUIConsole_H

#include "Client.h"

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

protected:
	Client *m_client;
	wxThread *m_read_thread;

protected:
	void OnTextEnter(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()

};

#endif
