#ifndef _CLIENT_H
#define _CLIENT_H

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &context, const wxString &cmd, const wxString &params) { return false; }
	virtual void OnClientDebug(const wxString &context, const wxString &text) = 0;
	virtual void OnClientWarning(const wxString &context, const wxString &text) = 0;
	virtual void OnClientInformation(const wxString &context, const wxString &text) = 0;

};

class Client
{

public:
	Client(ClientEventHandler *event_handler);
	virtual ~Client();

	virtual void ProcessInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);

	virtual void SendMessage(const wxString &nick, const wxString &message) = 0;

protected:
	ClientEventHandler *m_event_handler;

};

#endif
