#ifndef ServerUIFrame_H_
#define ServerUIFrame_H_

#include "Server.h"
#include <wx/listctrl.h>

class InputControl;
class TrayIcon;

class ServerUIFrame : public wxFrame, public ServerEventHandler
{

public:
	ServerUIFrame();
	virtual ~ServerUIFrame();

	virtual void Output(const wxString &line);

protected:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params);
	virtual void OnServerStateChange();
	virtual void OnServerConnectionChange();
	virtual void OnServerInformation(const wxString &line);
	virtual void OnServerWarning(const wxString &line);

protected:
	void OnInput(wxCommandEvent &event);
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnStartStop(wxCommandEvent& event);
	void OnConfiguration(wxCommandEvent& event);
	void OnClient(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnTimerUpdateConnections(wxTimerEvent &event);

protected:
	virtual bool ResetWindowPos();
	virtual void UpdateConnectionList();
	virtual void UpdateTrayIcon();

protected:
	Server *m_server;
	wxTextCtrl *m_txtLog;
	InputControl *m_txtInput;
	wxListCtrl *m_lstConnections;
	wxButton *m_cmdStartStop;
	wxButton *m_cmdConfiguration;
	wxButton *m_cmdClient;
	wxButton *m_cmdClear;
	wxTimer *m_tmrUpdateConnections;
	TrayIcon *m_tray;
	int m_last_xpm;
	wxString m_last_tooltip;

private:
	DECLARE_EVENT_TABLE()

};

#endif
