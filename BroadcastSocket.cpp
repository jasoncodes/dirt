#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: BroadcastSocket.cpp,v 1.3 2003-04-27 09:28:28 jason Exp $)

#include "BroadcastSocket.h"
#include "util.h"

const wxEventType wxEVT_BROADCAST_SOCKET = wxNewEventType();

class BroadcastSocketData
{

	friend class BroadcastSocket;

protected:
	BroadcastSocketData()
	{
		m_sck = NULL;
	}

public:
	virtual ~BroadcastSocketData()
	{
		delete m_sck;
	}

protected:
	IPInfoEntry m_ipinfo;
	wxDatagramSocket *m_sck;

private:
	DECLARE_NO_COPY_CLASS(BroadcastSocketData)

};

enum
{
	ID_IPCHECK = 1,
	ID_SOCKET
};

BEGIN_EVENT_TABLE(BroadcastSocket, wxEvtHandler)
	EVT_TIMER(ID_IPCHECK, BroadcastSocket::OnIPCheck)
	EVT_SOCKET(ID_SOCKET, BroadcastSocket::OnSocket)
END_EVENT_TABLE()

BroadcastSocket::BroadcastSocket(unsigned short port)
{

	m_handler = NULL;
	m_id = wxID_ANY;

	m_port = port;
	m_tmrIPCheck = new wxTimer(this, ID_IPCHECK);

	wxIPV4address addr;
	addr.AnyAddress();
	addr.Service(m_port);
	wxDatagramSocket *sck = new wxDatagramSocket(addr);
	if (sck->Ok() && GetIPInfo().GetCount() > 0)
	{
		sck->GetLocal(addr);
		delete sck;
		m_port = addr.Service();
		wxASSERT(m_port > 0);
		m_ok = true;
		CheckForIPs();
		m_tmrIPCheck->Start(5000);
	}
	else
	{
		delete sck;
		m_ok = false;
	}

}

BroadcastSocket::~BroadcastSocket()
{
	delete m_tmrIPCheck;
	for (size_t i = 0; i < m_data.GetCount(); ++i)
	{
		delete m_data[i];
	}
	m_data.Clear();
}

void BroadcastSocket::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

void BroadcastSocket::OnIPCheck(wxTimerEvent &event)
{
	CheckForIPs();
}

int BroadcastSocket::Find(const wxString &IP)
{
	for (size_t i = 0; i < m_data.GetCount(); ++i)
	{
		if (m_data[i]->m_ipinfo.IPAddressString == IP)
		{
			return i;
		}
	}
	return -1;
}

bool BroadcastSocket::Ok() const
{
	bool ok = m_ok;
	for (size_t i = 0; i < m_data.GetCount() && ok; ++i)
	{
		ok &= m_data[i]->m_sck->Ok();
	}
	return ok;
}

#if defined(__WXMSW__)
	#include "wx/msw/gsockmsw.h"
	#include <windows.h>
	#include <wx/msw/winundef.h>
	#define socket_t SOCKET
#elif defined(__UNIX__)
	#include "wx/unix/gsockunx.h"
	#define socket_t int
#else
	#error "Sorry, your OS is not supported yet"
#endif

static inline socket_t GetSocket(GSocket *gsck)
{
	#if defined(__WXMSW__)
		return gsck->m_fd;
	#elif defined(__UNIX__)
		return gsck->m_fd;
	#else
		#error "Sorry, your OS is not supported yet"
	#endif
}

class MySocketBase : public wxObject
{
public:
	GSocket *m_socket;
};

static inline GSocket* GetGSocket(wxDatagramSocket *sck)
{
	return ((MySocketBase*)sck)->m_socket;
}

static inline void EnableBroadcast(wxDatagramSocket *sck)
{
	GSocket *gsck = GetGSocket(sck);
	wxASSERT(gsck);
	socket_t fd = GetSocket(gsck);
	#if defined(__WXMSW__) || defined(__UNIX__)
		int value = 1;
		int retval = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char*)&value, sizeof(int));
		wxASSERT(retval == 0);
	#else
		#error "Sorry, your OS is not supported yet"
	#endif
}

void BroadcastSocket::CheckForIPs()
{

	IPInfoEntryArray entries = GetIPInfo();
	wxArrayString IPs;
	IPs.Alloc(entries.GetCount());
	for (size_t i = 0; i < entries.GetCount(); ++i)
	{
		IPs.Add(entries[i].IPAddressString);
	}

	if (m_entries != entries)
	{

		// remove any old IPs
		for (size_t i = m_data.GetCount(); i > 0; --i)
		{
			if (IPs.Index(m_data[i-1]->m_ipinfo.IPAddressString) == -1)
			{
				delete m_data[i-1];
				m_data.RemoveAt(i-1);
			}
		}

		// add any new IPs
		for (size_t i = 0; i < IPs.GetCount(); ++i)
		{
			if (Find(IPs[i]) == -1)
			{
				BroadcastSocketData *data = new BroadcastSocketData;
				data->m_ipinfo = entries[i];
				if (!data->m_ipinfo.BroadcastAddressString.Length())
				{
					data->m_ipinfo.BroadcastAddress = (wxUint32)(wxInt32)-1;
					data->m_ipinfo.BroadcastAddressString = wxT("255.255.255.255");
				}
				wxIPV4address addr;
				addr.Hostname(data->m_ipinfo.IPAddressString);
				addr.Service(m_port);
				data->m_sck = new wxDatagramSocket(addr);
				EnableBroadcast(data->m_sck);
				data->m_sck->SetEventHandler(*this, ID_SOCKET);
				data->m_sck->SetNotify(wxSOCKET_INPUT_FLAG);
				data->m_sck->Notify(true);
				m_data.Add(data);
			}
		}

		m_entries = entries;
		
	}

}

void BroadcastSocket::SendAll(wxUint16 port, const ByteBuffer &data)
{
	const byte *ptr = data.LockRead();
	for (size_t i = 0; i < m_data.GetCount(); ++i)
	{
		if (m_data[i]->m_sck->Ok())
		{
			wxIPV4address addr;
			addr.Hostname(m_data[i]->m_ipinfo.BroadcastAddressString);
			addr.Service(port);
			m_data[i]->m_sck->SendTo(addr, ptr, data.Length());
		}
	}
	data.Unlock();
}

void BroadcastSocket::Send(const wxString &ip, wxUint16 port, const ByteBuffer &data)
{
	const byte *ptr = data.LockRead();
	wxUint32 ipaddr = GetIPV4Address(ip);
	size_t count = 0;
	wxIPV4address addr;
	addr.Hostname(ip);
	addr.Service(port);
	for (size_t i = 0; i < m_data.GetCount(); ++i)
	{
		if (m_data[i]->m_sck->Ok())
		{
			const IPInfoEntry &ipinfo = m_data[i]->m_ipinfo;
			if (ipinfo.NetworkAddress == (ipaddr & ipinfo.SubnetMask))
			{
				count++;
				m_data[i]->m_sck->SendTo(addr, ptr, data.Length());
			}
		}
	}
	if (!count)
	{
		for (size_t i = 0; i < m_data.GetCount(); ++i)
		{
			if (m_data[i]->m_sck->Ok())
			{
				m_data[i]->m_sck->SendTo(addr, ptr, data.Length());
			}
		}
	}
	data.Unlock();
}

void BroadcastSocket::OnSocket(wxSocketEvent &event)
{
	if (event.GetSocketEvent() == wxSOCKET_INPUT)
	{
		wxDatagramSocket *sck = (wxDatagramSocket*)event.GetSocket();
		wxIPV4address addr;
		static ByteBuffer buff(32768);
		byte *ptr = buff.LockReadWrite();
		sck->RecvFrom(addr, ptr, buff.Length());
		if (!sck->Error())
		{
			sck->GetPeer(addr);
			ByteBuffer data(ptr, sck->LastCount());
			buff.Unlock();
			if (m_handler)
			{
				BroadcastSocketEvent evt(m_id, this, GetIPV4AddressString(addr), addr.Service(), data);
				m_handler->AddPendingEvent(evt);
			}
		}
		else
		{
			buff.Unlock();
		}
	}
}