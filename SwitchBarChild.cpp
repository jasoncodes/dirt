#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarChild.cpp,v 1.11 2003-03-29 05:30:09 jason Exp $)

#include "SwitchBarChild.h"
#include "SwitchBarParent.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

BEGIN_EVENT_TABLE(SwitchBarChild, wxMDIChildFrame)
	EVT_SIZE(SwitchBarChild::OnSize)
	EVT_MOVE(SwitchBarChild::OnMove)
	EVT_CLOSE(SwitchBarChild::OnClose)
	EVT_ACTIVATE(SwitchBarChild::OnActivate)
END_EVENT_TABLE()

SwitchBarChild::SwitchBarChild(
	SwitchBarParent *parent,
	const wxPoint& pos, const wxSize& size,
	const long style, SwitchBarCanvas *canvas)
	: wxMDIChildFrame(
		parent, -1, canvas->GetTitle(), pos, size,
		style | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN)
{

	m_parent = parent;
	m_canvas = canvas;

	SetIcon(canvas->GetIcon());
	canvas->SetSize(GetClientSize());
	canvas->Reparent(this);
	canvas->OnAttach();

	SetSizeHints(100, 100);

}

SwitchBarChild::~SwitchBarChild()
{
}

void SwitchBarChild::OnActivate(wxActivateEvent& event)
{

	if (m_canvas)
	{
		if (event.GetActive())
		{
			m_canvas->Show();
			m_canvas->SetFocus();
			m_canvas->OnActivate();
		}
	}

	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(m_canvas);
	if (button_index > -1)
	{
		if (event.GetActive())
		{
			m_parent->GetSwitchBar()->SelectButton(button_index);
		}
		else if (m_parent->GetSwitchBar()->GetSelectedIndex() == button_index)
		{
			m_parent->GetSwitchBar()->SelectButton(-1);
		}
	}

	m_parent->UpdateWindowMenu();

}

void SwitchBarChild::OnMove(wxMoveEvent& event)
{
	
	if (IsIconized())
	{
		int index = m_parent->GetSwitchBar()->GetIndexFromUserData(m_canvas);
		m_parent->GetSwitchBar()->SelectButton(-1);
		m_parent->GetSwitchBar()->RaiseEvent(index, false);
		m_canvas->Show(false);
	}

	m_parent->UpdateWindowMenu();
	event.Skip();

}

void SwitchBarChild::OnSize(wxSizeEvent& event)
{

	m_parent->UpdateWindowMenu();

	if (GetSize().x > 1 && GetSize().y > 1 && IsShown())
	{
		m_canvas->Show();
		event.Skip();
	}

	#ifdef __WXMSW__
		HWND hWnd = (HWND)(GetParent()->GetHandle());
		DefWindowProc(hWnd, WM_NCPAINT, 1, 0);
	#endif

}

void SwitchBarChild::OnClose(wxCloseEvent& event)
{
	m_parent->UpdateWindowMenu();
	if (!m_canvas->IsClosable() && event.CanVeto())
	{
		event.Veto();
		int index = m_parent->GetSwitchBar()->GetIndexFromUserData(m_canvas);
		m_parent->GetSwitchBar()->SelectButton(-1);
		m_parent->GetSwitchBar()->RaiseEvent(index, false);
	}
	else
	{
		m_canvas->OnClose();
		event.Skip();
	}
}
