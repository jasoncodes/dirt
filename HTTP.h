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


#ifndef HTTP_H_
#define HTTP_H_

#include <wx/socket.h>
#include "ByteBuffer.h"
#include "URL.h"
#include "util.h"

class HTTPHeader
{

public:
	HTTPHeader(const wxString &strHeader = wxEmptyString);
	~HTTPHeader();
	operator wxString() const;
	wxString GetStatusLine() const;
	bool IsValid() const;
	int GetStatusCode() const;
	int GetStatusClass() const;
	wxString GetStatusText() const;
	const StringHashMap& GetFields() const;
	const wxString GetField(const wxString &name) const;
	bool IsError() const;
	bool IsFinal() const;

protected:
	void Init();
	StringHashMap::const_iterator FindField(const wxString &name) const;
	StringHashMap::iterator FindField(const wxString &name);

protected:
	bool m_valid;
	wxString m_strHeader;
	wxString m_strStatusLine;
	int m_status_code;
	wxString m_status_text;
	StringHashMap m_fields;

};

class DNS;
class DNSEvent;

class HTTP : public wxEvtHandler
{

public:
	HTTP();
	virtual ~HTTP();

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);

	virtual void Connect(const URL &url);
	virtual URL GetURL() const;
	virtual void Close();
	virtual const HTTPHeader &GetHeader() const;
	virtual bool IsActive() const;
	virtual int GetContentReceived() const;
	virtual int GetContentLength() const;

	virtual void ResetURLSettings(); // post data, referer, extra url headers, etc
	virtual void ResetClientSettings(); // proxy, user agent, extra client headers, etc
	virtual void ResetAll(); // reset url & client settings, close any open connection

	virtual const StringHashMap& GetExtraURLHeaders() const;

	static wxString MakePostData(const StringHashMap &form_data);
	virtual void SetPostData(const ByteBuffer &post_data, const wxString &post_content_type = wxT("application/x-www-form-urlencoded"));
	virtual void SetPostData(const StringHashMap &form_data);
	virtual void SetReferer(const URL &referer);
	virtual void SetExtraURLHeaders(const StringHashMap &extra_url_headers);

	virtual const StringHashMap& GetExtraClientHeaders() const;

	virtual bool SetProxy(const URL &proxy);
	virtual void SetUserAgent(const wxString &user_agent);
	virtual void SetRedirectOkay(bool redirect_okay);
	virtual void SetMaxRedirectDepth(int max_depth);
	virtual void SetIdleTimeout(int idle_timeout_secs);
	virtual void SetExtraClientHeaders(const StringHashMap &extra_client_headers);

	static wxString GetDefaultUserAgent();

protected:
	void OnSocket(wxSocketEvent &event);
	void OnDNS(DNSEvent &event);
	void OnTimerTimeout(wxTimerEvent &event);

protected:
	virtual void ClearState();
	virtual void PrepareRequest();
	virtual void MaybeSendData();
	virtual void ProcessIncoming();
	virtual void DispatchContent(const ByteBuffer &data);

protected:
	static const ByteBuffer s_buffEmpty;
	wxEvtHandler *m_handler;
	wxEventType m_id;
	wxSocketClient *m_sck;
	DNS *m_dns;
	URL m_proxy;
	URL m_url;
	ByteBuffer m_buffOut;
	bool m_bOutputOkay;
	ByteBuffer m_buffIn;
	HTTPHeader m_header;
	ByteBuffer m_post_data;
	wxString m_post_content_type;
	URL m_referer;
	wxString m_user_agent;
	StringHashMap m_extra_url_headers;
	StringHashMap m_extra_client_headers;
	bool m_redirect_okay;
	int m_depth;
	int m_max_depth;
	bool m_active;
	wxLongLong_t m_last_active;
	wxTimer *m_tmrTimeout;
	int m_idle_timeout_secs;
	long m_content_length;
	long m_content_received;
	bool m_chunked;
	long m_chunk_remaining;
	bool m_chunk_crlf_left;
	bool m_transfer_complete;
	bool m_connect_ok;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(HTTP)

};

extern const wxEventType wxEVT_HTTP;

enum HTTPNotify
{
	HTTP_CONNECTION,
	HTTP_LOST,
	HTTP_COMPLETE,
	HTTP_HEADER,
	HTTP_DATA
};

class HTTPEvent : public wxEvent
{

public:
	HTTPEvent(int id, HTTPNotify event, HTTP *src)
		: wxEvent(id, wxEVT_HTTP), m_data()
	{
		wxASSERT(event != HTTP_DATA && event != HTTP_HEADER);
		m_event = event;
		SetEventObject(src);
	}

	HTTPEvent(int id, HTTPNotify event, HTTP *src, const ByteBuffer &data)
		: wxEvent(id, wxEVT_HTTP), m_data(data)
	{
		wxASSERT(event == HTTP_DATA);
		m_event = event;
		SetEventObject(src);
	}

	HTTPEvent(int id, HTTPNotify event, HTTP *src, const HTTPHeader &header, bool is_redirecting)
		: wxEvent(id, wxEVT_HTTP), m_data(), m_header(header), m_is_redirecting(is_redirecting)
	{
		wxASSERT(event == HTTP_HEADER);
		m_event = event;
		SetEventObject(src);
	}

	virtual ~HTTPEvent()
	{
	}

	virtual HTTPNotify GetHTTPEvent() const
	{
		return m_event;
	}

	virtual HTTP *GetHTTP() const
	{
		return (HTTP*)GetEventObject();
	}

	virtual const ByteBuffer& GetData() const
	{
		return m_data;
	}

	virtual const HTTPHeader& GetHeader() const
	{
		return m_header;
	}

	virtual bool IsRedirecting() const
	{
		return m_is_redirecting;
	}

	virtual wxEvent *Clone() const
	{
		return new HTTPEvent(*this);
	}

protected:
	HTTPNotify m_event;
	ByteBuffer m_data;
	HTTPHeader m_header;
	bool m_is_redirecting;
	void *m_userdata;

};

typedef void (wxEvtHandler::*HTTPEventFunction)(HTTPEvent&);

#define EVT_HTTP(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_HTTP, id, -1, \
		(wxObjectEventFunction) \
		wxStaticCastEvent( HTTPEventFunction, &func ), \
		(wxObject *) NULL ),

#endif
