/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


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
	DECLARE_NO_COPY_CLASS(LanListFrame)

};

#endif
