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

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();

protected:
	Server *m_server;
	wxTextCtrl *m_txtListenPort;
	wxTextCtrl *m_txtUserPass;
	wxTextCtrl *m_txtAdminPass;

private:
	DECLARE_EVENT_TABLE()

};

#endif
