#ifndef CryptSocket_H_
#define CryptSocket_H_

#include <wx/socket.h>
#include "ByteBuffer.h"
#include "Crypt.h"

class CryptSocketProxy;
class CryptSocketProxySettings;
class DNS;
class DNSEvent;

//////// CryptSocketBase ////////

enum CryptSocketType
{
	cstClient,
	cstServer
};

class CryptSocketBase : public wxEvtHandler
{

	friend class CryptSocketProxy;

public:
	CryptSocketBase();
	virtual ~CryptSocketBase();

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);
	virtual void SetUserData(void *userdata) { m_userdata = userdata; }
	virtual void* GetUserData() const { return m_userdata; }
	virtual void Destroy();
	virtual void Close();
	virtual void CloseWithEvent();
	virtual void Send(const ByteBuffer &data);
	virtual bool IsSendBufferFull() const;
	virtual bool Ok() const { return m_sck ? m_sck->Ok() : false; }
	virtual bool GetLocal(wxSockAddress &addr) const { return m_sck->GetLocal(addr); }
	virtual bool GetPeer(wxSockAddress &addr) const { return m_sck->GetPeer(addr); }
	
	virtual const ByteBuffer GetKeyLocalPublic() const { return m_keyLocalPublic; }
	virtual const ByteBuffer GetKeyLocalPrivate() const { return m_keyLocalPrivate; }
	virtual const ByteBuffer GetKeyRemotePublic() const { return m_keyRemotePublic; }

	virtual void SetKey(const ByteBuffer &public_key, const ByteBuffer &private_key);

	virtual CryptSocketType GetType() const = 0;

	virtual void SetProxySettings(const CryptSocketProxySettings *settings);
	virtual const CryptSocketProxySettings* GetProxySettings() const;

protected:
	virtual void InitBuffers();
	virtual void InitSocketEvents();
	virtual void ProcessIncoming(const byte *ptr, size_t len);
	virtual void DispatchIncoming();
	virtual void AddToSendQueue(const ByteBuffer &data);
	virtual void MaybeSendData();
	virtual void GenerateNewPublicKey();
	virtual void GenerateNewBlockKey();
	virtual void EncryptPendingSends();
	virtual bool InitProxyConnect(wxString &dest_ip, wxUint16 dest_port);
	virtual bool InitProxyListen();
	virtual void RaiseSocketEvent(wxSocketNotify type);

protected:
	void OnSocket(wxSocketEvent &event);
	virtual void OnSocketInput();
	virtual void OnSocketOutput();
	virtual void OnSocketConnection() = 0;
	virtual void OnSocketConnectionLost(const wxString &msg = wxEmptyString);
	virtual void OnSocketConnectionError(const wxString &msg = wxEmptyString);

protected:
	virtual void OnProxyInput(const ByteBuffer &data);
	virtual void ProxySendData(const ByteBuffer &data);

protected:
	wxEvtHandler *m_handler;
	wxEventType m_id;
	void *m_userdata;
	wxSocketBase *m_sck;
	DNS *m_DNS;
	bool m_has_connected;
	ByteBuffer m_buffIn;
	ByteBuffer m_buffOut;
	ByteBuffer m_buffOutProxy;
	ByteBufferArray m_buffOutUnencrypted;
	bool m_bOutputOkay;
	bool m_bInitialOutputEventSent;
	size_t m_currentBlockKeyAgeBytes;
	static const size_t s_maxBlockKeyAgeBytes;
	time_t m_nextBlockKeyChangeTime;
	static const time_t s_maxBlockKeyAgeSeconds;

	ByteBuffer m_keyLocal;
	ByteBuffer m_keyLocalPublic;
	ByteBuffer m_keyLocalPrivate;
	ByteBuffer m_keyRemote;
	ByteBuffer m_keyRemotePublic;

	Crypt m_crypt;

	CryptSocketProxySettings *m_proxy_settings;
	CryptSocketProxy *m_proxy;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(CryptSocketBase)

};

#include <wx/dynarray.h>
WX_DEFINE_ARRAY(CryptSocketBase*, CryptSocketArray);

//////// CryptSocketClient ////////

class CryptSocketServer;

class CryptSocketClient : public CryptSocketBase
{

	friend class CryptSocketServer;

public:
	CryptSocketClient();
	virtual ~CryptSocketClient();

	virtual void Connect(const wxString &host, wxUint16 port);

	virtual CryptSocketType GetType() const { return cstClient; }

protected:
	void OnDNS(DNSEvent &event);

protected:
	virtual void OnSocketConnection();

protected:
	wxUint16 m_port;

private:
	DECLARE_EVENT_TABLE()

};

//////// CryptSocketServer ////////

class CryptSocketServer : public CryptSocketBase
{

public:
	CryptSocketServer();
	virtual ~CryptSocketServer();

	virtual bool Listen(const wxString &host = wxEmptyString, wxUint16 port = 0);
	virtual CryptSocketClient *Accept(wxEvtHandler *handler = NULL, wxEventType id = wxID_ANY, void *userdata = NULL);

	virtual CryptSocketType GetType() const { return cstServer; }

protected:
	virtual void OnSocketConnection();

private:
	DECLARE_EVENT_TABLE()

};

//////// CryptSocketEvent ////////

extern const wxEventType wxEVT_CRYPTSOCKET;

enum CryptSocketNotify
{
	CRYPTSOCKET_INPUT,
	CRYPTSOCKET_OUTPUT,
	CRYPTSOCKET_CONNECTION,
	CRYPTSOCKET_CONNECTION_LOST,
	CRYPTSOCKET_CONNECTION_ERROR
};

class CryptSocketEvent : public wxEvent
{

public:
	CryptSocketEvent(int id, CryptSocketNotify event, CryptSocketBase *src)
		: wxEvent(id, wxEVT_CRYPTSOCKET), m_data(), m_userdata(src->GetUserData())
	{
		wxASSERT(event != CRYPTSOCKET_INPUT);
		m_event = event;
		SetEventObject(src);
	}

	CryptSocketEvent(int id, CryptSocketNotify event, CryptSocketBase *src, const ByteBuffer &data)
		: wxEvent(id, wxEVT_CRYPTSOCKET), m_data(data), m_userdata(src->GetUserData())
	{
		wxASSERT(
			event == CRYPTSOCKET_INPUT ||
			event == CRYPTSOCKET_CONNECTION_LOST ||
			event == CRYPTSOCKET_CONNECTION_ERROR);
		m_event = event;
		SetEventObject(src);
	}

	virtual ~CryptSocketEvent()
	{
	}

	virtual CryptSocketNotify GetSocketEvent() const
	{
		return m_event;
	}

	virtual CryptSocketBase *GetSocket() const
	{
		return (CryptSocketBase*)GetEventObject();
	}

	virtual const ByteBuffer& GetData() const
	{
		return m_data;
	}

	virtual void* GetUserData() const
	{
		return m_userdata;
	}

	virtual wxEvent *Clone() const
	{
		return new CryptSocketEvent(*this);
	}

protected:
	CryptSocketNotify m_event;
	ByteBuffer m_data;
	void *m_userdata;

};

typedef void (wxEvtHandler::*CryptSocketEventFunction)(CryptSocketEvent&);

#define EVT_CRYPTSOCKET(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_CRYPTSOCKET, id, -1, \
		(wxObjectEventFunction) \
		(wxEventFunction) \
		(CryptSocketEventFunction) & func, \
		(wxObject *) NULL ),

#endif
