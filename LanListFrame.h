#ifndef LanListFrame_H_
#define LanListFrame_H_

#include "ConfigFile.h"
#include <wx/listctrl.h>

class LanListFrame : public wxFrame
{

public:
	LanListFrame();
	virtual ~LanListFrame();

protected:
	void OnClose(wxCloseEvent &event);
	void OnSize(wxSizeEvent &event);

protected:
	ConfigFile m_config;
	wxListCtrl *m_lstServers;

private:
	DECLARE_EVENT_TABLE()

};

#endif
