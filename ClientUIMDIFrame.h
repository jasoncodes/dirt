#ifndef _ClientUIMDIFrame_H
#define _ClientUIMDIFrame_H

#include "SwitchBarParent.h"
#include "Client.h"
#include "ClientUIMDICanvas.h"

class ClientUIMDIFrame : public SwitchBarParent, public ClientEventHandler
{

public:

	ClientUIMDIFrame();
	virtual ~ClientUIMDIFrame();

	Client* GetClient() { return m_client; }

protected:
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnFocusInputControlTimer(wxTimerEvent &event);

	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private);


protected:
	wxTimer *tmrFocusInputControl;
	Client *m_client;

	ClientUIMDICanvas* GetContext(const wxString &context, bool create_if_not_exist = true);
	void AddLine(const wxString &context, const wxString &line, const wxColour &line_colour = *wxBLACK, bool create_if_not_exist = true, bool suppress_alert = false);

private:
	DECLARE_EVENT_TABLE()

};

#endif
