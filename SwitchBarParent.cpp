#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "SwitchBarParent.h"
#include "SwitchBarChild.h"

enum
{
	ID_SWITCHBARPARENT_FIRST = wxEVT_USER_FIRST + 1000,
	ID_WINDOW_WINDOWS = ID_SWITCHBARPARENT_FIRST,
	ID_SWITCHBAR = ID_WINDOW_WINDOWS + 1000,
	ID_UPDATEWINDOWMENUTIMER,
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
	EVT_MENU(ID_WINDOW_CLOSE, SwitchBarParent::OnWindowClose)
	EVT_MENU(ID_WINDOW_CASCADE, SwitchBarParent::OnWindowCascade)
	EVT_MENU(ID_WINDOW_TILE, SwitchBarParent::OnWindowTile)
	EVT_MENU(ID_WINDOW_NEXT, SwitchBarParent::OnWindowNext)
	EVT_MENU(ID_WINDOW_PREV, SwitchBarParent::OnWindowPrev)
	EVT_MENU(ID_SWITCHBAR_RESTORE, SwitchBarParent::OnSwitchBarRestore)
	EVT_MENU(ID_SWITCHBAR_MINIMIZE, SwitchBarParent::OnSwitchBarMinimize)
	EVT_MENU(ID_SWITCHBAR_CLOSE, SwitchBarParent::OnSwitchBarClose)
END_EVENT_TABLE()

SwitchBarParent::SwitchBarParent(
	wxWindow *parent,
	const wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	const long style)
	: wxMDIParentFrame(
		parent, id, title, pos, size,
		style | wxNO_FULL_REPAINT_ON_RESIZE | wxVSCROLL | wxHSCROLL | wxFRAME_NO_WINDOW_MENU)
{

	m_switchbar = new SwitchBar(this, ID_SWITCHBAR, wxDefaultPosition, wxDefaultSize);

	tmrUpdateWindowMenu = new wxTimer(this, ID_UPDATEWINDOWMENUTIMER);

	mnuWindow = new wxMenu;
	mnuWindow->Append(ID_WINDOW_CLOSE, "Cl&ose\tCtrl-F4");
	mnuWindow->AppendSeparator();
	mnuWindow->Append(ID_WINDOW_CASCADE, "&Cascade");
	mnuWindow->Append(ID_WINDOW_TILE, "&Tile");
	mnuWindow->AppendSeparator();
	mnuWindow->Append(ID_WINDOW_NEXT, "&Next");
	mnuWindow->Append(ID_WINDOW_PREV, "&Previous");
	mnuWindow->AppendSeparator();
	num_window_menus = 0;

	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_CTRL, WXK_TAB, ID_WINDOW_NEXT);
	entries[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_TAB, ID_WINDOW_PREV);
	entries[2].Set(wxACCEL_CTRL, WXK_F6, ID_WINDOW_NEXT);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_F6, ID_WINDOW_PREV);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);

}

SwitchBarParent::~SwitchBarParent()
{
	delete tmrUpdateWindowMenu;
	tmrUpdateWindowMenu = NULL;
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
	if (tmrUpdateWindowMenu)
	{
		tmrUpdateWindowMenu->Start(100, true);
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

	if ( mnuWindow->IsEnabled(id) != enabled )
	{
		mnuWindow->Enable(id, enabled);
	}
	if ( mnuWindow->GetLabel(id) != label )
	{
		mnuWindow->SetLabel(id, label);
	}
	if ( mnuWindow->IsChecked(id) != checked )
	{
		mnuWindow->Check(id, checked);
	}

}

void SwitchBarParent::DoUpdateWindowMenu()
{

	if (!tmrUpdateWindowMenu) return;
	if (GetMenuBar() == NULL) return;
	if (GetMenuBar()->FindItem(ID_WINDOW_CLOSE) == NULL) return;

	bool bIsActiveChild = (GetActiveChild() != NULL);
	int iNumChildren = m_switchbar->GetButtonCount();
	bool bIsChildren = (iNumChildren > 0);
	
	GetMenuBar()->Enable(ID_WINDOW_CLOSE, bIsActiveChild);
	GetMenuBar()->Enable(ID_WINDOW_CASCADE, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_TILE, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_NEXT, bIsChildren);
	GetMenuBar()->Enable(ID_WINDOW_PREV, bIsChildren);

	int iNumDesired = (iNumChildren>0) ? iNumChildren : 1;

	while (num_window_menus < iNumDesired)
	{
		mnuWindow->AppendCheckItem(ID_WINDOW_WINDOWS + num_window_menus, "XYZ");
		num_window_menus++;
	}

	while (num_window_menus > iNumDesired)
	{
		int id = ID_WINDOW_WINDOWS + num_window_menus - 1;
		wxASSERT(mnuWindow->FindItem(id) != NULL);
		mnuWindow->Destroy(id);
		num_window_menus--;
	}

	wxASSERT(num_window_menus == iNumDesired);

	if (bIsChildren)
	{

		wxASSERT(num_window_menus == iNumChildren);

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
					<< '&' << i+1 << ' '
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

		wxASSERT(num_window_menus == 1);

		UpdateCheckMenuItem(
			ID_WINDOW_WINDOWS,
			"(None)",
			false,
			false);

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

	if (canvas->saved_state_valid)
	{
		pos = canvas->saved_state_rect.GetPosition();
		size = canvas->saved_state_rect.GetSize();
		if (!current_child || !bMaximized)
		{
			bMaximized = canvas->saved_state_maximized;
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

	switchbar_popup_button_index = event.GetExtraLong();
	switchbar_popup_canvas =
		(SwitchBarCanvas*)(m_switchbar->
			GetUserDataFromIndex(switchbar_popup_button_index));

	wxMenu menu;

	menu.Append(ID_SWITCHBAR_RESTORE, "&Restore");
	menu.Append(ID_SWITCHBAR_MINIMIZE, "Mi&nimize");
	menu.AppendSeparator();
	menu.Append(ID_SWITCHBAR_CLOSE, "&Close");

	menu.Enable(ID_SWITCHBAR_RESTORE, m_switchbar->GetSelectedIndex() != switchbar_popup_button_index);
	menu.Enable(ID_SWITCHBAR_MINIMIZE, switchbar_popup_canvas->IsAttached());
	menu.Enable(ID_SWITCHBAR_CLOSE, switchbar_popup_canvas->IsClosable());

	wxPoint pos = m_switchbar->ScreenToClient(wxGetMousePosition());
	m_switchbar->PopupMenu(&menu, pos);

}

void SwitchBarParent::OnSwitchBarRestore(wxCommandEvent& event)
{
	m_switchbar->SimulateClick(switchbar_popup_button_index);
}

void SwitchBarParent::FocusCanvas(SwitchBarCanvas *canvas)
{
	int button_index = m_switchbar->GetIndexFromUserData(canvas);
	if (button_index > -1 && button_index != m_switchbar->GetSelectedIndex())
	{
		m_switchbar->SimulateClick(button_index);
	}
}

void SwitchBarParent::OnSwitchBarMinimize(wxCommandEvent& event)
{
	m_switchbar->SelectButton(-1);
	m_switchbar->RaiseEvent(switchbar_popup_button_index, false);
	int new_index = m_switchbar->GetIndexFromUserData(GetActiveChild());
	m_switchbar->SelectButton(new_index);
}

void SwitchBarParent::OnSwitchBarClose(wxCommandEvent& event)
{
	if (switchbar_popup_canvas->IsAttached())
	{
		SwitchBarChild *child = (SwitchBarChild*)switchbar_popup_canvas->GetParent();
		child->Close();
	}
	else
	{
		switchbar_popup_canvas->Destroy();
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
			wndpl.length = sizeof WINDOWPLACEMENT;
			::GetWindowPlacement((HWND)child->GetHandle(), &wndpl);
			canvas->saved_state_rect.x =
				wndpl.rcNormalPosition.left;
			canvas->saved_state_rect.y =
				wndpl.rcNormalPosition.top;
			canvas->saved_state_rect.width =
				wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
			canvas->saved_state_rect.height =
				wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

			canvas->saved_state_maximized =
				(wndpl.flags & WPF_RESTORETOMAXIMIZED) != 0;
				
		#else

			if (child->IsIconized())
			{
				child->Iconize(false);
			}
			canvas->saved_state_maximized = child->IsMaximized();
			if (child->IsMaximized())
			{
				child->Maximize(false);
			}
			canvas->saved_state_rect = child->GetRect();

		#endif

		canvas->saved_state_valid = true;
		canvas->Show(false);
		canvas->OnDetach();
		canvas->Reparent(this);
		if (canvas->saved_state_maximized)
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
			canvas->Show(true);
		}
		child->Show(true);
		child->Activate();

	}

	UpdateWindowMenu();

}
