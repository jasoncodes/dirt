#ifndef ServerUIFrameConfig_H_
#define ServerUIFrameConfig_H_

class ServerUIFrame;
class Server;
class TristateConfigPanel;

class ServerUIFrameConfig : public wxDialog
{

public:
	ServerUIFrameConfig(ServerUIFrame *parent, Server *server);
	virtual ~ServerUIFrameConfig();

protected:
	void OnOK(wxCommandEvent &event);
	void OnReset(wxCommandEvent &event);
	void OnChangeText(wxCommandEvent &event);
	void OnChangeCheck(wxCommandEvent &event);
	void OnTimer(wxTimerEvent &event);
	void OnBrowse(wxCommandEvent &event);

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
	wxTextCtrl *m_txtSoundConnection;
	wxTextCtrl *m_txtSoundJoin;

	TristateConfigPanel *m_pnlLog;

	wxCheckBox *m_chkPublicListEnabled;
	wxStaticText *m_lblPublicListAuthentication;
	wxTextCtrl *m_txtPublicListAuthentication;
	wxStaticText *m_lblPublicListComment;
	wxTextCtrl *m_txtPublicListComment;

	wxCheckBox *m_chkHTTPProxyEnabled;
	wxStaticText *m_lblHTTPProxyHostname;
	wxTextCtrl *m_txtHTTPProxyHostname;
	wxStaticText *m_lblHTTPProxyPort;
	wxTextCtrl *m_txtHTTPProxyPort;
	wxStaticText *m_lblHTTPProxyUsername;
	wxTextCtrl *m_txtHTTPProxyUsername;
	wxStaticText *m_lblHTTPProxyPassword;
	wxTextCtrl *m_txtHTTPProxyPassword;

	wxButton *m_cmdApply;

	wxTimer *m_tmrNextUpdate;
	wxStaticText *m_lblNextUpdate;

private:
	DECLARE_EVENT_TABLE()

};

#endif
