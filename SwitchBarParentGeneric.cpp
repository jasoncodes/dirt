#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarParentGeneric.cpp,v 1.2 2004-04-25 19:27:34 jason Exp $)

#include "SwitchBarMDI.h"

#if !NATIVE_MDI

#include "util.h"

enum
{
	ID_SWITCHBARPARENT_FIRST = 11000,
	ID_SWITCHBAR = ID_SWITCHBARPARENT_FIRST + 1000,
	ID_WINDOW_MINIMIZE,
	ID_WINDOW_CLOSE,
	ID_WINDOW_NEXT,
	ID_WINDOW_PREV,
	ID_SWITCHBAR_RESTORE,
	ID_SWITCHBAR_MINIMIZE,
	ID_SWITCHBAR_CLOSE,
	ID_SWITCHBARPARENT_LAST
};

BEGIN_EVENT_TABLE(SwitchBarParent, wxFrame)
	EVT_SIZE(SwitchBarParent::OnSize)
	EVT_BUTTON(ID_SWITCHBAR, SwitchBarParent::OnSwitchBar)
	EVT_MENU(ID_SWITCHBAR, SwitchBarParent::OnSwitchBarMenu)
	EVT_MIDDLE_CLICK(ID_SWITCHBAR, SwitchBarParent::OnSwitchBarMiddleClick)
	EVT_MENU(ID_WINDOW_MINIMIZE, SwitchBarParent::OnWindowMinimize)
	EVT_MENU(ID_WINDOW_CLOSE, SwitchBarParent::OnWindowClose)
	EVT_MENU(ID_WINDOW_NEXT, SwitchBarParent::OnWindowNext)
	EVT_MENU(ID_WINDOW_PREV, SwitchBarParent::OnWindowPrev)
	EVT_MENU(wxID_ANY, SwitchBarParent::OnSwitchBarMenuItem)
END_EVENT_TABLE()

SwitchBarParent::SwitchBarParent(
	wxWindow *parent, const wxWindowID id, const wxString& title,
	const wxPoint& pos, const wxSize& size, const long style,
	const wxString& name)
	: wxFrame(parent, id, title, pos, size, style, name)
{
	
	m_switchbar = new SwitchBar(this, ID_SWITCHBAR, wxDefaultPosition, wxDefaultSize);
	m_switchbar_popup_button_index = -1;
	m_switchbar_popup_canvas = NULL;

	m_client_area = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_CHILDREN);

	m_accelerator_count = 6;
	m_accelerators = new wxAcceleratorEntry[6];
	m_accelerators[0].Set(wxACCEL_CTRL, WXK_TAB, ID_WINDOW_NEXT);
	m_accelerators[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_TAB, ID_WINDOW_PREV);
	m_accelerators[2].Set(wxACCEL_CTRL, WXK_F6, ID_WINDOW_NEXT);
	m_accelerators[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_F6, ID_WINDOW_PREV);
	m_accelerators[4].Set(0, WXK_ESCAPE, ID_WINDOW_MINIMIZE);
	m_accelerators[5].Set(wxACCEL_CTRL, WXK_F4, ID_WINDOW_CLOSE);
	SetAcceleratorTable(wxAcceleratorTable(m_accelerator_count, m_accelerators));

}

SwitchBarParent::~SwitchBarParent()
{
	m_visible_windows.Clear();
}

SwitchBarChild* SwitchBarParent::NewWindow(SwitchBarCanvas *canvas, bool focus)
{
	if (focus)
	{
		FocusCanvas(canvas);
	}
	return (SwitchBarChild*)canvas->GetParent();
}

void SwitchBarParent::FocusCanvas(SwitchBarCanvas *canvas)
{
	SwitchBarChild *old_child = GetActiveChild();
	if (old_child)
	{
		old_child->Show(false);
	}
	SwitchBarChild *new_child = (SwitchBarChild*)canvas->GetParent();
	new_child->Move(0, 0);
	new_child->SetSize(m_client_area->GetClientSize());
	new_child->Show(true);
	m_switchbar->SelectButton(m_switchbar->GetIndexFromUserData(canvas));
	canvas->OnActivate();
	AddVisibleCanvas(canvas);
}

void SwitchBarParent::AddVisibleCanvas(SwitchBarCanvas *canvas)
{
	RemoveVisibleCanvas(canvas);
	m_visible_windows.Insert(canvas, 0u);
}

void SwitchBarParent::RemoveVisibleCanvas(SwitchBarCanvas *canvas)
{
	if (m_visible_windows.Index(canvas) != wxNOT_FOUND)
	{
		m_visible_windows.Remove(canvas);
	}
}

wxWindow* SwitchBarParent::GetActiveChildWindow() const
{
	wxWindowList &list = m_client_area->GetChildren();
	for (wxWindowListNode *node = list.GetFirst(); node; node = node->GetNext())
	{
		wxWindow *child = node->GetData();
		if (child->IsShown())
		{
			return child;
		}
	}
	return NULL;
}

SwitchBarChild* SwitchBarParent::GetActiveChild() const
{
	return (SwitchBarChild*)GetActiveChildWindow();
}

SwitchBarCanvas* SwitchBarParent::GetActiveCanvas() const
{
	SwitchBarChild *child = GetActiveChild();
	return child ? child->GetCanvas() : NULL;
}

void SwitchBarParent::OnSize(wxSizeEvent& WXUNUSED(event))
{
	int switchbar_height = m_switchbar->GetSize().y;
	wxSize size = GetClientSize();
	m_client_area->SetSize(0, 0, size.x, size.y - switchbar_height);
	m_switchbar->SetSize(0, size.y - switchbar_height, size.x, switchbar_height);
	wxWindow *wnd = GetActiveChildWindow();
	if (wnd)
	{
		wnd->SetSize(m_client_area->GetClientSize());
	}
}

void SwitchBarParent::CloseCanvas(SwitchBarCanvas *canvas)
{
	SwitchBarChild *child = (SwitchBarChild*)canvas->GetParent();
	child->Destroy();
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
		m_switchbar->RaiseEvent(sel, wxEVT_COMMAND_BUTTON_CLICKED);

	}

}

void SwitchBarParent::OnSwitchBar(wxCommandEvent& event)
{

	SwitchBarCanvas *canvas = (SwitchBarCanvas*)event.GetClientData();

	if (event.GetInt() > -1)
	{

		FocusCanvas(canvas);

	}
	else
	{

		SwitchBarChild *child = (SwitchBarChild*)canvas->GetParent();
		child->Show(false);
		RemoveVisibleCanvas(canvas);
		SelectLastVisibleWindow();

	}

}

void SwitchBarParent::SelectLastVisibleWindow()
{
	if (m_visible_windows.GetCount() > 0)
	{
		FocusCanvas(m_visible_windows[0u]);
	}
}

void SwitchBarParent::OnSwitchBarMiddleClick(wxCommandEvent& event)
{
	m_switchbar_popup_button_index = event.GetExtraLong();
	m_switchbar_popup_canvas = (SwitchBarCanvas*)event.GetClientData();
	event.SetId(
		m_switchbar_popup_canvas->IsClosable() ?
		ID_SWITCHBAR_CLOSE :
		ID_SWITCHBAR_MINIMIZE);
	OnSwitchBarMenuItem(event);
	m_switchbar_popup_button_index = -1;
	m_switchbar_popup_canvas = NULL;
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

	m_switchbar_popup_button_index = -1;
	m_switchbar_popup_canvas = NULL;

}

void SwitchBarParent::OnSwitchBarMenuItem(wxCommandEvent& event)
{
	wxASSERT(m_switchbar_popup_canvas && m_switchbar_popup_button_index > -1);
	if (m_switchbar_popup_canvas->OnPopupMenuItem(event))
	{
		if (event.GetId() == ID_SWITCHBAR_RESTORE)
		{
			m_switchbar->SimulateClick(m_switchbar_popup_button_index);
		}
		else if (event.GetId() == ID_SWITCHBAR_MINIMIZE)
		{
			if (m_switchbar->GetSelectedIndex() != -1 && m_switchbar_popup_canvas->IsAttached())
			{
				if (m_switchbar->GetSelectedIndex() != m_switchbar_popup_button_index)
				{
					m_switchbar->SimulateClick(m_switchbar_popup_button_index);
				}
				m_switchbar->SimulateClick(m_switchbar_popup_button_index);
			}
		}
		else if (event.GetId() == ID_SWITCHBAR_CLOSE)
		{
			CloseCanvas(m_switchbar_popup_canvas);
		}
	}
}

void SwitchBarParent::OnWindowMinimize(wxCommandEvent& WXUNUSED(event))
{
	if (GetActiveChild() != NULL)
	{
		int button_index = m_switchbar->GetIndexFromUserData(GetActiveCanvas());
		m_switchbar->SimulateClick(button_index);
	}
}

void SwitchBarParent::OnWindowClose(wxCommandEvent& WXUNUSED(event))
{
	wxASSERT(GetActiveChild() != NULL);
	GetActiveChild()->Close();
}

void SwitchBarParent::OnWindowNext(wxCommandEvent& WXUNUSED(event))
{
	NextChild(false);
}

void SwitchBarParent::OnWindowPrev(wxCommandEvent& WXUNUSED(event))
{
	NextChild(true);
}

#endif
