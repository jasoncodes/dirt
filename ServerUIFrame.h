#ifndef ServerUIFrame_H_
#define ServerUIFrame_H_

#include "Server.h"

class InputControl;

class ServerUIFrame : public wxFrame, public ServerEventHandler
{

public:
	ServerUIFrame();
	virtual ~ServerUIFrame();

	virtual void Output(const wxString &line);

protected:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params);
	virtual void OnServerInformation(const wxString &line);
	virtual void OnServerWarning(const wxString &line);

protected:
	void OnInput(wxCommandEvent &event);
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);

protected:
	virtual bool ResetWindowPos();

protected:
	Server *m_server;
	wxTextCtrl *m_txtLog;
	InputControl *m_txtInput;

private:
	DECLARE_EVENT_TABLE()

};

#endif
