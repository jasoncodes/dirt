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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: DNS.cpp,v 1.18 2004-05-16 08:36:16 jason Exp $)

#include "DNS.h"

const wxEventType wxEVT_DNS = wxNewEventType();

static wxUint32 GetIPV4Address(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, 0, wxT("Not an IPV4 address"));
	return GAddress_INET_GetHostAddress(addr.GetAddress());
}

class DNSWorker : public wxThread
{

	friend class DNS;

protected:
	DNSWorker(DNS *dns)
		: wxThread(wxTHREAD_JOINABLE), m_dns(dns)
	{
		m_no_event = false;
	}

	virtual ExitCode Entry()
	{
		wxString hostname = m_dns->m_hostname;
		m_dns->m_section.Leave();
		wxIPV4address addr;
		s_section_lookup.Enter();
		bool success = addr.Hostname(hostname);
		s_section_lookup.Leave();
		if (!TestDestroy())
		{
			wxUint32 ip = GetIPV4Address(addr);
			if (!m_no_event)
			{
				DNSEvent evt(m_dns->m_id, m_dns, success, hostname, addr, ip);
				m_dns->m_handler->AddPendingEvent(evt);
				#if defined(__WXMSW__)
					::wxWakeUpMainThread();
				#endif
			}
		}
		return NULL;
	}

protected:
	DNS *m_dns;
	bool m_no_event;
	static wxCriticalSection s_section_lookup;

	DECLARE_NO_COPY_CLASS(DNSWorker)

};

wxCriticalSection DNSWorker::s_section_lookup;

DNS::DNS()
{
	m_handler = NULL;
	m_id = wxID_ANY;
	m_worker = NULL;
}

DNS::~DNS()
{
	CleanUp(true);
}

void DNS::CleanUp(bool lock_section)
{
	if (!m_worker) return;
	m_worker->m_no_event = true;
	if (lock_section)
	{
		m_section.Enter();
	}
	wxLogNull supress_log;
	if (IsBusy())
	{
		// if the worker is busy, this will block until complete
		m_worker->Delete();
	}
	else
	{
		m_worker->Delete();
	}
	m_worker->Wait();
	delete m_worker;
	m_worker = NULL;
	if (lock_section)
	{
		m_section.Leave();
	}
}

void DNS::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

bool DNS::IsBusy() const
{
	return m_worker && m_worker->IsAlive();
}

bool DNS::Cancel()
{
	bool x = IsBusy();
	CleanUp(true);
	return x;
}

bool DNS::Lookup(const wxString &hostname, bool block_if_busy)
{
	
	m_section.Enter();
	
	// cleanup call below blocks if busy
	// if we don't want to block, need to check now
	if (!block_if_busy && IsBusy())
	{
		m_section.Leave();
		return false;
	}

	CleanUp(false);

	m_worker = new DNSWorker(this);

	if (m_worker->Create() != wxTHREAD_NO_ERROR)
	{
		CleanUp(false);
		m_section.Leave();
		return false;
	}

	m_hostname = hostname;

	if (m_worker->Run() == wxTHREAD_NO_ERROR)
	{
		return true;
	}
	else
	{
		CleanUp(false);
		m_section.Leave();
		return false;
	}

}
