#ifndef ServerUIFrameConfig_H_
#define ServerUIFrameConfig_H_

#include "ServerUIFrame.h"

class ServerUIFrameConfig : public wxDialog
{

public:
	ServerUIFrameConfig(ServerUIFrame *parent, Server *server);
	virtual ~ServerUIFrameConfig();

protected:
	void OnOK(wxCommandEvent &event);
	void OnReset(wxCommandEvent &event);
	void OnChange(wxCommandEvent &event);
	void OnTimer(wxTimerEvent &event);

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();
	virtual void ReportError(const wxString &error_message, wxTextCtrl *txt);
	virtual void ReportError(const wxString &error_message, wxCheckBox *chk);

protected:
	Server *m_server;

	wxTextCtrl *m_txtListenPort;
	wxTextCtrl *m_txtUserPassword;
	wxTextCtrl *m_txtAdminPassword;
	wxTextCtrl *m_txtMaxUsers;
	wxTextCtrl *m_txtMaxUsersIP;
	wxTextCtrl *m_txtServerName;
	wxTextCtrl *m_txtHostname;

	wxCheckBox *m_chkPublicListEnabled;
	wxTextCtrl *m_txtPublicListAuthentication;
	wxTextCtrl *m_txtPublicListComment;

	wxCheckBox *m_chkHTTPProxyEnabled;
	wxTextCtrl *m_txtHTTPProxyHostname;
	wxTextCtrl *m_txtHTTPProxyPort;
	wxTextCtrl *m_txtHTTPProxyUsername;
	wxTextCtrl *m_txtHTTPProxyPassword;

	wxButton *m_cmdApply;

	wxTimer *m_tmrNextUpdate;
	wxStaticText *m_lblNextUpdate;

private:
	DECLARE_EVENT_TABLE()

};

#endif
