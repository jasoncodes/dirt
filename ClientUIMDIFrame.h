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
	void OnHelpAbout(wxCommandEvent& event);
	void OnFileNewWindow(wxCommandEvent& event);
	void OnFileNewWindowNoFocus(wxCommandEvent& event);
	void OnFileExit(wxCommandEvent& event);
	void OnFocusInputControlTimer(wxTimerEvent &event);

	virtual bool OnClientPreprocess(const wxString &context, const wxString &cmd, const wxString &params);
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);

protected:
	wxTimer *tmrFocusInputControl;
	Client *m_client;

	ClientUIMDICanvas* GetContext(const wxString &context, bool create_if_not_exist = true);
	void AddLine(const wxString &context, const wxString &line, const wxColour &line_colour = *wxBLACK, bool create_if_not_exist = true);

private:
	DECLARE_EVENT_TABLE()

};

#endif
