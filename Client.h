#ifndef Client_H_
#define Client_H_

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params) { return false; }
	virtual void OnClientDebug(const wxString &context, const wxString &text) = 0;
	virtual void OnClientWarning(const wxString &context, const wxString &text) = 0;
	virtual void OnClientInformation(const wxString &context, const wxString &text) = 0;
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text) = 0;
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private) = 0;
	virtual void OnClientUserList(const wxArrayString &nicklist) = 0;
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details) = 0;
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message) = 0;

};

class Client : public wxEvtHandler
{

public:
	Client(ClientEventHandler *event_handler);
	virtual ~Client();

	virtual void ProcessInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);

	virtual void SendMessage(const wxString &nick, const wxString &message) = 0;

	virtual wxString GetNickname() = 0;

protected:
	ClientEventHandler *m_event_handler;

private:
	DECLARE_EVENT_TABLE()

};

#endif
