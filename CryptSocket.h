#ifndef CryptSocket_H_
#define CryptSocket_H_

#include <wx/socket.h>
#include "ByteBuffer.h"
#include "Crypt.h"

//////// CryptSocketBase ////////

class CryptSocketBase : public wxEvtHandler
{

public:
	CryptSocketBase();
	virtual ~CryptSocketBase();

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);
	virtual void SetUserData(void *userdata) { m_userdata = userdata; }
	virtual void* GetUserData() const { return m_userdata; }
	virtual void Destroy();
	virtual void Close();
	virtual void Send(const ByteBuffer &data);
	virtual bool IsSendBufferFull() const;
	virtual bool Ok() const { return m_sck ? m_sck->Ok() : false; }
	virtual bool GetLocal(wxSockAddress &addr) const { return m_sck->GetLocal(addr); }
	virtual bool GetPeer(wxSockAddress &addr) const { return m_sck->GetPeer(addr); }
	
	virtual const ByteBuffer GetKeyLocalPublic() const { return m_keyLocalPublic; }
	virtual const ByteBuffer GetKeyLocalPrivate() const { return m_keyLocalPrivate; }
	virtual const ByteBuffer GetKeyRemotePublic() const { return m_keyRemotePublic; }

	virtual void SetKey(const ByteBuffer &public_key, const ByteBuffer &private_key);

protected:
	virtual void InitBuffers();
	virtual void InitSocketEvents();
	virtual void ProcessIncoming(const byte *ptr, size_t len);
	virtual void DispatchIncoming();
	virtual void AddToSendQueue(const ByteBuffer &data);
	virtual void MaybeSendData();
	virtual void GenerateNewPublicKey();
	virtual void GenerateNewBlockKey();
	virtual void CloseWithEvent();
	virtual void EncryptPendingSends();

protected:
	void OnSocket(wxSocketEvent &event);
	virtual void OnSocketInput();
	virtual void OnSocketOutput();
	virtual void OnSocketConnection() = 0;
	virtual void OnSocketLost();

protected:
	wxEvtHandler *m_handler;
	wxEventType m_id;
	void *m_userdata;
	wxSocketBase *m_sck;
	ByteBuffer m_buffIn;
	ByteBuffer m_buffOut;
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

private:
	DECLARE_EVENT_TABLE()

};

//////// CryptSocketClient ////////

class CryptSocketServer;

class CryptSocketClient : public CryptSocketBase
{

	friend CryptSocketServer;

public:
	CryptSocketClient();
	virtual ~CryptSocketClient();

	virtual void Connect(wxSockAddress& addr);

protected:
	virtual void OnSocketConnection();

private:
	DECLARE_EVENT_TABLE()

};

//////// CryptSocketServer ////////

class CryptSocketServer : public CryptSocketBase
{

public:
	CryptSocketServer();
	virtual ~CryptSocketServer();

	virtual bool Listen(wxSockAddress& address);
	virtual CryptSocketClient *Accept(wxEvtHandler *handler = NULL, wxEventType id = wxID_ANY, void *userdata = NULL);

protected:
	virtual void OnSocketConnection();

private:
	DECLARE_EVENT_TABLE()

};

//////// CryptSocketEvent ////////

extern const wxEventType wxEVT_CRYPTSOCKET;

enum CryptSocketNotify
{
	CRYPTSOCKET_INPUT = wxSOCKET_INPUT,
	CRYPTSOCKET_OUTPUT = wxSOCKET_OUTPUT,
	CRYPTSOCKET_CONNECTION = wxSOCKET_CONNECTION,
	CRYPTSOCKET_LOST = wxSOCKET_LOST
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
		wxASSERT(event == CRYPTSOCKET_INPUT);
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

//////// IPV4 Utility Functions ////////

wxUint32 GetIPV4Address(wxSockAddress &addr);
wxString GetIPV4AddressString(wxSockAddress &addr);
wxString GetIPV4String(wxSockAddress &addr);
