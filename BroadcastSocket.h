#ifndef BroadcastSocket_H_
#define BroadcastSocket_H_

class BroadcastSocketData;

#include <wx/dynarray.h>
WX_DEFINE_ARRAY(BroadcastSocketData*, BroadcastSocketDataArray);

#include "ByteBuffer.h"
#include "IPInfo.h"

class BroadcastSocket : public wxEvtHandler
{

public:
	BroadcastSocket(wxUint16 port = 0);
	virtual ~BroadcastSocket();

	virtual bool Ok() const;
	virtual wxUint16 GetPort() const { return m_port; }

	virtual void SendAll(wxUint16 port, const ByteBuffer &data);
	virtual void Send(const wxString &ip, wxUint16 port, const ByteBuffer &data);

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);

protected:
	virtual void CheckForIPs();
	virtual int Find(const wxString &IP);

protected:
	void OnIPCheck(wxTimerEvent &event);
	void OnSocket(wxSocketEvent &event);

protected:
	wxEvtHandler *m_handler;
	wxEventType m_id;
	wxTimer *m_tmrIPCheck;
	IPInfoEntryArray m_entries;
	BroadcastSocketDataArray m_data;
	bool m_ok;
	wxUint16 m_port;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(BroadcastSocket)

};

extern const wxEventType wxEVT_BROADCAST_SOCKET;

class BroadcastSocketEvent : public wxEvent
{

public:
	BroadcastSocketEvent(int id, BroadcastSocket *src, const wxString &ip, wxUint16 port, const ByteBuffer &data)
		: wxEvent(id, wxEVT_BROADCAST_SOCKET), m_ip(ip), m_port(port), m_data(data)
	{
		SetEventObject(src);
	}

	BroadcastSocketEvent(const BroadcastSocketEvent &evt)
		: wxEvent(evt.GetId(), wxEVT_BROADCAST_SOCKET), m_ip(evt.m_ip), m_port(evt.m_port), m_data(evt.m_data)
	{
		SetEventObject(evt.GetEventObject());
	}

	virtual ~BroadcastSocketEvent()
	{
	}

	virtual wxEvent *Clone() const
	{
		return new BroadcastSocketEvent(*this);
	}

	inline wxString GetIP() const { return m_ip; }
	inline wxUint16 GetPort() const { return m_port; }
	inline ByteBuffer GetData() const { return m_data; }

protected:
	wxString m_ip;
	wxUint16 m_port;
	ByteBuffer m_data;

};

typedef void (wxEvtHandler::*BroadcastSocketEventFunction)(BroadcastSocketEvent&);

#define EVT_BROADCAST_SOCKET(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_BROADCAST_SOCKET, id, -1, \
		(wxObjectEventFunction) \
		(wxEventFunction) \
		(BroadcastSocketEventFunction) & func, \
		(wxObject *) NULL ),

#endif
