#ifndef DNS_H_
#define DNS_H_

#include <wx/socket.h>
#include <wx/thread.h>

class DNSWorker;

class DNS : public wxEvtHandler
{

	friend class DNSWorker;

public:
	DNS();
	virtual ~DNS();

	virtual bool Lookup(const wxString &hostname, bool block_if_busy = true);
	virtual bool Cancel();
	virtual bool IsBusy() const;

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);

protected:
	virtual void CleanUp();

protected:
	wxEvtHandler *m_handler;
	wxEventType m_id;
	DNSWorker *m_worker;
	wxString m_hostname;
	wxCriticalSection m_section;

	DECLARE_NO_COPY_CLASS(DNS)

};

extern const wxEventType wxEVT_DNS;

class DNSEvent : public wxEvent
{

public:
	DNSEvent(int id, DNS *src, bool success, const wxString &hostname, const wxIPV4address &addr, wxUint32 ip)
		: wxEvent(id, wxEVT_DNS), m_success(success), m_hostname(hostname), m_addr((wxIPV4address*)addr.Clone()), m_ip(ip)
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
