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
RCS_ID($Id: TrayIcon.cpp,v 1.18 2004-06-06 11:07:33 jason Exp $)

#include "TrayIcon.h"

#if defined(__WXMSW__)

// As of 2004/05/29, wxWidget's wxTaskBarIcon fits our needs fine
// except it doesn't support long tooltips (i.e. > 64 chars)
// So our own implementation will still be used for wxMSW.

#include <windows.h>
#include <wx/msw/winundef.h>

struct MYNOTIFYICONDATA
{
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
    wxChar szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    wxChar szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;
    } DUMMYUNIONNAME;
    wxChar szInfoTitle[64];
    DWORD dwInfoFlags;
};

static const size_t normal_len = 24 + sizeof(wxChar)*64;
static const size_t extended_len = normal_len + 16 + sizeof(wxChar)*384;

class TrayIconPrivate : public wxFrame
{

	friend class TrayIcon;

protected:
	static BOOL MyNotifyIcon(DWORD dwMessage, MYNOTIFYICONDATA *lpData)
	{
		return Shell_NotifyIcon(dwMessage, (PNOTIFYICONDATA)lpData);
	}

	TrayIconPrivate(TrayIcon *trayicon)
		: wxFrame(NULL, -1, wxT("TrayIcon Event Handler"), wxDefaultPosition, wxSize(128, 64), wxCAPTION)
	{
		m_trayicon = trayicon;
		m_TaskbarCreated = RegisterWindowMessage(wxT("TaskbarCreated"));
		SetExtraStyle(wxWS_EX_TRANSIENT);
		m_nid.cbSize = extended_len;
		m_nid.hWnd = (HWND)GetHandle();
		m_nid.uID = 1;
		m_nid.uFlags = NIF_MESSAGE;
		m_nid.uCallbackMessage = WM_USER + 1;
		m_nid.hIcon = 0;
		m_nid.szTip[0] = 0;
		m_ok = MyNotifyIcon(NIM_ADD, &m_nid) != 0;
		if (!m_ok)
		{
			m_nid.cbSize = normal_len;
			m_ok = MyNotifyIcon(NIM_ADD, &m_nid) != 0;
		}
	}

	virtual ~TrayIconPrivate()
	{
		MyNotifyIcon(NIM_DELETE, &m_nid);
	}

	virtual void SetIcon(const char **xpm)
	{
		m_nid.uFlags = NIF_ICON;
		m_icon = wxIcon(xpm);
		m_nid.hIcon = (HICON)m_icon.GetHICON();
		MyNotifyIcon(NIM_MODIFY, &m_nid);
	}

	virtual void SetToolTip(const wxString &tooltip)
	{
		m_nid.uFlags = NIF_TIP;
		size_t len = (sizeof m_nid.szTip) / (sizeof m_nid.szTip[0]);
		wxStrncpy(m_nid.szTip, tooltip.c_str(), len);
		m_nid.szTip[len-1] = 0;
		MyNotifyIcon(NIM_MODIFY, &m_nid);
	}

protected:
	void OnClose(wxCloseEvent &WXUNUSED(event))
	{
		if (m_trayicon)
		{
			m_trayicon->m_priv = NULL;
			m_trayicon = NULL;
		}
	}

	void OnMenu(wxCommandEvent &event)
	{
		if (m_trayicon->m_handler)
		{
			m_trayicon->m_handler->AddPendingEvent(event);
		}
	}

	virtual long MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
	{
		
		if (nMsg == WM_USER + 1 && m_trayicon)
		{
			
			wxASSERT(wParam == 1);

			WXTYPE type = 0;
			bool left_down = false;
			bool right_down = false;
			bool middle_down = false;

			switch (lParam)
			{
				case WM_LBUTTONDOWN:
					type = wxEVT_LEFT_DOWN;
					left_down = true;
					break;
				case WM_LBUTTONUP:
					type = wxEVT_LEFT_UP;
					left_down = false;
					break;
				case WM_LBUTTONDBLCLK:
					type = wxEVT_LEFT_DCLICK;
					left_down = true;
					break;
				case WM_RBUTTONDOWN:
					type = wxEVT_RIGHT_DOWN;
					right_down = true;
					break;
				case WM_RBUTTONUP:
					type = wxEVT_RIGHT_UP;
					right_down = false;
					break;
				case WM_RBUTTONDBLCLK:
					type = wxEVT_RIGHT_DCLICK;
					right_down = true;
					break;
				case WM_MBUTTONDOWN:
					type = wxEVT_MIDDLE_DOWN;
					middle_down = true;
					break;
				case WM_MBUTTONUP:
					type = wxEVT_MIDDLE_UP;
					middle_down = false;
					break;
				case WM_MBUTTONDBLCLK:
					type = wxEVT_MIDDLE_DCLICK;
					middle_down = true;
					break;
			}

			if (type)
			{

				wxMouseEvent evt(type);

				evt.SetId(m_trayicon->m_id);

				DWORD dwPoint = GetMessagePos();
				POINTS pt = MAKEPOINTS(dwPoint);

				evt.m_x = pt.x;
				evt.m_y = pt.y;

				evt.m_leftDown = left_down |
					((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
				evt.m_middleDown = middle_down |
					((GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0);
				evt.m_rightDown = right_down |
					((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);

				if (m_trayicon->m_handler)
				{
					m_trayicon->m_handler->AddPendingEvent(evt);
				}

			}
			return 0;
		}
		else if (nMsg == m_TaskbarCreated && m_trayicon)
		{
			m_nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
			MyNotifyIcon(NIM_ADD, &m_nid);
			return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
		}
		else
		{
			return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
		}

	}

protected:
	TrayIcon *m_trayicon;
	MYNOTIFYICONDATA m_nid;
	wxIcon m_icon;
	bool m_ok;
	UINT m_TaskbarCreated;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(TrayIconPrivate)

};

BEGIN_EVENT_TABLE(TrayIconPrivate, wxFrame)
	EVT_CLOSE(TrayIconPrivate::OnClose)
	EVT_MENU(wxID_ANY, TrayIconPrivate::OnMenu)
END_EVENT_TABLE()

TrayIcon::TrayIcon()
{
	m_priv = new TrayIconPrivate(this);
}

TrayIcon::~TrayIcon()
{
	if (m_priv)
	{
		m_priv->m_trayicon = NULL;
		m_priv->Destroy();
	}
}

void TrayIcon::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

bool TrayIcon::Ok()
{
	return m_priv && m_priv->m_ok;
}

void TrayIcon::SetIcon(const char **xpm)
{
	if (m_priv)
	{
		m_priv->SetIcon(xpm);
	}
}

void TrayIcon::SetToolTip(const wxString &tooltip)
{
	if (m_priv)
	{
		m_priv->SetToolTip(tooltip);
	}
}

bool TrayIcon::PopupMenu(wxMenu *menu, const wxPoint &pos)
{

	static bool s_inPopup = false;

	if (s_inPopup)
	{
		return false;
	}

	s_inPopup = TRUE;

	menu->UpdateUI();

	::SetForegroundWindow((HWND)m_priv->GetHWND());

	bool rval = m_priv->PopupMenu(menu, m_priv->ScreenToClient(pos));

	::PostMessage((HWND)m_priv->GetHWND(), WM_NULL, 0, 0);

	s_inPopup = FALSE;

	return rval;

}

#else

#include "wx/taskbar.h"

class TrayIconPrivate : public wxTaskBarIcon
{

public:
	TrayIconPrivate(TrayIcon *trayicon)
		: wxTaskBarIcon(), m_trayicon(trayicon)
	{
		last_set_okay = true;
	}

    void OnMouse(wxTaskBarIconEvent &event)
	{

		wxEventType src_type = event.GetEventType();
		wxEventType type;

		if (src_type == wxEVT_TASKBAR_LEFT_DOWN)
		{
			type = wxEVT_LEFT_DOWN;
		}
		else if (src_type == wxEVT_TASKBAR_LEFT_UP)
		{
			type = wxEVT_LEFT_UP;
		}
		else if (src_type == wxEVT_TASKBAR_RIGHT_DOWN)
		{
			type = wxEVT_RIGHT_DOWN;
		}
		else if (src_type == wxEVT_TASKBAR_RIGHT_UP)
		{
			type = wxEVT_RIGHT_UP;
		}
		else if (src_type == wxEVT_TASKBAR_LEFT_DCLICK)
		{
			type = wxEVT_LEFT_DCLICK;
		}
		else if (src_type == wxEVT_TASKBAR_RIGHT_DCLICK)
		{
			type = wxEVT_RIGHT_DCLICK;
		}
		else
		{
			return; // a type we wern't expecting
		}

		wxMouseEvent evt(type);

		evt.SetId(m_trayicon->m_id);

		evt.m_x = 0;
		evt.m_y = 0;

		evt.m_leftDown = false;
		evt.m_middleDown = false;
		evt.m_rightDown = false;

		if (m_trayicon->m_handler)
		{
			m_trayicon->m_handler->AddPendingEvent(evt);
		}

	}

    void OnMenu(wxCommandEvent &event)
	{
		if (m_trayicon->m_handler)
		{
			m_trayicon->m_handler->AddPendingEvent(event);
		}
	}

	void Update()
	{
		last_set_okay = SetIcon(icon, tooltip);
	}

	TrayIcon *m_trayicon;
	bool last_set_okay;
	wxIcon icon;
	wxString tooltip;

	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(TrayIconPrivate, wxTaskBarIcon)
    EVT_MENU(wxID_ANY, TrayIconPrivate::OnMenu)
	EVT_TASKBAR_LEFT_DOWN(TrayIconPrivate::OnMouse)
	EVT_TASKBAR_LEFT_UP(TrayIconPrivate::OnMouse)
	EVT_TASKBAR_RIGHT_DOWN(TrayIconPrivate::OnMouse)
	EVT_TASKBAR_RIGHT_UP(TrayIconPrivate::OnMouse)
	EVT_TASKBAR_LEFT_DCLICK(TrayIconPrivate::OnMouse)
	EVT_TASKBAR_RIGHT_DCLICK(TrayIconPrivate::OnMouse)
END_EVENT_TABLE()

TrayIcon::TrayIcon()
{
	m_priv = new TrayIconPrivate(this);
}

TrayIcon::~TrayIcon()
{
	delete m_priv;
}

void TrayIcon::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

bool TrayIcon::Ok()
{
	return m_priv->IsOk() && m_priv->last_set_okay;
}

void TrayIcon::SetIcon(const char **xpm)
{
	m_priv->icon = wxIcon(xpm);
	m_priv->Update();
}

void TrayIcon::SetToolTip(const wxString &tooltip)
{
	m_priv->tooltip = tooltip;
	m_priv->Update();
}

bool TrayIcon::PopupMenu(wxMenu *menu, const wxPoint &WXUNUSED(pos))
{
	return m_priv->PopupMenu(menu);
}

#endif
