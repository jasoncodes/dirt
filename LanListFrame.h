#ifndef LanListFrame_H_
#define LanListFrame_H_

#include "ConfigFile.h"
#include <wx/listctrl.h>

class BroadcastSocket;
class BroadcastSocketEvent;
class StringHashMap;

class LanListFrame : public wxFrame
{

public:
	LanListFrame();
	virtual ~LanListFrame();

protected:
	void OnClose(wxCloseEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnTimer(wxTimerEvent &event);
	void OnBroadcast(BroadcastSocketEvent &event);
	void OnItemActivate(wxListEvent &event);

protected:
	virtual void DoPing();
	virtual void ProcessPong(const wxString &ip, wxUint16 port, wxLongLong_t last_update_tick, wxLongLong_t latency, const StringHashMap &server_info);
	virtual long GetEntry(const wxString &ip, wxUint16 port, bool create_if_not_exist);
	virtual void Sort();
	virtual void Cleanup();

protected:
	ConfigFile m_config;
	wxListCtrl *m_lstServers;
	wxTimer *m_tmr;
	BroadcastSocket *m_bcast;

private:
	DECLARE_EVENT_TABLE()

};

#endif
