#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: DNS.cpp,v 1.5 2003-03-05 08:09:44 jason Exp $)

#include "DNS.h"

const wxEventType wxEVT_DNS = wxNewEventType();

static wxUint32 GetIPV4Address(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, 0, wxT("Not an IPV4 address"));
	return GAddress_INET_GetHostAddress(addr.GetAddress());
}

class DNSWorker : public wxThread
{

	friend DNS;

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
		m_dns->s_section_lookup.Enter();
		bool success = addr.Hostname(hostname);
		m_dns->s_section_lookup.Leave();
		wxUint32 ip = GetIPV4Address(addr);
		if (!m_no_event)
		{
			DNSEvent evt(m_dns->m_id, m_dns, success, hostname, addr, ip);
			m_dns->m_handler->AddPendingEvent(evt);
		}
		return NULL;
	}

protected:
	DNS *m_dns;
	bool m_no_event;

};

wxCriticalSection DNS::s_section_lookup;

DNS::DNS()
{
	m_handler = NULL;
	m_id = wxID_ANY;
	m_worker = new DNSWorker(this);
	if (m_worker->Create() != wxTHREAD_NO_ERROR)
	{
		delete m_worker;
		m_worker = NULL;
	}
}

DNS::~DNS()
{
	if (m_worker)
	{
		m_worker->m_no_event = true;
		m_section.Enter();
		StopThread();
		delete m_worker;
		m_section.Leave();
	}
}

void DNS::StopThread()
{
	if (IsBusy())
	{
		#ifdef __WXMSW__
			m_worker->Kill();
		#else
			m_worker->Delete();
		#endif
	}
	else
	{
		m_worker->Delete();
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
	bool was_busy = IsBusy();
	bool entered = false;
	if (m_worker && IsBusy())
	{
		m_worker->m_no_event = true;
		m_section.Enter();
		entered = true;
		StopThread();
		delete m_worker;
		m_worker = NULL;
	}
	if (!m_worker)
	{
		if (!entered)
		{
			m_section.Enter();
			entered = true;
		}
		m_worker = new DNSWorker(this);
		if (m_worker->Create() != wxTHREAD_NO_ERROR)
		{
			delete m_worker;
			m_worker = NULL;
		}
	}
	if (entered)
	{
		m_section.Leave();
	}
	return was_busy && !IsBusy();
}

bool DNS::Lookup(const wxString &hostname)
{
	if (!m_worker) return false;
	m_section.Enter();
	if (IsBusy())
	{
		m_section.Leave();
		return false;
	}
	else
	{
		m_hostname = hostname;
		bool retry = false;
		do
		{
			wxThreadError err = m_worker->Run();
			if (err == wxTHREAD_NO_ERROR)
			{
				return true;
			}
			else if (err == wxTHREAD_RUNNING)
			{
				if (retry)
				{
					m_section.Leave();
					return false;
				}
				if (m_worker->Delete() != wxTHREAD_NO_ERROR)
				{
					m_section.Leave();
					return false;
				}
				delete m_worker;
				m_worker = new DNSWorker(this);
				if (m_worker->Create() != wxTHREAD_NO_ERROR)
				{
					m_section.Leave();
					return false;
				}
				retry = true;
			}
			else
			{
				m_section.Leave();
				retry = false;
			}
		}
		while (retry);
		return false;
	}
}
