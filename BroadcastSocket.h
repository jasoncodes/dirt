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

	bool Ok() const;
	wxUint16 GetPort() const { return m_port; }

	void SendAll(wxUint16 port, const ByteBuffer &data);
	void Send(const wxString &ip, wxUint16 port, const ByteBuffer &data);

protected:
	virtual void CheckForIPs();

protected:
	void OnIPCheck(wxTimerEvent &event);
	int Find(const wxString &IP);

protected:
	wxTimer *m_tmrIPCheck;
	IPInfoEntryArray m_entries;
	BroadcastSocketDataArray m_data;
	bool m_ok;
	wxUint16 m_port;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(BroadcastSocket)

};

#endif
