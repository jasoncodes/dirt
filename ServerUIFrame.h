#ifndef ServerUIFrame_H_
#define ServerUIFrame_H_

#include "Server.h"

class InputControl;

class ServerUIFrame : public wxFrame, public ServerEventHandler
{

public:
	ServerUIFrame();
	virtual ~ServerUIFrame();

protected:
	virtual void OnServerLog(const wxString &line);

protected:
	void OnSize(wxSizeEvent &event);
	void OnInput(wxCommandEvent &event);

protected:
	Server *m_server;
	wxTextCtrl *m_txtLog;
	InputControl *m_txtInput;

private:
	DECLARE_EVENT_TABLE()

};

#endif
