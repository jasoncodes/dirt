#ifndef _ClientUIMDIFrame_H
#define _ClientUIMDIFrame_H

#include "SwitchBarParent.h"
#include "Client.h"

class ClientUIMDICanvas;

class ClientUIMDIFrame : public SwitchBarParent, public ClientEventHandler
{

public:

	ClientUIMDIFrame();
	virtual ~ClientUIMDIFrame();

	Client* GetClient() { return m_client; }

	bool IsFocused();

protected:
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnFocusTimer(wxTimerEvent &event);
	void OnActivate(wxActivateEvent &event);

	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);

protected:
	wxTimer *tmrFocus;
	Client *m_client;
	bool m_focused;
	wxListBox *m_lstNickList;

	ClientUIMDICanvas* GetContext(const wxString &context, bool create_if_not_exist = true);
	void AddLine(const wxString &context, const wxString &line, const wxColour &line_colour = *wxBLACK, bool create_if_not_exist = true, bool suppress_alert = false);

private:
	DECLARE_EVENT_TABLE()

};

#endif
