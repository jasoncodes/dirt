#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: DNS.cpp,v 1.1 2003-03-05 04:52:16 jason Exp $)

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
		bool success = addr.Hostname(hostname);
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
		if (IsBusy())
		{
			m_worker->Kill();
		}
		else
		{
			m_worker->Delete();
		}
		delete m_worker;
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
				return false;
			}
		}
		while (retry);
		wxFAIL;
		return false;
	}
}
