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

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();
	virtual void ReportError(const wxString &error_message, wxTextCtrl *txt);

protected:
	Server *m_server;
	wxTextCtrl *m_txtListenPort;
	wxTextCtrl *m_txtUserPassword;
	wxTextCtrl *m_txtAdminPassword;
	wxButton *m_cmdApply;

private:
	DECLARE_EVENT_TABLE()

};

#endif
