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

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();

protected:
	Server *m_server;

private:
	DECLARE_EVENT_TABLE()

};

#endif
