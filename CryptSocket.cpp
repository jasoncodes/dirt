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
RCS_ID($Id: CryptSocket.cpp,v 1.42 2004-05-16 04:42:44 jason Exp $)

#include "CryptSocket.h"
#include "Crypt.h"
#include "util.h"
#include <wx/datetime.h>
#include "CryptSocketProxy.h"
#include "DNS.h"
#include "IPInfo.h"

//////// CryptSocketBase ////////

#define CRYPTSOCKET_CHECK_RET(cond,msg) wxCHECK2_MSG(cond, { if (!m_fail_msg.Length()) { m_fail_msg = wxString() << wxT("Assertion failed: ") << msg; } CloseWithEvent(); return; }, msg)

const size_t CryptSocketBase::s_maxBlockKeyAgeBytes = 1024 * 512; // 512 KB
const time_t CryptSocketBase::s_maxBlockKeyAgeSeconds = 300; // 5 minutes

static time_t Now()
{
	return wxDateTime::Now().GetTicks();
}

enum
{
	ID_SOCKET = 1,
	ID_DNS
};

enum CryptSocketMessageTypes
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
	m_DNS = NULL;
	m_userdata = NULL;
	m_bOutputOkay = false;
	m_bInitialOutputEventSent = false;
	InitBuffers();
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
	delete m_DNS;
	m_DNS = NULL;
	m_has_connected = false;
	wxASSERT(!Ok());
	InitBuffers();
}

void CryptSocketBase::InitBuffers()
{
	m_buffIn = ByteBuffer();
	m_buffOut = ByteBuffer();
	m_buffOutProxy = ByteBuffer();
	m_buffOutUnencrypted.Clear();
	m_bOutputOkay = false;
	m_bInitialOutputEventSent = false;
	m_keyLocal = ByteBuffer();
	m_keyRemote = ByteBuffer();
	m_crypt.ZlibResetCompress();
	m_crypt.ZlibResetDecompress();
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
			m_has_connected = true;
			OnSocketConnection();
			break;
		
		case wxSOCKET_LOST:
			if (m_has_connected)
			{
				OnSocketConnectionLost(m_fail_msg);
			}
			else
			{
				OnSocketConnectionError(m_fail_msg);
			}
			m_fail_msg.Clear();
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
		if (!m_fail_msg.Length())
		{
			m_fail_msg = wxT("Error reading from socket");
		}
		CloseWithEvent();
	}
	else if (m_sck->LastCount())
	{
		ByteBuffer data;
		if (m_sck->LastCount() == buff.Length())
		{
			data = buff;
		}
		else
		{
			data = ByteBuffer(buff.LockRead(), m_sck->LastCount());
			buff.Unlock();
		}
		if (m_proxy && !m_proxy->IsConnectedToRemote())
		{
			m_proxy->OnInput(data);
		}
		else
		{
			m_buffIn += data;
			DispatchIncoming();
		}
	}

}

void CryptSocketBase::OnProxyInput(const ByteBuffer &data)
{
	m_buffIn += data;
	DispatchIncoming();
}

void CryptSocketBase::OnConnectedToRemote()
{
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_CONNECTION, this);
		m_handler->AddPendingEvent(evt);
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
		CRYPTSOCKET_CHECK_RET(data_len <= dec.Length(), wxT("Data length greater than packet length"));
		ByteBuffer compressed(dec.LockRead(), data_len);
		dec.Unlock();
		ByteBuffer plain;
		try
		{
			plain = m_crypt.ZlibDecompress(compressed);
		}
		catch (...)
		{
			CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error decompressing message"));
		}
		CryptSocketEvent evt(m_id, CRYPTSOCKET_INPUT, this, plain);
		m_handler->AddPendingEvent(evt);

	}
	else
	{
		
		CRYPTSOCKET_CHECK_RET(len >= 2, wxT("Message length must be at least 2 bytes long"));
		CryptSocketMessageTypes type = (CryptSocketMessageTypes)BytesToUint16(ptr, 2);
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
	
	bool proxy_mode = (m_proxy && !m_proxy->IsConnectedToRemote());

	ByteBuffer &buff =
		proxy_mode ?
		m_buffOutProxy :
		m_buffOut;

	if (!proxy_mode && m_buffOutProxy.Length())
	{
		m_buffOut = m_buffOutProxy + m_buffOut;
		m_buffOutProxy = ByteBuffer();
	}

	const byte *ptr = buff.LockRead();
	size_t len = buff.Length();

	while (len > 0)
	{

		int iSendLen = wxMin(4096, len);
		m_sck->Write(ptr, iSendLen);
		if (m_sck->Error())
		{
			if (m_sck->LastError() != wxSOCKET_WOULDBLOCK)
			{
				if (!m_fail_msg.Length())
				{
					m_fail_msg = wxT("Error writing to socket");
				}
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

	if (len != buff.Length())
	{
		ByteBuffer new_buff(ptr, len);
		buff.Unlock();
		buff = new_buff;
	}
	else
	{
		buff.Unlock();
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
		ByteBuffer compressed;
		try
		{
			compressed = m_crypt.ZlibCompress(data);
		}
		catch (...)
		{
			CRYPTSOCKET_CHECK_RET(wxAssertFailure, wxT("Error compressing message"));
		}
		try
		{
			AddToSendQueue(Uint16ToBytes(compressed.Length()) + m_crypt.AESEncrypt(compressed));
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

void CryptSocketBase::ProxySendData(const ByteBuffer &data)
{
	wxASSERT(m_proxy && !m_proxy->IsConnectedToRemote());
	m_buffOutProxy += data;
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

void CryptSocketBase::OnSocketConnectionLost(const wxString &msg)
{
	if (m_sck)
	{
		m_sck->Close();
	}
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_LOST, this, msg);
		m_handler->AddPendingEvent(evt);
	}
}

void CryptSocketBase::OnSocketConnectionError(const wxString &msg)
{
	if (m_sck)
	{
		m_sck->Close();
	}
	if (m_handler)
	{
		CryptSocketEvent evt(m_id, CRYPTSOCKET_ERROR, this, msg);
		m_handler->AddPendingEvent(evt);
	}
}

void CryptSocketBase::SetProxySettings(const CryptSocketProxySettings *settings)
{
	delete m_proxy_settings;
	if (settings)
	{
		m_proxy_settings = new CryptSocketProxySettings(*settings);
	}
	else
	{
		m_proxy_settings = NULL;
	}
}

const CryptSocketProxySettings* CryptSocketBase::GetProxySettings() const
{
	return m_proxy_settings;
}

bool CryptSocketBase::InitProxyConnect(wxString &dest_ip, wxUint16 dest_port)
{
	wxASSERT(GetType() == cstClient);
	delete m_proxy;
	m_proxy = NULL;
	if (m_proxy_settings &&
		m_proxy_settings->DoesDestDestIPMatch(dest_ip) &&
		m_proxy_settings->DoesDestPortMatch(dest_port))
	{
		m_proxy = m_proxy_settings->NewProxyConnect(this, dest_ip, dest_port);
	}
	return (m_proxy != NULL);
}

bool CryptSocketBase::InitProxyListen()
{
	wxASSERT(GetType() == cstServer);
	delete m_proxy;
	m_proxy = NULL;
	if (m_proxy_settings)
	{
		m_proxy = m_proxy_settings->NewProxyListen(this);
	}
	return (m_proxy != NULL);
}

void CryptSocketBase::RaiseSocketEvent(wxSocketNotify type)
{
	wxSocketEvent event;
	event.SetEventObject(m_sck);
	event.SetEventType(type);
	OnSocket(event);
}

//////// CryptSocketClient ////////

BEGIN_EVENT_TABLE(CryptSocketClient, CryptSocketBase)
	EVT_DNS(ID_DNS, CryptSocketClient::OnDNS)
END_EVENT_TABLE()

CryptSocketClient::CryptSocketClient()
{
}

CryptSocketClient::~CryptSocketClient()
{
}

void CryptSocketClient::Connect(const wxString &host, wxUint16 port)
{
	
	Destroy();

	m_sck = new wxSocketClient;
	InitBuffers();
	InitSocketEvents();

	m_port = port;

	m_DNS = new DNS;
	m_DNS->SetEventHandler(this, ID_DNS);

	if (!m_DNS->Lookup(host))
	{
		OnSocketConnectionError(wxT("Cannot start DNS"));
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

	if (m_proxy)
	{
		m_proxy->OnConnect();
	}
	else
	{
		OnConnectedToRemote();
	}

}

void CryptSocketClient::OnDNS(DNSEvent &event)
{

	if (event.IsSuccess())
	{

		wxString ip = GetIPV4AddressString(event.GetIP());

		if (!m_proxy)
		{

			if (InitProxyConnect(ip, m_port))
			{

				m_port = m_proxy_settings->GetPort();

				if (!m_DNS->Lookup(m_proxy_settings->GetHostname()))
				{
					OnSocketConnectionError(wxT("Cannot start DNS (2)"));
				}

				return;

			}
		}

		wxIPV4address addr;
		addr.Hostname(ip);
		addr.Service(m_port);

		if (((wxSocketClient*)m_sck)->Connect(addr, false))
		{
			RaiseSocketEvent(wxSOCKET_CONNECTION);
		}

	}
	else
	{

		OnSocketConnectionError(wxT("Error resolving ") + event.GetHostname());

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

void CryptSocketServer::Listen(const wxString &host, wxUint16 port)
{

	Destroy();

	if (InitProxyListen())
	{

		if (host.Length() || port)
		{
			CryptSocketEvent evt(m_id, CRYPTSOCKET_ERROR, this, wxString(wxT("Proxy listen doesn't support binding")));
			m_handler->AddPendingEvent(evt);
			return;
		}

		CryptSocketEvent evt(m_id, CRYPTSOCKET_ERROR, this, wxString(wxT("Proxy listen not implemented yet")));
		m_handler->AddPendingEvent(evt);

	}
	else
	{

		wxIPV4address addr;
		
		bool ok = true;

		if (host.Length())
		{
			if (!addr.Hostname(host))
			{
				ok = false;
			}
		}
		else
		{
			addr.AnyAddress();
		}

		if (ok)
		{
			addr.Service(port);
			m_sck = new wxSocketServer(addr, wxSOCKET_NOWAIT);
			InitSocketEvents();
			ok = m_sck->Ok();
		}

		if (m_handler)
		{
			CryptSocketEvent evt(m_id, ok?CRYPTSOCKET_LISTEN:CRYPTSOCKET_ERROR, this);
			m_handler->AddPendingEvent(evt);
		}

	}

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
	sck->SetProxySettings(GetProxySettings());
	sck->SetUserData(userdata);
	bool success = ((wxSocketServer*)m_sck)->AcceptWith(*sck->m_sck);
	if (success)
	{
		sck->m_has_connected = true;
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

wxString CryptSocketServer::GetListenIP() const
{
	wxIPV4address addr;
	wxString IP;
	if (Ok() && GetLocal(addr))
	{
		IP = GetIPV4String(addr, false);
		if (IP == wxT("*"))
		{
			IP.Empty();
		}
	}
	return IP;
}

wxUint16 CryptSocketServer::GetListenPort() const
{
	wxIPV4address addr;
	return (Ok() && GetLocal(addr)) ? addr.Service() : 0;
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
