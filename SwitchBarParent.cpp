#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarParent.cpp,v 1.22 2003-05-24 12:01:06 jason Exp $)

#include "SwitchBar.h"
#include "SwitchBarParent.h"
#include "SwitchBarChild.h"
#include "SwitchBarCanvas.h"
#include "util.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

enum
{
	ID_SWITCHBARPARENT_FIRST = wxEVT_USER_FIRST + 1000,
	ID_WINDOW_WINDOWS = ID_SWITCHBARPARENT_FIRST,
	ID_SWITCHBAR = ID_WINDOW_WINDOWS + 1000,
	ID_UPDATEWINDOWMENUTIMER,
	ID_WINDOW_MINIMIZE,
	ID_WINDOW_CLOSE,
	ID_WINDOW_CASCADE,
	ID_WINDOW_TILE,
	ID_WINDOW_ARRANGE,
	ID_WINDOW_NEXT,
	ID_WINDOW_PREV,
	ID_SWITCHBAR_RESTORE,
	ID_SWITCHBAR_MINIMIZE,
	ID_SWITCHBAR_CLOSE,
	ID_SWITCHBARPARENT_LAST
};

BEGIN_EVENT_TABLE(SwitchBarParent, wxMDIParentFrame)
	EVT_MENU_RANGE(ID_WINDOW_WINDOWS + 0, ID_WINDOW_WINDOWS + 999, SwitchBarParent::OnWindowWindows)
	EVT_CLOSE(SwitchBarParent::OnClose)
	EVT_SIZE(SwitchBarParent::OnSize)
	EVT_BUTTON(ID_SWITCHBAR, SwitchBarParent::OnSwitchBar)
	EVT_MENU(ID_SWITCHBAR, SwitchBarParent::OnSwitchBarMenu)
	EVT_TIMER(ID_UPDATEWINDOWMENUTIMER, SwitchBarParent::OnUpdateWindowMenuTimer)
	EVT_IDLE(SwitchBarParent::OnUpdateWindowMenuIdle)
	EVT_MENU(ID_WINDOW_MINIMIZE, SwitchBarParent::OnWindowMinimize)
	EVT_MENU(ID_WINDOW_CLOSE, SwitchBarParent::OnWindowClose)
	EVT_MENU(ID_WINDOW_CASCADE, SwitchBarParent::OnWindowCascade)
	EVT_MENU(ID_WINDOW_TILE, SwitchBarParent::OnWindowTile)
	EVT_MENU(ID_WINDOW_NEXT, SwitchBarParent::OnWindowNext)
	EVT_MENU(ID_WINDOW_PREV, SwitchBarParent::OnWindowPrev)
	EVT_MENU(wxID_ANY, SwitchBarParent::OnSwitchBarMenuItem)
END_EVENT_TABLE()

SwitchBarParent::SwitchBarParent(
	wxWindow *parent,
	const wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	const long style,
	const wxString& name)
	: wxMDIParentFrame(
		parent, id, title, pos, size,
		style | wxNO_FULL_REPAINT_ON_RESIZE | wxVSCROLL | wxHSCROLL | wxFRAME_NO_WINDOW_MENU, name)
{

	m_switchbar = new SwitchBar(this, ID_SWITCHBAR, wxDefaultPosition, wxDefaultSize);

	m_tmrUpdateWindowMenu = new wxTimer(this, ID_UPDATEWINDOWMENUTIMER);

	m_mnuWindow = new wxMenu;
	m_mnuWindow->Append(ID_WINDOW_MINIMIZE, wxT("Mi&nimize\tEsc"));
	m_mnuWindow->Append(ID_WINDOW_CLOSE, wxT("Cl&ose\tCtrl-F4"));
	m_mnuWindow->AppendSeparator();
	m_mnuWindow->Append(ID_WINDOW_CASCADE, wxT("&Cascade"));
	m_mnuWindow->Append(ID_WINDOW_TILE, wxT("&Tile"));
	m_mnuWindow->AppendSeparator();
	m_mnuWindow->Append(ID_WINDOW_NEXT, wxT("&Next"));
	m_mnuWindow->Append(ID_WINDOW_PREV, wxT("&Previous"));
	m_mnuWindow->AppendSeparator();
	m_num_window_menus = 0;

	m_accelerator_count = 5;
	m_accelerators = new wxAcceleratorEntry[5];
	m_accelerators[0].Set(wxACCEL_CTRL, WXK_TAB, ID_WINDOW_NEXT);
	m_accelerators[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_TAB, ID_WINDOW_PREV);
	m_accelerators[2].Set(wxACCEL_CTRL, WXK_F6, ID_WINDOW_NEXT);
	m_accelerators[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_F6, ID_WINDOW_PREV);
	m_accelerators[4].Set(0, WXK_ESCAPE, ID_WINDOW_MINIMIZE);
	SetAcceleratorTable(wxAcceleratorTable(m_accelerator_count, m_accelerators));

}

SwitchBarParent::~SwitchBarParent()
{
	delete m_tmrUpdateWindowMenu;
	m_tmrUpdateWindowMenu = NULL;
	delete[] m_accelerators;
}

void SwitchBarParent::OnClose(wxCloseEvent& event)
{
	event.Skip();
}

void SwitchBarParent::NextChild(bool bPrevious)
{

	if (m_switchbar->GetButtonCount() > 0)
	{
		
		int sel = m_switchbar->GetSelectedIndex();
		
		if (bPrevious)
		{
			sel--;
			if (sel < 0)
			{
				sel = m_switchbar->GetButtonCount() - 1;
			}
		}
		else
		{
			sel++;
			if (sel >= m_switchbar->GetButtonCount())
			{
				sel = 0;
			}
		}

		m_switchbar->SelectButton(sel);
		m_switchbar->RaiseEvent(sel, false);

	}

}

void SwitchBarParent::UpdateWindowMenu()
{
	DoUpdateWindowMenu();
	if (m_tmrUpdateWindowMenu)
	{
		m_tmrUpdateWindowMenu->Start(100, true);
	}
}

void SwitchBarParent::OnUpdateWindowMenuIdle(wxIdleEvent &event)
{
	DoUpdateWindowMenu();
}

void SwitchBarParent::OnUpdateWindowMenuTimer(wxTimerEvent &event)
{
	DoUpdateWindowMenu();
}

void SwitchBarParent::UpdateCheckMenuItem(int id, const wxString &label, bool enabled, bool checked)
{

	if ( m_mnuWindow->IsEnabled(id) != enabled )
	{
		m_mnuWindow->Enable(id, enabled);
	}
	if ( m_mnuWindow->GetLabel(id) != label )
	{
		m_mnuWindow->SetLabel(id, label);
	}
	if ( m_mnuWindow->IsChecked(id) != checked )
	{
		m_mnuWindow->Check(id, checked);
	}

}

void SwitchBarParent::DoUpdateWindowMenu()
{

	if (!m_tmrUpdateWindowMenu) return;
	if (GetMenuBar() == NULL) return;
	if (GetMenuBar()->FindItem(ID_WINDOW_CLOSE) == NULL) return;

	bool bIsActiveChild = (GetActiveChild() != NULL);
	int iNumChildren = m_switchbar->GetButtonCount();
	bool bIsChildren = (iNumChildren > 0);
	
	GetMenuBar()->Enable(ID_WINDOW_MINIMIZE, bIsActiveChild);
	GetMenuBar()->Enable(ID_WINDOW_CLOSE, bIsActiveChild);
	GetMenuBar()->Enable(ID_WINDOW_CASCADE, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_TILE, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_NEXT, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_PREV, bIsChildren);

	int iNumDesired = (iNumChildren>0) ? iNumChildren : 1;

	while (m_num_window_menus < iNumDesired)
	{
		m_mnuWindow->AppendCheckItem(ID_WINDOW_WINDOWS + m_num_window_menus, wxT("XYZ"));
		m_num_window_menus++;
	}

	while (m_num_window_menus > iNumDesired)
	{
		int id = ID_WINDOW_WINDOWS + m_num_window_menus - 1;
		wxASSERT(m_mnuWindow->FindItem(id) != NULL);
		m_mnuWindow->Destroy(id);
		m_num_window_menus--;
	}

	wxASSERT(m_num_window_menus == iNumDesired);

	if (bIsChildren)
	{

		wxASSERT(m_num_window_menus == iNumChildren);

		for (int i = 0; i < iNumChildren; ++i)
		{

			void *user_data = m_switchbar->GetUserDataFromIndex(i);
			SwitchBarCanvas *canvas = (SwitchBarCanvas*)user_data;
			wxMDIChildFrame *child = (wxMDIChildFrame*)canvas->GetParent();
			bool bIsFocused = (child == GetActiveChild());

			wxString caption = m_switchbar->GetButtonCaption(i);

			if (i < 9)
			{
				caption = wxString()
					<< wxT('&') << i+1 << wxT(' ')
					<< caption;
			}

			UpdateCheckMenuItem(
				ID_WINDOW_WINDOWS + i,
				caption,
				true,
				bIsFocused);

		}

	}
	else
	{

		wxASSERT(m_num_window_menus == 1);

		UpdateCheckMenuItem(
			ID_WINDOW_WINDOWS,
			wxT("(None)"),
			false,
			false);

	}

}

void SwitchBarParent::OnWindowMinimize(wxCommandEvent& event)
{
	if (GetActiveChild() != NULL)
	{
		int button_index = m_switchbar->GetIndexFromUserData(GetActiveCanvas());
		m_switchbar->SimulateClick(button_index);
	}
}

void SwitchBarParent::OnWindowClose(wxCommandEvent& event)
{
	wxASSERT(GetActiveChild() != NULL);
	GetActiveChild()->Close();
	UpdateWindowMenu();
}

void SwitchBarParent::OnWindowCascade(wxCommandEvent& event)
{
	Cascade();
}

void SwitchBarParent::OnWindowTile(wxCommandEvent& event)
{
	Tile();
}

void SwitchBarParent::OnWindowNext(wxCommandEvent& event)
{
	NextChild(false);
}

void SwitchBarParent::OnWindowPrev(wxCommandEvent& event)
{
	NextChild(true);
}

void SwitchBarParent::OnWindowWindows(wxCommandEvent& event)
{
	int button_index = event.GetId() - ID_WINDOW_WINDOWS;
	m_switchbar->SimulateClick(button_index);
}

SwitchBarChild* SwitchBarParent::NewWindow(SwitchBarCanvas *canvas, bool focus)
{

	wxMDIChildFrame *current_child = GetActiveChild();

	bool bMaximized = (current_child != NULL) ? current_child->IsMaximized() : true;

	wxPoint pos = wxDefaultPosition;
	wxSize size = wxDefaultSize;

	wxASSERT(canvas);

	if (canvas->m_saved_state_valid)
	{
		pos = canvas->m_saved_state_rect.GetPosition();
		size = canvas->m_saved_state_rect.GetSize();
		if (!current_child || !bMaximized)
		{
			bMaximized = canvas->m_saved_state_maximized;
		}
	}

	if (focus)
	{

		SwitchBarChild *child = OnCreateNewChild(pos, size, bMaximized, canvas);

		child->Show(TRUE);

		child->Activate();

		m_switchbar->SelectButton(m_switchbar->GetIndexFromUserData(canvas));
		
		UpdateWindowMenu();

		return child;
	
	}
	else
	{

		canvas->Show(false);

		UpdateWindowMenu();

		return NULL;

	}

}

SwitchBarChild* SwitchBarParent::OnCreateNewChild(wxPoint pos, wxSize size, bool bMaximized, SwitchBarCanvas *canvas)
{
	return new SwitchBarChild(
		this, pos, size,
		wxDEFAULT_FRAME_STYLE | (bMaximized?wxMAXIMIZE:0),
		canvas);
}

void SwitchBarParent::OnSize(wxSizeEvent& event)
{

	int w, h;
	GetClientSize(&w, &h);

	int sb_height = m_switchbar->GetSize().y;

	m_switchbar->SetSize(0, h - sb_height, w, sb_height);
	GetClientWindow()->SetSize(0, 0, w, h - sb_height);

	// The following comment is from the MDI sample in wxWindows:
	// FIXME: On wxX11, we need the MDI frame to process this
	// event, but on other platforms this should not
	// be done.
	#ifdef __WXX11__
		event.Skip();
	#endif

}

void SwitchBarParent::OnSwitchBarMenu(wxCommandEvent& event)
{

	m_switchbar_popup_button_index = event.GetExtraLong();
	m_switchbar_popup_canvas =
		(SwitchBarCanvas*)(m_switchbar->
			GetUserDataFromIndex(m_switchbar_popup_button_index));

	wxMenu menu;

	menu.Append(ID_SWITCHBAR_RESTORE, wxT("&Restore"));
	menu.Append(ID_SWITCHBAR_MINIMIZE, wxT("Mi&nimize"));
	menu.AppendSeparator();
	menu.Append(ID_SWITCHBAR_CLOSE, wxT("&Close"));

	menu.Enable(ID_SWITCHBAR_RESTORE, m_switchbar->GetSelectedIndex() != m_switchbar_popup_button_index);
	menu.Enable(ID_SWITCHBAR_MINIMIZE, m_switchbar_popup_canvas->IsAttached());
	menu.Enable(ID_SWITCHBAR_CLOSE, m_switchbar_popup_canvas->IsClosable());

	SetDefaultMenuItem(menu, menu.IsEnabled(ID_SWITCHBAR_RESTORE) ?
		ID_SWITCHBAR_RESTORE :
		ID_SWITCHBAR_MINIMIZE);

	if (m_switchbar_popup_canvas->OnPopupMenu(menu))
	{
		wxPoint pos = m_switchbar->ScreenToClient(wxGetMousePosition());
		m_switchbar->PopupMenu(&menu, pos);
	}

}

void SwitchBarParent::OnSwitchBarMenuItem(wxCommandEvent& event)
{
	if (m_switchbar_popup_canvas->OnPopupMenuItem(event))
	{
		if (event.GetId() == ID_SWITCHBAR_RESTORE)
		{
			m_switchbar->SimulateClick(m_switchbar_popup_button_index);
		}
		else if (event.GetId() == ID_SWITCHBAR_MINIMIZE)
		{
			if (m_switchbar->GetSelectedIndex() != m_switchbar_popup_button_index)
			{
				m_switchbar->SimulateClick(m_switchbar_popup_button_index);
			}
			m_switchbar->SimulateClick(m_switchbar_popup_button_index);
		}
		else if (event.GetId() == ID_SWITCHBAR_CLOSE)
		{
			CloseCanvas(m_switchbar_popup_canvas);
		}
	}
}

void SwitchBarParent::FocusCanvas(SwitchBarCanvas *canvas)
{
	int button_index = m_switchbar->GetIndexFromUserData(canvas);
	if (button_index > -1 && button_index != m_switchbar->GetSelectedIndex())
	{
		m_switchbar->SimulateClick(button_index);
	}
}

SwitchBarCanvas *SwitchBarParent::GetActiveCanvas()
{
	SwitchBarChild *child = (SwitchBarChild*)GetActiveChild();
	if (child)
	{
		return child->GetCanvas();
	}
	else
	{
		return NULL;
	}
}

void SwitchBarParent::CloseCanvas(SwitchBarCanvas *canvas)
{
	if (canvas->IsAttached())
	{
		SwitchBarChild *child = (SwitchBarChild*)canvas->GetParent();
		child->Close();
	}
	else
	{
		canvas->OnClose();
		canvas->Destroy();
	}
}

void SwitchBarParent::OnSwitchBar(wxCommandEvent& event)
{

	SwitchBarCanvas *canvas = (SwitchBarCanvas*)event.GetClientData();
	wxMDIChildFrame *child;

	if (canvas->GetParent() != this)
	{
		child = (wxMDIChildFrame*)canvas->GetParent();
	}
	else
	{
		child = NULL;
	}

	if (event.GetInt() == -1)
	{

		child->Show(false);

		#ifdef __WXMSW__

			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof (WINDOWPLACEMENT);
			::GetWindowPlacement((HWND)child->GetHandle(), &wndpl);
			canvas->m_saved_state_rect.x =
				wndpl.rcNormalPosition.left;
			canvas->m_saved_state_rect.y =
				wndpl.rcNormalPosition.top;
			canvas->m_saved_state_rect.width =
				wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
			canvas->m_saved_state_rect.height =
				wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

			canvas->m_saved_state_maximized =
				(wndpl.flags & WPF_RESTORETOMAXIMIZED) != 0;
				
		#else

			if (child->IsIconized())
			{
				child->Iconize(false);
			}
			canvas->m_saved_state_maximized = child->IsMaximized();
			if (child->IsMaximized())
			{
				child->Maximize(false);
			}
			canvas->m_saved_state_rect = child->GetRect();

		#endif

		canvas->m_saved_state_valid = true;
		canvas->Show(false);
		canvas->OnDetach();
		canvas->Reparent(this);
		if (canvas->m_saved_state_maximized)
		{
			child->Maximize(true);
		}
		child->Close(true);

	}
	else
	{

		if (child == NULL)
		{
			child = NewWindow(canvas, true);
		}
		child->Show(true);

		#ifdef __WXMSW__
			wxMDIChildFrame *org = GetActiveChild();
			while (GetActiveChild() != child)
			{
				ActivateNext();
				if (GetActiveChild() == org)
				{
					child->Activate();
					break;
				}
			}
		#else
			child->Activate();
		#endif

	}

	UpdateWindowMenu();

}
