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


#ifndef DNS_H_
#define DNS_H_

#include <wx/socket.h>

class DNSThread;

class DNS : public wxEvtHandler
{

	friend class DNSThread;

public:
	DNS();
	virtual ~DNS();

	virtual void Lookup(const wxString &question, bool is_reverse = false, void *userdata = NULL);
	virtual void Cancel(void *userdata = NULL);

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);

protected:
	wxEvtHandler *m_handler;
	wxEventType m_id;

	DECLARE_NO_COPY_CLASS(DNS)

};

extern const wxEventType wxEVT_DNS;

class DNSEvent : public wxEvent
{

public:
	DNSEvent(int id, DNS *src, bool success, const wxString &hostname, const wxIPV4address &addr, wxUint32 ip, void *userdata)
		: wxEvent(id, wxEVT_DNS), m_success(success), m_hostname(hostname), m_addr((wxIPV4address*)addr.Clone()), m_ip(ip), m_userdata(userdata)
	{
		SetEventObject(src);
	}

	DNSEvent(const DNSEvent &evt)
		: wxEvent(evt.GetId(), wxEVT_DNS)
	{
		SetEventObject(evt.GetDNS());
		m_success = evt.m_success;
		m_hostname = evt.m_hostname;
		m_addr = (wxIPV4address*)evt.m_addr->Clone();
		m_ip = evt.m_ip;
		m_userdata = evt.m_userdata;
	}

	virtual ~DNSEvent()
	{
		delete m_addr;
	}

	virtual DNS *GetDNS() const
	{
		return (DNS*)GetEventObject();
	}

	virtual bool IsSuccess() const
	{
		return m_success;
	}

	virtual const wxString& GetHostname() const
	{
		return m_hostname;
	}

	virtual const wxIPV4address& GetAddress() const
	{
		return *m_addr;
	}

	virtual wxUint32 GetIP() const
	{
		return m_ip;
	}

	virtual wxEvent *Clone() const
	{
		return new DNSEvent(*this);
	}

	virtual void* GetUserData() const
	{
		return m_userdata;
	}

protected:
	bool m_success;
	wxString m_hostname;
	wxIPV4address *m_addr;
	wxUint32 m_ip;
	void *m_userdata;

};

typedef void (wxEvtHandler::*DNSEventFunction)(DNSEvent&);

#define EVT_DNS(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_DNS, id, -1, \
		(wxObjectEventFunction) \
		(wxEventFunction) \
		(DNSEventFunction) & func, \
		(wxObject *) NULL ),

#endif
