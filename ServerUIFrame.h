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
	void SetPositionAndShow(bool force_foreground);

protected:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params);
	virtual wxArrayString OnServerSupportedCommands();
	virtual void OnServerStateChange();
	virtual void OnServerConnectionChange();
	virtual void OnServerInformation(const wxString &line);
	virtual void OnServerWarning(const wxString &line);

protected:
	void OnSize(wxSizeEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnInput(wxCommandEvent &event);
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnStartStop(wxCommandEvent& event);
	void OnConfiguration(wxCommandEvent& event);
	void OnClient(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnTimerUpdateConnections(wxTimerEvent &event);
	void OnTrayDblClick(wxMouseEvent &event);
	void OnIconize(wxIconizeEvent &event);
	void OnIdle(wxIdleEvent &event);
	void OnTrayRightClick(wxMouseEvent &event);
	void OnRestore(wxCommandEvent &event);
	void OnConnectionRClick(wxListEvent &event);
	void OnConnectionKick(wxCommandEvent &event);

	#ifdef __WXMSW__
		virtual long MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
	#endif

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
	bool m_hide_self;
	ServerConnection *m_right_click_conn;
	bool m_size_set;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ServerUIFrame)

};

#endif
