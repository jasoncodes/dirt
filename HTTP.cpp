#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: HTTP.cpp,v 1.6 2003-03-05 01:29:46 jason Exp $)

#include "HTTP.h"
#include "util.h"
#include "Crypt.h"

HTTPHeader::HTTPHeader(const wxString &strHeader)
	: m_strHeader(strHeader)
{
	Init();
}

HTTPHeader::~HTTPHeader()
{
}

HTTPHeader::operator wxString() const
{
	return m_strHeader;
}

wxString HTTPHeader::GetStatusLine() const
{
	return m_strStatusLine;
}

bool HTTPHeader::IsValid() const
{
	return m_valid;
}

int HTTPHeader::GetStatusCode() const
{
	return m_status_code;
}

int HTTPHeader::GetStatusClass() const
{
	return ((m_status_code / 100) * 100);
}

wxString HTTPHeader::GetStatusText() const
{
	return m_status_text;
}

const StringHashMap& HTTPHeader::GetFields() const
{
	return m_fields;
}

bool HTTPHeader::IsError() const
{

	switch (GetStatusClass())
	{

		case 100: // Informational
		case 200: // Successful
		case 300: // Redirection
			return false;

		case 400: // Client Error
		case 500: // Server Error
		default:  // Unknown
			return true;

	}

}

bool HTTPHeader::IsFinal() const
{

	switch (GetStatusClass())
	{

		case 100: // Informational
			return (GetStatusCode() != 100);

		case 200: // Successful
			return true;

		case 300: // Redirection
			return false;

		case 400: // Client Error
		case 500: // Server Error
		default:  // Unknown
			return true;

	}

}

const wxString HTTPHeader::GetField(const wxString &name) const
{
	for (StringHashMap::const_iterator i = m_fields.begin(); i != m_fields.end(); ++i)
	{
		if (name.CmpNoCase(i->first) == 0)
		{
			return i->second;
		}
	}
	return wxEmptyString;
}

void HTTPHeader::Init()
{
	int i = m_strHeader.Find(wxT("\r\n"));
	if (i == -1 || !LeftEq(m_strHeader, "HTTP/"))
	{
		m_strStatusLine = wxEmptyString;
		m_status_code = 0;
		m_status_text = wxEmptyString;
		m_fields.clear();
		m_valid = false;
	}
	else
	{
		m_strStatusLine = m_strHeader.Left(i);
		wxArrayString lines = SplitString(m_strHeader.Mid(i+2), wxT("\r\n"));
		i = m_strStatusLine.Find(wxT(' '));
		wxString tmp = m_strStatusLine.Mid(i+1);
		i = tmp.Find(wxT(' '));
		m_status_text = tmp.Mid(i+1);
		tmp = tmp.Left(i);
		unsigned long x;
		if (tmp.ToULong(&x))
		{
			m_status_code = x;
		}
		else
		{
			m_status_code = -1;
			m_valid = false;
			return;
		}
		m_fields.clear();
		size_t pos = 0;
		wxString line;
		while (pos < lines.GetCount())
		{
			line = lines[pos];
			while (
				pos+1 < lines.GetCount() &&
				lines[pos+1].Length() > 0 &&
				wxIsspace(lines[pos+1][0]))
			{
				pos++;
				line += lines[pos];
			}
			HeadTail ht = SplitHeadTail(line, wxT(":"));
			ht.head.Trim(false);
			ht.head.Trim(true);
			ht.tail.Trim(false);
			ht.tail.Trim(true);
			StringHashMap::iterator it = m_fields.find(ht.head);
			if (it == m_fields.end())
			{
				m_fields[ht.head] = ht.tail;
			}
			else
			{
				m_fields[ht.head] += wxT(' ') + ht.tail;
			}
			pos++;
		}
		m_valid = true;
	}
}

const wxEventType wxEVT_HTTP = wxNewEventType();
const ByteBuffer HTTP::s_buffEmpty = ByteBuffer();

enum
{
	ID_SOCKET = 1,
	ID_TIMER_TIMEOUT
};

BEGIN_EVENT_TABLE(HTTP, wxEvtHandler)
	EVT_SOCKET(ID_SOCKET, HTTP::OnSocket)
	EVT_TIMER(ID_TIMER_TIMEOUT, HTTP::OnTimerTimeout)
END_EVENT_TABLE()

HTTP::HTTP()
{
	
	m_handler = NULL;
	m_id = -1;
	
	m_sck = new wxSocketClient(wxSOCKET_NOWAIT);
	m_sck->SetEventHandler(*this, ID_SOCKET);
	m_sck->SetNotify(
		wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG |
		wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
	m_sck->Notify(true);

	m_tmrTimeout = new wxTimer(this, ID_TIMER_TIMEOUT);

	ResetAll();

}

HTTP::~HTTP()
{
	ResetAll();
	delete m_sck;
	delete m_tmrTimeout;
}

void HTTP::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

void HTTP::OnTimerTimeout(wxTimerEvent &event)
{
	wxLongLong_t idle_time = GetMillisecondTicks() - m_last_active;
	if (idle_time >= m_idle_timeout_secs)
	{
		wxSocketEvent evt(ID_SOCKET);
		evt.m_event = wxSOCKET_LOST;
		AddPendingEvent(evt);
	}
}

void HTTP::OnSocket(wxSocketEvent &event)
{

	switch (event.GetSocketEvent())
	{

		case wxSOCKET_INPUT:
			if (m_active)
			{
				m_last_active = GetMillisecondTicks();
				ByteBuffer buff(4096);
				m_sck->Read(buff.LockReadWrite(), buff.Length());
				buff.Unlock();
				wxCHECK_RET(!m_sck->Error(), wxT("Socket error has occured"));
				
				if (m_sck->LastCount())
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
					ProcessIncoming();
				}
			}
			break;

		case wxSOCKET_OUTPUT:
			m_bOutputOkay = true;
			MaybeSendData();
			break;

		case wxSOCKET_CONNECTION:
			m_connect_ok = true;
			m_active = true;
			m_tmrTimeout->Start(1000);
			m_last_active = GetMillisecondTicks();
			ClearState();
			PrepareRequest();
			if (m_handler)
			{
				HTTPEvent evt(m_id, HTTP_CONNECTION, this);
				m_handler->AddPendingEvent(evt);
			}
			break;

		case wxSOCKET_LOST:
			if (m_connect_ok)
			{
				if (m_content_length > -1)
				{
					m_transfer_complete = (m_content_received == m_content_length);
				}
				else if (!m_chunked)
				{
					// cannot tell if complete or not as no content-length and
					// no chunked encoding. will have to hope all went well.
					m_transfer_complete = true;
				}
			}
			else
			{
				m_transfer_complete = false;
			}
			Close();
			if (m_handler)
			{
				HTTPEvent evt(m_id, m_transfer_complete?HTTP_COMPLETE:HTTP_LOST, this);
				m_handler->AddPendingEvent(evt);
			}
			break;

		default:
			wxFAIL_MSG(wxT("Unknown event type in HTTP::OnSocket"));
			break;

	}

}

bool HTTP::SetProxy(const URL &proxy)
{
	if (((wxString)proxy).Length() == 0)
	{
		m_proxy = proxy;
		return true;
	}
	else if (
		proxy.GetPath() == wxT("/") &&
		proxy.GetQuery().Length() == 0 &&
		proxy.GetReference().Length() == 0 &&
		proxy.GetHostname().Length() > 0 &&
		(proxy.GetProtocol().Length() == 0 || proxy.GetProtocol().Lower() == wxT("http")))
	{
		m_proxy = proxy;
		return true;
	}
	else
	{
		return false;
	}
}

void HTTP::DispatchContent(const ByteBuffer &data)
{
	m_content_received += data.Length();
	if (m_handler && data.Length())
	{
		HTTPEvent evt(m_id, HTTP_DATA, this, data);
		m_handler->AddPendingEvent(evt);
	}
}

void HTTP::ProcessIncoming()
{
	
	if (m_buffIn.Length() == 0) return;
	
	if (m_header.IsValid())
	{
		
		if (m_chunked)
		{
			
			if (!m_transfer_complete)
			{

				if (m_chunk_remaining > 0)
				{
					size_t data_len = wxMin((size_t)m_chunk_remaining, m_buffIn.Length());
					const byte *ptr = m_buffIn.LockRead();
					DispatchContent(ByteBuffer(ptr, data_len));
					ByteBuffer rest(ptr+data_len, m_buffIn.Length()-data_len);
					m_buffIn.Unlock();
					m_chunk_remaining -= data_len;
					m_buffIn = rest;
					if (m_chunk_remaining == 0)
					{
						m_chunk_crlf_left = true;
					}
				}

				if (m_chunk_remaining == 0 && !m_transfer_complete)
				{

					if (m_chunk_crlf_left && m_buffIn.Length() >= 2)
					{
						const byte *ptr = m_buffIn.LockRead();
						if (ptr[0] == '\r' && ptr[1] == '\n')
						{
							ByteBuffer rest(ptr+2, m_buffIn.Length()-2);
							m_buffIn.Unlock();
							m_buffIn = rest;
							m_chunk_crlf_left = false;
						}
						else
						{
							m_buffIn.Unlock();
							wxSocketEvent evt(ID_SOCKET);
							evt.m_event = wxSOCKET_LOST;
							AddPendingEvent(evt);
							return;
						}
					}

					if (!m_chunk_crlf_left && m_buffIn.Length() > 0)
					{
						const byte *ptr = m_buffIn.LockRead();
						const byte *pos = findbytes(ptr, m_buffIn.Length(), (const byte*)"\r\n", 2);
						if (pos)
						{
							size_t len = pos-ptr;
							wxString header = ByteBuffer(ptr, len);
							ByteBuffer rest(pos+2, m_buffIn.Length()-len-2);
							m_buffIn.Unlock();
							m_buffIn = rest;
							int i = header.Find(wxT(';'));
							if (i > -1)
							{
								header = header.Left(i);
							}
							if (!header.ToLong(&m_chunk_remaining, 16) || m_chunk_remaining < 0)
							{
								wxSocketEvent evt(ID_SOCKET);
								evt.m_event = wxSOCKET_LOST;
								AddPendingEvent(evt);
								return;
							}
							if (m_chunk_remaining == 0)
							{
								m_transfer_complete = true;
								m_content_length = m_content_received;
								return;
							}
							else
							{
								ProcessIncoming();
							}
						}
						else
						{
							m_buffIn.Unlock();
						}
					}

				}

			}

		}
		else
		{

			DispatchContent(m_buffIn);
			m_buffIn = s_buffEmpty;

		}

	}
	else
	{

		const byte *ptr = m_buffIn.LockRead();
		const byte *pos = findbytes(ptr, m_buffIn.Length(), (const byte*)"\r\n\r\n", 4);
		if (pos)
		{
			
			size_t header_len = pos-ptr+4;
			size_t data_len = m_buffIn.Length() - header_len;
			m_header = wxString(ByteBuffer(ptr, header_len));
			ByteBuffer tmp(pos + 4, data_len);
			m_buffIn.Unlock();
			m_buffIn = tmp;

			wxString len = m_header.GetField(wxT("Content-Length"));
			if (!len.ToLong(&m_content_length))
			{
				m_content_length = -1;
			}
			
			wxString transfer_enc = m_header.GetField(wxT("Transfer-Encoding"));
			m_chunked = (transfer_enc.Lower().Find(wxT("chunked")) > -1);

			if (m_header.GetStatusCode() == 100)
			{
				m_header = HTTPHeader();
			}
			else
			{
				URL dest_url;
				bool is_redirecting = false;
				if (!m_header.IsFinal() && m_redirect_okay)
				{
					wxString location = m_header.GetField(wxT("Location"));
					m_depth++;
					if (location.Length()&& m_depth <= m_max_depth)
					{

						bool keep_post_data;
						switch (m_header.GetStatusCode())
						{
							case 300: case 301:
							case 302: case 305:
								keep_post_data = true;
								break;
							default:
								keep_post_data = false;
								break;
						}

						if (!keep_post_data)
						{
							SetPostData(ByteBuffer());
						}

						if (m_header.GetStatusCode() == 305)
						{
							dest_url = m_url;
							SetProxy(location);
						}
						else
						{
							ResetURLSettings();
							SetReferer(m_url);
							dest_url = URL(m_url, location);
						}
						is_redirecting = true;
						
					}
				}
				if (m_handler)
				{
					HTTPEvent evt(m_id, HTTP_HEADER, this, m_header, is_redirecting);
					m_handler->AddPendingEvent(evt);
				}
				if (is_redirecting)
				{
					int depth = m_depth;
					Connect(dest_url);
					m_depth = depth;
				}
			}
			ProcessIncoming();
		}
		else
		{
			m_buffIn.Unlock();
		}

	}

}

void HTTP::MaybeSendData()
{

	if (!m_bOutputOkay && m_sck && m_sck->IsConnected()) return;
	
	const byte *ptr = m_buffOut.LockRead();
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
			wxCHECK_RET(!m_sck->Error(), wxT("Socket error has occured"));
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

bool HTTP::IsActive() const
{
	return m_active;
}

int HTTP::GetContentReceived() const
{
	return m_content_received;
}

int HTTP::GetContentLength() const
{
	return m_content_length;
}

void HTTP::ResetAll()
{
	Close();
	ResetURLSettings();
	ResetClientSettings();
}

void HTTP::ResetURLSettings()
{
	SetUserAgent(wxString() << GetProductVersion() << wxT(" (") << GetRCSDate() << wxT("; ") << wxGetOsDescription() << wxT(")"));
	SetProxy(URL());
	m_extra_url_headers.clear();
}

void HTTP::ResetClientSettings()
{
	SetReferer(URL());
	SetPostData(ByteBuffer());
	m_redirect_okay = true;
	m_max_depth = 5;
	m_idle_timeout_secs = 60;
	m_extra_client_headers.clear();
	m_content_length = -1;
	m_content_received = 0;
	m_chunked = false;
	m_chunk_remaining = 0;
	m_chunk_crlf_left = false;
	m_transfer_complete = false;
}

void HTTP::SetUserAgent(const wxString &user_agent)
{
	m_user_agent = user_agent;
}

void HTTP::SetRedirectOkay(bool redirect_okay)
{
	m_redirect_okay = redirect_okay;
}

void HTTP::SetMaxRedirectDepth(int max_depth)
{
	m_max_depth = max_depth;
}

void HTTP::SetIdleTimeout(int idle_timeout_secs)
{
	m_idle_timeout_secs = idle_timeout_secs;
}

void HTTP::SetPostData(const ByteBuffer &post_data, const wxString &post_content_type)
{
	m_post_data = post_data;
	m_post_content_type = post_content_type;
}

void HTTP::SetPostData(const StringHashMap &form_data)
{
	wxString post_data;
	for (StringHashMap::const_iterator i = form_data.begin(); i != form_data.end(); ++i)
	{
		if (post_data.Length())
		{
			post_data << wxT('&');
		}
		post_data << URL::Escape(i->first) << wxT('=') << URL::Escape(i->second);
	}
	SetPostData(post_data);
}

void HTTP::SetReferer(const URL &referer)
{
	m_referer = referer;
	if (m_referer.GetProtocol().Length() == 0)
	{
		m_referer.SetProtocol(wxT("http"));
	}
	m_referer.SetAuthentication(wxEmptyString);
}

URL HTTP::GetURL() const
{
	return m_url;
}

const StringHashMap& HTTP::GetExtraURLHeaders() const
{
	return m_extra_url_headers;
}

const StringHashMap& HTTP::GetExtraClientHeaders() const
{
	return m_extra_client_headers;
}

void HTTP::SetExtraURLHeaders(const StringHashMap &extra_url_headers)
{
	m_extra_url_headers = extra_url_headers;
}

void HTTP::SetExtraClientHeaders(const StringHashMap &extra_client_headers)
{
	m_extra_client_headers = extra_client_headers;
}

void HTTP::PrepareRequest()
{
	const wxString CRLF = wxT("\r\n");
	wxString strHeader;
	strHeader << (m_post_data.Length()?wxT("POST "):wxT("GET "));
	strHeader << m_url << wxT(" HTTP/1.1") << CRLF;
	strHeader << wxT("Host: ") << m_url.GetHostname();
	if (m_url.GetPort() > 0)
	{
		strHeader << wxT(':') << m_url.GetPort();
	}
	strHeader << CRLF;
	if (m_proxy.GetAuthentication().Length() > 0)
	{
		strHeader << wxT("Proxy-Authorization: Basic ") << Crypt::Base64Encode(m_proxy.GetAuthentication(), false) << CRLF;
	}
	if (m_url.GetAuthentication().Length() > 0)
	{
		strHeader << wxT("Authorization: Basic ") << Crypt::Base64Encode(m_url.GetAuthentication(), false) << CRLF;
	}
	strHeader << wxT("Connection: close") << CRLF;
	if (m_user_agent.Length())
	{
		strHeader << wxT("User-Agent: ") << m_user_agent << CRLF;
	}
	if (((wxString)m_referer).Length())
	{
		strHeader << wxT("Referer: ") << m_referer << CRLF;
	}
	if (m_post_data.Length())
	{
		if (m_post_content_type.Length())
		{
			strHeader << wxT("Content-Type: ") << m_post_content_type << CRLF;
		}
		strHeader << wxT("Content-Length: ") << m_post_data.Length() << CRLF;
	}
	for (StringHashMap::iterator i = m_extra_url_headers.begin(); i != m_extra_url_headers.end(); ++i)
	{
		if (i->first.Length() && i->second.Length())
		{
			strHeader << i->first << ": " << i->second << CRLF;
		}
	}
	for (StringHashMap::iterator i = m_extra_client_headers.begin(); i != m_extra_client_headers.end(); ++i)
	{
		if (i->first.Length() && i->second.Length())
		{
			strHeader << i->first << ": " << i->second << CRLF;
		}
	}
	strHeader << CRLF;
	m_buffOut = strHeader;
	if (m_post_data.Length())
	{
		m_buffOut += m_post_data;
	}
}

void HTTP::Close()
{
	m_tmrTimeout->Stop();
	m_sck->Close();
	m_active = false;
	m_connect_ok = false;
	ClearState();
}

void HTTP::ClearState()
{
	m_buffOut = s_buffEmpty;
	m_buffIn = s_buffEmpty;
	m_header = HTTPHeader();
	m_bOutputOkay = false;
}

void HTTP::Connect(const URL &url)
{
	Close();
	m_url = url;
	m_depth = 1;
	if (m_url.GetProtocol().Length() == 0)
	{
		m_url.SetProtocol(wxT("http"));
	}
	if (m_url.GetProtocol() != wxT("http"))
	{
		wxSocketEvent evt(ID_SOCKET);
		evt.m_event = wxSOCKET_LOST;
		AddPendingEvent(evt);
		return;
	}
	wxIPV4address addr;
	bool valid;
	if (((wxString)m_proxy).Length() > 0)
	{
		valid = addr.Hostname(m_proxy.GetHostname());
		valid &= addr.Service(m_proxy.GetPort(80));
	}
	else
	{
		valid = addr.Hostname(m_url.GetHostname());
		valid &= addr.Service(m_url.GetPort(80));
	}
	if (!valid)
	{
		wxSocketEvent evt(ID_SOCKET);
		evt.m_event = wxSOCKET_LOST;
		AddPendingEvent(evt);
	}
	else if (m_sck->Connect(addr, false))
	{
		wxSocketEvent evt(ID_SOCKET);
		evt.m_event = wxSOCKET_CONNECTION;
		AddPendingEvent(evt);
	}
}

const HTTPHeader& HTTP::GetHeader() const
{
	return m_header;
}
