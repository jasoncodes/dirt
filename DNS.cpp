#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: DNS.cpp,v 1.12 2003-06-30 13:06:12 jason Exp $)

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
				#if defined(__WXMSW__) || defined(__WXMAC__)
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
	CleanUp();
}

void DNS::CleanUp()
{
	if (!m_worker) return;
	m_worker->m_no_event = true;
	m_section.Enter();
	wxLogNull supress_log;
	if (IsBusy())
	{
		// Kill() has issues even on Win32 (s_section_lookup not leaving)
		//#ifdef __WXMSW__
		//	m_worker->Kill();
		//#else
			m_worker->Delete();
		//#endif
	}
	else
	{
		m_worker->Delete();
	}
	delete m_worker;
	m_worker = NULL;
	m_section.Leave();
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
	CleanUp();
	return x;
}

bool DNS::Lookup(const wxString &hostname)
{
	
	m_section.Enter();
	
	if (IsBusy())
	{
		m_section.Leave();
		return false;
	}

	CleanUp();

	m_worker = new DNSWorker(this);

	if (m_worker->Create() != wxTHREAD_NO_ERROR)
	{
		CleanUp();
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
		CleanUp();
		m_section.Leave();
		return false;
	}

}
