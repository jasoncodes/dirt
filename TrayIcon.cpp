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
RCS_ID($Id: TrayIcon.cpp,v 1.14 2004-05-16 04:42:49 jason Exp $)

#include "TrayIcon.h"

#if defined(WIN32)

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

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

static size_t normal_len = 24 + sizeof(wxChar)*64;
static size_t extended_len = normal_len + 16 + sizeof(wxChar)*384;

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

#elif defined(KDE_AVAILABLE)

#include "KDE.h"
#include <kapplication.h>
#include <ksystemtray.h>
#include <qimage.h>
#include <qtooltip.h>
#include <netwm.h>
#include "util.h"

class TrayIconPrivate : public KSystemTray
{

public:
	TrayIconPrivate(TrayIcon *trayicon)
		: KSystemTray(), m_trayicon(trayicon)
	{

		kapp->lock();
		setAlignment(Qt::AlignTop|Qt::AlignLeft);
		setGeometry(-128,-128,64,64);
		NETRootInfo nri_before(qt_xdisplay(), NET::KDESystemTrayWindows);
		nri_before.activate();
		int before = nri_before.kdeSystemTrayWindowsCount();
		show();
		bool success = false;
		wxLongLong_t stop_time = GetMillisecondTicks() + 1000;
		while (!success && GetMillisecondTicks() < stop_time)
		{
			NETRootInfo nri_after(qt_xdisplay(), NET::KDESystemTrayWindows);
			nri_after.activate();
			int after = nri_after.kdeSystemTrayWindowsCount();
			if (after > before)
			{
				success = true;
				break;
			}
			wxThread::Sleep(50);
		}

		if (success)
		{
			update();
			repaint();
		}
		else
		{
			hide();
		}
		kapp->unlock();

	}

	virtual ~TrayIconPrivate()
	{
	}

protected:
	void mousePressEvent(QMouseEvent *e)
	{
		OnMouseEvent(e);
	}

	void mouseReleaseEvent(QMouseEvent *e)
	{
		OnMouseEvent(e);
	}

	void OnMouseEvent(QMouseEvent *e)
	{
		WXTYPE type = 0;
		switch (e->button())
		{
			case Qt::LeftButton:
				switch (e->type())
				{
					case QEvent::MouseButtonPress:
						type = wxEVT_LEFT_DOWN;
						break;
					case QEvent::MouseButtonRelease:
						type = wxEVT_LEFT_UP;
						break;
					case QEvent::MouseButtonDblClick:
						type = wxEVT_LEFT_DCLICK;
						break;
				}
				break;
			case Qt::MidButton:
				switch (e->type())
				{
					case QEvent::MouseButtonPress:
						type = wxEVT_MIDDLE_DOWN;
						break;
					case QEvent::MouseButtonRelease:
						type = wxEVT_MIDDLE_UP;
						break;
					case QEvent::MouseButtonDblClick:
						type = wxEVT_MIDDLE_DCLICK;
						break;
				}
				break;
			case Qt::RightButton:
				switch (e->type())
				{
					case QEvent::MouseButtonPress:
						type = wxEVT_RIGHT_DOWN;
						break;
					case QEvent::MouseButtonRelease:
						type = wxEVT_RIGHT_UP;
						break;
					case QEvent::MouseButtonDblClick:
						type = wxEVT_RIGHT_DCLICK;
						break;
				}
				break;
		}

		if (type)
		{

			wxMouseEvent evt(type);

			evt.SetId(m_trayicon->m_id);

			evt.m_x = e->globalX();
			evt.m_y = e->globalY();

			evt.m_leftDown = (e->stateAfter()&Qt::LeftButton);
			evt.m_middleDown = (e->stateAfter()&Qt::MidButton);
			evt.m_rightDown = (e->stateAfter()&Qt::RightButton);

			if (m_trayicon->m_handler)
			{
				m_trayicon->m_handler->AddPendingEvent(evt);
			}

		}

	}

protected:
	TrayIcon *m_trayicon;

};

TrayIcon::TrayIcon()
	: wxEvtHandler()
{
	m_handler = NULL;
	m_id = -1;
	if (DoesDCOPFileExist())
	{
		KDEThread::Init();
		m_priv = new TrayIconPrivate(this);
	}
	else
	{
		m_priv = NULL;
	}
}

TrayIcon::~TrayIcon()
{
	if (m_priv)
	{
		m_priv->deleteLater();
	}
}

void TrayIcon::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

bool TrayIcon::Ok()
{
	return m_priv && m_priv->winId() && m_priv->isVisible();
}

void TrayIcon::SetIcon(const char **xpm)
{
	if (m_priv)
	{
		kapp->lock();
		QImage img(xpm);
		QPixmap pixmap(img.smoothScale(24, 24));
		m_priv->setPixmap(pixmap);
		m_priv->update();
		kapp->processEvents();
		kapp->unlock();
	}
}

void TrayIcon::SetToolTip(const wxString &tooltip)
{
	if (m_priv)
	{
		kapp->lock();
		#if wxUSE_UNICODE
			QToolTip::add(m_priv, QString((const QChar*)tooltip.c_str(), tooltip.Length()));
		#else
			QToolTip::add(m_priv, tooltip.c_str());
		#endif
		kapp->unlock();
	}
}

class TrayIconPopupHandler : public wxEvtHandler
{

public:
	TrayIconPopupHandler(wxEvtHandler *handler)
		: wxEvtHandler(), m_handler(handler)
	{
	}

protected:
	void OnMenu(wxCommandEvent &event)
	{
		if (m_handler)
		{
			m_handler->AddPendingEvent(event);
		}
	}

protected:
	wxEvtHandler *m_handler;

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(TrayIconPopupHandler, wxEvtHandler)
	EVT_MENU(wxID_ANY, TrayIconPopupHandler::OnMenu)
END_EVENT_TABLE()

bool TrayIcon::PopupMenu(wxMenu *menu, const wxPoint &pos)
{
	TrayIconPopupHandler evt(m_handler);
	wxFrame *frm = new wxFrame(NULL, wxID_ANY, wxEmptyString);
	frm->PushEventHandler(&evt);
	wxTopLevelWindows.DeleteObject(frm);
	menu->UpdateUI();
	bool result = frm->PopupMenu(menu, frm->ScreenToClient(pos));
	frm->PopEventHandler(false);
	frm->Destroy();
	return result;
}

#else

TrayIcon::TrayIcon()
{
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

bool TrayIcon::Ok()
{
	return false;
}

void TrayIcon::SetIcon(const char **xpm)
{
}

void TrayIcon::SetToolTip(const wxString &tooltip)
{
}

bool TrayIcon::PopupMenu(wxMenu *menu, const wxPoint &pos)
{
	return false;
}

#endif
