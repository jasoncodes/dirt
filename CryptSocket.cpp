#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: CryptSocket.cpp,v 1.26 2003-05-31 04:25:32 jason Exp $)

#include "CryptSocket.h"
#include "Crypt.h"
#include "util.h"
#include <wx/datetime.h>

//////// CryptSocketBase ////////

#define CRYPTSOCKET_CHECK_RET(cond,msg) wxCHECK2_MSG(cond, {CloseWithEvent();return;}, msg)

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
	m_proxy = NULL;
	m_proxy_settings = NULL;
	m_userdata = NULL;
	m_bOutputOkay = false;
	m_bInitialOutputEventSent = false;
}

CryptSocketBase::~CryptSocketBase()
{
	Destroy();
	delete m_proxy_settings;
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
	delete m_proxy;
	m_proxy = NULL;
	wxASSERT(!Ok());
	InitBuffers();
}

void CryptSocketBase::InitBuffers()
{
	m_buffIn = ByteBuffer();
	m_buffOut = ByteBuffer();
	m_buffOutUnencrypted.Empty();
	m_bOutputOkay = false;
	m_bInitialOutputEventSent = false;
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
	wxCHECK_RET(public_key.Length() > 0, wxT("public key is empty"));
	wxCHECK_RET(private_key.Length() > 0, wxT("private key is empty"));
	m_keyLocalPrivate = private_key;
	m_keyLocalPublic = public_key;
	if (Ok())
	{
		AddToSendQueue(Uint16ToBytes(0) + Uint16ToBytes(mtNewPublicKey) + m_keyLocalPublic);
	}
}

void CryptSocketBase::GenerateNewPublicKey()
{
	try
	{
		ByteBuffer public_key, private_key;
		Crypt::RSAGenerateKey(1024, public_key, private_key);
		SetKey(public_key, private_key);
	}
	catch (...)
	{
		CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error generating public key"));
	}
}

void CryptSocketBase::GenerateNewBlockKey()
{
	CRYPTSOCKET_CHECK_RET(m_keyRemotePublic.Length() > 0, wxT("Cannot generate block key without remote's public key"));
	try
	{
		m_keyLocal = Crypt::Random(32);
		m_crypt.SetAESEncryptKey(m_keyLocal);
		ByteBuffer tmp = m_crypt.RSAEncrypt(m_keyRemotePublic, m_keyLocal);
		AddToSendQueue(Uint16ToBytes(0) + Uint16ToBytes(mtNewBlockKey) + tmp);
		m_currentBlockKeyAgeBytes = 0;
		m_nextBlockKeyChangeTime = Now() + s_maxBlockKeyAgeSeconds;
	}
	catch (...)
	{
		CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error generating block key"));
	}
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
			wxFAIL_MSG(wxT("Unknown wxSocket event type in CryptSocketBase::OnSocket"));
			break;

	}

}

void CryptSocketBase::OnSocketInput()
{

	ByteBuffer buff(4096);
	m_sck->Read(buff.LockReadWrite(), buff.Length());
	buff.Unlock();
	
	if (m_sck->Error())
	{
		CloseWithEvent();
	}
	else if (m_sck->LastCount())
	{
		if (m_sck->LastCount() == buff.Length())
		{
			m_buffIn += buff;
		}
		else
		{
			m_buffIn += ByteBuffer(buff.LockRead(), m_sck->LastCount());
			buff.Unlock();
		}
		DispatchIncoming();
	}

}

void CryptSocketBase::ProcessIncoming(const byte *ptr, size_t len)
{

	CRYPTSOCKET_CHECK_RET(len > 2, wxT("Packet length must be greater than 2 bytes long"));
	size_t data_len = BytesToUint16(ptr, 2);
	ptr += 2;
	len -= 2;
	
	if (data_len > 0)
	{
		
		CRYPTSOCKET_CHECK_RET(m_keyRemote.Length() > 0, wxT("No remote public key"));
		ByteBuffer enc(ptr, len);
		ByteBuffer dec;
		try
		{
			dec = m_crypt.AESDecrypt(enc);
		}
		catch (...)
		{
			CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error decryping message"));
		}
		ByteBuffer plain(dec.LockRead(), data_len);
		dec.Unlock();
		CryptSocketEvent evt(m_id, CRYPTSOCKET_INPUT, this, plain);
		m_handler->AddPendingEvent(evt);

	}
	else
	{
		
		CRYPTSOCKET_CHECK_RET(len >= 2, wxT("Message length must be at least 2 bytes long"));
		MessageTypes type = (MessageTypes)BytesToUint16(ptr, 2);
		ptr += 2;
		len -= 2;
		
		switch (type)
		{

			case mtNewPublicKey:
				m_keyRemotePublic = ByteBuffer(ptr, len);
				EncryptPendingSends();
				if (!m_bInitialOutputEventSent && m_bOutputOkay)
				{
					OnSocketOutput();
				}
				break;

			case mtNewBlockKey:
				try
				{
					ByteBuffer tmp(ptr, len);
					m_keyRemote = m_crypt.RSADecrypt(m_keyLocalPrivate, tmp);
					m_crypt.SetAESDecryptKey(m_keyRemote);
				}
				catch (...)
				{
					CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error decrypting block key"));
				}
				break;

			default:
				CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Unknown message type"));
				break;

		}

	}

}

void CryptSocketBase::DispatchIncoming()
{

	const byte *ptr = m_buffIn.LockRead();
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

	if (!m_sck || !m_sck->IsConnected()) return;

	MaybeSendData();

	if (m_bOutputOkay && m_keyRemotePublic.Length() > 0)
	{
		if (m_handler)
		{
			CryptSocketEvent evt(m_id, CRYPTSOCKET_OUTPUT, this);
			m_handler->AddPendingEvent(evt);
			m_bInitialOutputEventSent = true;
		}
	}

}

void CryptSocketBase::EncryptPendingSends()
{
	CRYPTSOCKET_CHECK_RET(m_keyRemotePublic.Length() > 0, wxT("EncryptPendingSends() called when no remote public key"));
	for (size_t i = 0; i < m_buffOutUnencrypted.GetCount(); ++i)
	{
		Send(m_buffOutUnencrypted.Item(i));
	}
	m_buffOutUnencrypted.Clear();
}

void CryptSocketBase::MaybeSendData()
{

	if (!m_bOutputOkay && m_sck && m_sck->IsConnected()) return;
	
	const byte *ptr = m_buffOut.LockRead();
	size_t len = m_buffOut.Length();

	while (len > 0)
	{

		int iSendLen = wxMin(4096, len);
		m_sck->Write(ptr, iSendLen);
		if (m_sck->Error())
		{
			if (m_sck->LastError() != wxSOCKET_WOULDBLOCK)
			{
				CloseWithEvent();
			}
			m_bOutputOkay = false;
			break;
		}
		else
		{
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
	if (m_keyRemotePublic.Length() > 0)
	{
		if ((m_keyLocal.Length() == 0) ||
			(m_currentBlockKeyAgeBytes >= s_maxBlockKeyAgeBytes) ||
			(Now() >= m_nextBlockKeyChangeTime))
		{
			GenerateNewBlockKey();
		}
		CRYPTSOCKET_CHECK_RET(m_keyLocal.Length() > 0, wxT("No local block key"));
		try
		{
			AddToSendQueue(Uint16ToBytes(data.Length()) + m_crypt.AESEncrypt(data));
		}
		catch (...)
		{
			CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error encrypting message"));
		}
		m_currentBlockKeyAgeBytes += data.Length();
	}
	else
	{
		m_buffOutUnencrypted.Add(data);
	}
}

void CryptSocketBase::AddToSendQueue(const ByteBuffer &data)
{
	m_buffOut += (Uint16ToBytes(data.Length()) + data);
	MaybeSendData();
}

void CryptSocketBase::CloseWithEvent()
{
	if (m_sck->IsConnected())
	{
		if (m_sck)
		{
			m_sck->Close();
		}
		wxSocketEvent evt(ID_SOCKET);
		evt.m_event = wxSOCKET_LOST;
		AddPendingEvent(evt);
	}
}

void CryptSocketBase::OnSocketLost()
{
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_LOST, this);
		m_handler->AddPendingEvent(evt);
	}
}

void CryptSocketBase::SetProxySettings(const CryptSocketProxySettings &settings)
{
	delete m_proxy_settings;
	m_proxy_settings = new CryptSocketProxySettings(settings);
}

const CryptSocketProxySettings* CryptSocketBase::GetProxySettings() const
{
	return m_proxy_settings;
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
	if (m_keyLocalPublic.Length() == 0 || m_keyLocalPrivate.Length() == 0)
	{
		wxCHECK2_MSG(m_keyLocalPublic.Length() == 0 && m_keyLocalPrivate.Length() == 0, {}, wxT("One of the local public or private keys is empty"));
		GenerateNewPublicKey();
	}
	else
	{
		SetKey(m_keyLocalPublic, m_keyLocalPrivate);
	}

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

CryptSocketClient* CryptSocketServer::Accept(wxEvtHandler *handler, wxEventType id, void *userdata)
{
	CryptSocketClient *sck = new CryptSocketClient;
	wxCHECK_MSG(!sck->m_sck, NULL, wxT("Socket is not initialised"));
	sck->Destroy();
	if (m_keyLocalPublic.Length() && m_keyLocalPrivate.Length())
	{
		sck->SetKey(m_keyLocalPublic, m_keyLocalPrivate);
	}
	sck->m_sck = new wxSocketClient;
	sck->InitBuffers();
	sck->InitSocketEvents();
	sck->SetEventHandler(handler?handler:m_handler, (id!=wxID_ANY)?id:m_id);
	sck->SetUserData(userdata);
	bool success = ((wxSocketServer*)m_sck)->AcceptWith(*sck->m_sck);
	if (success)
	{
		sck->OnSocketConnection();
		return sck;
	}
	else
	{
		wxFAIL_MSG(wxT("CryptSocketServer::Accept failed"));
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
