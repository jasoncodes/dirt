#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarCanvas.cpp,v 1.7 2004-04-25 17:06:01 jason Exp $)

#include "SwitchBarMDI.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

BEGIN_EVENT_TABLE(SwitchBarCanvas, wxPanel)
END_EVENT_TABLE()

inline wxWindow* NewCanvasParent(SwitchBarParent *parent)
{
#if NATIVE_MDI
	return parent;
#else
	SwitchBarChild *child = new SwitchBarChild(parent);
	return child;
#endif
}

SwitchBarCanvas::SwitchBarCanvas(SwitchBarParent *parent, const wxPoint& pos, const wxSize& size)
	: wxPanel(
		NewCanvasParent(parent), -1,
		(pos == wxDefaultPosition ? wxPoint(-256, -256) : pos),
		(size == wxDefaultSize ? wxSize(128, 128) : size),
		wxNO_BORDER | wxCLIP_CHILDREN), m_saved_state_valid(false)
{

	m_parent = parent;

#if !NATIVE_MDI
	Move(0, 0);
	((SwitchBarChild*)GetParent())->SetCanvas(this);
#endif
	
	SwitchBar *switchbar = m_parent->GetSwitchBar();
	switchbar->AddButton(wxEmptyString, wxNullIcon, this);

}

SwitchBarCanvas::~SwitchBarCanvas()
{
	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(this);
	if (button_index > -1)
	{
		m_parent->GetSwitchBar()->RemoveButton(button_index);
	}
}

bool SwitchBarCanvas::IsAttached()
{
#if NATIVE_MDI
	return GetParent() != (wxWindow*)m_parent;
#else
	return true;
#endif
}

wxString SwitchBarCanvas::GetTitle()
{
	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(this);
	wxASSERT(button_index > -1);
	return m_parent->GetSwitchBar()->GetButtonCaption(button_index);
}

void SwitchBarCanvas::SetTitle(const wxString &title)
{
	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(this);
	wxASSERT(button_index > -1);
	m_parent->GetSwitchBar()->SetButtonCaption(button_index, title);
	if (GetParent() != m_parent)
	{
		SwitchBarParent *frame = (SwitchBarParent*)GetParent();
		frame->SetTitle(title);
	}
}

wxIcon SwitchBarCanvas::GetIcon()
{
	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(this);
	wxASSERT(button_index > -1);
	return m_parent->GetSwitchBar()->GetButtonIcon(button_index);
}

void SwitchBarCanvas::SetIcon(const wxIcon &icon)
{
	int button_index = m_parent->GetSwitchBar()->GetIndexFromUserData(this);
	wxASSERT(button_index > -1);
	m_parent->GetSwitchBar()->SetButtonIcon(button_index, icon);
	#if NATIVE_MDI
		if (GetParent() != m_parent)
		{
			SwitchBarParent *frame = (SwitchBarParent*)GetParent();
			frame->SetIcon(icon);
			#ifdef __WXMSW__
				HWND hWnd = (HWND)(m_parent->GetHandle());
				DefWindowProc(hWnd, WM_NCPAINT, 1, 0);
			#endif
	}
	#endif
}

SwitchBar* SwitchBarCanvas::GetSwitchBar()
{
	return m_parent->GetSwitchBar();
}
