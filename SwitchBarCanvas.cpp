#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarCanvas.cpp,v 1.5 2003-05-06 06:58:13 jason Exp $)

#include "SwitchBarCanvas.h"
#include "SwitchBarParent.h"
#include "SwitchBar.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

BEGIN_EVENT_TABLE(SwitchBarCanvas, wxPanel)
END_EVENT_TABLE()

SwitchBarCanvas::SwitchBarCanvas(SwitchBarParent *parent, const wxPoint& pos, const wxSize& size)
	: wxPanel(
		parent, -1,
		(pos == wxDefaultPosition ? wxPoint(-256, -256) : pos),
		(size == wxDefaultSize ? wxSize(128, 128) : size),
		wxNO_BORDER | wxCLIP_CHILDREN), saved_state_valid(false)
{

	m_parent = parent;

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
	if (GetParent() != m_parent)
	{
		SwitchBarParent *frame = (SwitchBarParent*)GetParent();
		frame->SetIcon(icon);
		#ifdef __WXMSW__
			HWND hWnd = (HWND)(m_parent->GetHandle());
			DefWindowProc(hWnd, WM_NCPAINT, 1, 0);
		#endif
	}
}

SwitchBar* SwitchBarCanvas::GetSwitchBar()
{
	return m_parent->GetSwitchBar();
}
