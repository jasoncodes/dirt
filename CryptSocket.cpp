#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: CryptSocket.cpp,v 1.3 2003-02-14 12:43:01 jason Exp $)

#include "CryptSocket.h"
#include "Crypt.h"
#include "util.h"
#include <wx/datetime.h>

//////// CryptSocketBase ////////

const size_t CryptSocketBase::s_maxBlockKeyAgeBytes = 1024 * 512; // 512 KB
const time_t CryptSocketBase::s_maxBlockKeyAgeSeconds = 300; // 5 minutes

static time_t Now()
{
	return wxDateTime::Now().GetTicks();
}

enum
{
	ID_SOCKET = 1
};

enum MessageTypes
{
	mtNewPublicKey,
	mtNewBlockKey
};

BEGIN_EVENT_TABLE(CryptSocketBase, wxEvtHandler)
	EVT_SOCKET(ID_SOCKET, CryptSocketBase::OnSocket)
END_EVENT_TABLE()

CryptSocketBase::CryptSocketBase()
{
	m_handler = NULL;
	m_id = 0;
	m_sck = NULL;
	m_bOutputOkay = false;
}

CryptSocketBase::~CryptSocketBase()
{
	Destroy();
}

void CryptSocketBase::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

void CryptSocketBase::Destroy()
{
	Close();
	if (m_sck)
	{
		m_sck->Destroy();
		m_sck = NULL;
	}
	m_keyRemotePublic = ByteBuffer();
}

void CryptSocketBase::Close()
{
	if (m_sck)
	{
		m_sck->Destroy();
		m_sck = NULL;
	}
	wxASSERT(!Ok());
	InitBuffers();
}

void CryptSocketBase::InitBuffers()
{
	m_buffIn = ByteBuffer();
	m_buffOut = ByteBuffer();
	m_bOutputOkay = false;
	m_keyLocal = ByteBuffer();
	m_keyRemote = ByteBuffer();
}

void CryptSocketBase::InitSocketEvents()
{
	m_sck->SetFlags(wxSOCKET_NOWAIT);
	m_sck->SetEventHandler(*this, ID_SOCKET);
	m_sck->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
	m_sck->Notify(true);
}

void CryptSocketBase::SetKey(const ByteBuffer &public_key, const ByteBuffer &private_key)
{
	wxCHECK_RET(public_key.Length() > 0, "public key is empty");
	wxCHECK_RET(private_key.Length() > 0, "private key is empty");
	m_keyLocalPrivate = private_key;
	m_keyLocalPublic = public_key;
	AddToSendQueue(Uint16ToBytes(0) + Uint16ToBytes(mtNewPublicKey) + m_keyLocalPublic);
}

void CryptSocketBase::GenerateNewPublicKey()
{
	ByteBuffer public_key, private_key;
	Crypt::RSAGenerateKey(1024, public_key, private_key);
	SetKey(public_key, private_key);
}

void CryptSocketBase::GenerateNewBlockKey()
{
	m_keyLocal = Crypt::Random(32);
	m_crypt.SetAESEncryptKey(m_keyLocal);
	AddToSendQueue(Uint16ToBytes(0) + Uint16ToBytes(mtNewBlockKey) + m_keyLocal);
	m_currentBlockKeyAgeBytes = 0;
	m_nextBlockKeyChangeTime = Now() + s_maxBlockKeyAgeSeconds;
}

void CryptSocketBase::OnSocket(wxSocketEvent &event)
{

	if (!m_sck) return;

	switch (event.GetSocketEvent())
	{

		case wxSOCKET_INPUT:
			OnSocketInput();
			break;
		
		case wxSOCKET_OUTPUT:
			OnSocketOutput();
			break;
		
		case wxSOCKET_CONNECTION:
			OnSocketConnection();
			break;
		
		case wxSOCKET_LOST:
			OnSocketLost();
			break;
		
		default:
			wxFAIL_MSG("Unknown wxSocket event type in CryptSocketBase::OnSocket");
			break;

	}

}

void CryptSocketBase::OnSocketInput()
{

	ByteBuffer buff(4096);
	m_sck->Read(buff.Lock(), buff.Length());
	buff.Unlock();
	wxCHECK_RET(!m_sck->Error(), "Socket error has occured");
	
	if (m_sck->LastCount())
	{
		if (m_sck->LastCount() == buff.Length())
		{
			m_buffIn += buff;
		}
		else
		{
			m_buffIn += ByteBuffer(buff.Lock(), m_sck->LastCount());
			buff.Unlock();
		}
		DispatchIncoming();
	}

}

void CryptSocketBase::ProcessIncoming(const byte *ptr, size_t len)
{

	wxCHECK_RET(len > 2, "Packet length must be greater than 2 bytes long");
	size_t data_len = BytesToUint16(ptr, 2);
	ptr += 2;
	len -= 2;
	
	if (data_len > 0)
	{
		
		wxCHECK_RET(m_keyRemote.Length() > 0, "No remote public key");
		ByteBuffer enc(ptr, len);
		ByteBuffer dec = m_crypt.AESDecrypt(enc);
		ByteBuffer plain(dec.Lock(), data_len);
		dec.Unlock();
		CryptSocketEvent evt(m_id, CRYPTSOCKET_INPUT, this, plain);
		m_handler->AddPendingEvent(evt);

	}
	else
	{
		
		wxCHECK_RET(len >= 2, "Message length must be at least 2 bytes long");
		MessageTypes type = (MessageTypes)BytesToUint16(ptr, 2);
		ptr += 2;
		len -= 2;
		
		switch (type)
		{

			case mtNewPublicKey:
				m_keyRemotePublic = ByteBuffer(ptr, len);
				break;

			case mtNewBlockKey:
				m_keyRemote = ByteBuffer(ptr, len);
				m_crypt.SetAESDecryptKey(m_keyRemote);
				break;

			default:
				wxFAIL_MSG("Unknown message type");
				break;

		}

	}

}

void CryptSocketBase::DispatchIncoming()
{

	byte *ptr = m_buffIn.Lock();
	size_t len = m_buffIn.Length();

	while (len >= 2)
	{

		size_t size = BytesToUint16(ptr, 2);

		if (len >= size + 2)
		{
			
			size_t segment_len = size + 2;
			
			if (m_handler)
			{
				ProcessIncoming(ptr + 2, size);
			}

			ptr += segment_len;
			len -= segment_len;

		}
		else
		{
			break;
		}

	}

	if (len != m_buffIn.Length())
	{
		ByteBuffer new_buff(ptr, len);
		m_buffIn.Unlock();
		m_buffIn = new_buff;
	}
	else
	{
		m_buffIn.Unlock();
	}

}

void CryptSocketBase::OnSocketOutput()
{

	m_bOutputOkay = true;

	MaybeSendData();

	if (m_bOutputOkay)
	{
		if (m_handler)
		{
			CryptSocketEvent evt(m_id, CRYPTSOCKET_OUTPUT, this);
			m_handler->AddPendingEvent(evt);
		}
	}

}

void CryptSocketBase::MaybeSendData()
{

	wxCHECK_RET(m_sck && m_sck->IsConnected(), "Socket not connected");

	if (!m_bOutputOkay && m_sck && m_sck->IsConnected()) return;
	
	byte *ptr = m_buffOut.Lock();
	size_t len = m_buffOut.Length();

	while (len > 0)
	{

		int iSendLen = wxMin(4096, len);
		m_sck->Write(ptr, iSendLen);
		if (m_sck->Error() && m_sck->LastError() == wxSOCKET_WOULDBLOCK)
		{
			m_bOutputOkay = false;
			break;
		}
		else
		{
			wxCHECK_RET(!m_sck->Error(), "Socket error has occured");
			iSendLen = m_sck->LastCount();
			ptr += iSendLen;
			len -= iSendLen;
		}

	}

	if (len != m_buffOut.Length())
	{
		ByteBuffer new_buff(ptr, len);
		m_buffOut.Unlock();
		m_buffOut = new_buff;
	}
	else
	{
		m_buffOut.Unlock();
	}

}

bool CryptSocketBase::IsSendBufferFull() const
{
	return !m_bOutputOkay;
}

void CryptSocketBase::Send(const ByteBuffer &data)
{
	wxCHECK_RET(m_keyLocal.Length() > 0, "No local block key");
	if ((m_currentBlockKeyAgeBytes >= s_maxBlockKeyAgeBytes) ||
		(Now() >= m_nextBlockKeyChangeTime))
	{
		GenerateNewBlockKey();
	}
	AddToSendQueue(Uint16ToBytes(data.Length()) + m_crypt.AESEncrypt(data));
	m_currentBlockKeyAgeBytes += data.Length();
}

void CryptSocketBase::AddToSendQueue(const ByteBuffer &data)
{
	m_buffOut += (Uint16ToBytes(data.Length()) + data);
	MaybeSendData();
}

void CryptSocketBase::OnSocketLost()
{
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_LOST, this);
		m_handler->AddPendingEvent(evt);
	}
}

//////// CryptSocketClient ////////

BEGIN_EVENT_TABLE(CryptSocketClient, CryptSocketBase)
END_EVENT_TABLE()

CryptSocketClient::CryptSocketClient()
{
}

CryptSocketClient::~CryptSocketClient()
{
}

void CryptSocketClient::Connect(wxSockAddress& addr)
{
	
	Destroy();

	m_sck = new wxSocketClient;
	InitBuffers();
	InitSocketEvents();

	if (((wxSocketClient*)m_sck)->Connect(addr, false))
	{
		wxSocketEvent event;
		event.SetEventObject(m_sck);
		event.SetEventType(wxSOCKET_CONNECTION);
		OnSocket(event);
	}

}

void CryptSocketClient::OnSocketConnection()
{

	InitBuffers();
	if (m_keyLocalPrivate.Length() == 0 || m_keyLocalPublic.Length() == 0)
	{
		wxCHECK2_MSG(m_keyLocalPrivate.Length() == 0 && m_keyLocalPublic.Length() == 0, {}, "One of the local public or private keys is empty");
		GenerateNewPublicKey();
	}
	GenerateNewBlockKey();

	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_CONNECTION, this);
		m_handler->AddPendingEvent(evt);
	}

}

//////// CryptSocketServer ////////

BEGIN_EVENT_TABLE(CryptSocketServer, CryptSocketBase)
END_EVENT_TABLE()

CryptSocketServer::CryptSocketServer()
{
}

CryptSocketServer::~CryptSocketServer()
{
}

bool CryptSocketServer::Listen(wxSockAddress& address)
{
	Destroy();
	m_sck = new wxSocketServer(address, wxSOCKET_NOWAIT);
	InitSocketEvents();
	return m_sck->Ok();
}

CryptSocketClient* CryptSocketServer::Accept(wxEvtHandler *handler, wxEventType id)
{
	CryptSocketClient *sck = new CryptSocketClient;
	wxCHECK_MSG(!sck->m_sck, NULL, "Socket is not initialised");
	sck->Destroy();
	sck->m_sck = new wxSocketClient;
	sck->InitBuffers();
	sck->InitSocketEvents();
	sck->SetEventHandler(handler?handler:m_handler, (id!=wxID_ANY)?id:m_id);
	bool success = ((wxSocketServer*)m_sck)->AcceptWith(*sck->m_sck);
	if (success)
	{
		sck->OnSocketConnection();
		return sck;
	}
	else
	{
		wxFAIL_MSG("CryptSocketServer::Accept failed");
		sck->Destroy();
		return NULL;
	}
}

void CryptSocketServer::OnSocketConnection()
{
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_CONNECTION, this);
		m_handler->AddPendingEvent(evt);
	}
}

//////// CryptSocketEvent ////////

const wxEventType wxEVT_CRYPTSOCKET = wxNewEventType();

//////// IPV4 Utility Functions ////////

wxUint32 GetIPV4Address(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, 0, "Not an IPV4 address");
	return GAddress_INET_GetHostAddress(addr.GetAddress());
}

wxString GetIPV4AddressString(wxSockAddress &addr)
{
	
	wxUint32 ip = GetIPV4Address(addr);

	ip = wxUINT32_SWAP_ON_LE(ip);
		
	wxString result = wxString()
		<< ((ip >> 24) & 0xff) << "."
		<< ((ip >> 16) & 0xff) << "."
		<< ((ip >> 8)  & 0xff) << "."
		<< ((ip >> 0)  & 0xff);
	
	return result;
	
}

wxString GetIPV4String(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, wxEmptyString, "Not an IPV4 address");
	wxIPV4address *ipv4 = static_cast<wxIPV4address*>(&addr);
	wxString retval;
	wxString ip = GetIPV4AddressString(*ipv4);
	if (ip == "127.0.0.1")
	{
		retval << "localhost";
	}
	else if (ip == "0.0.0.0")
	{
		retval << "*";
	}
	else if (LeftEq(ip,"127."))
	{
		retval << ip;
	}
	else
	{
		retval << ipv4->Hostname();
	}
	retval << ":" << ipv4->Service();
	return retval;
}
