#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBarChildGeneric.cpp,v 1.1 2004-04-25 17:06:02 jason Exp $)

#include "SwitchBarMDI.h"

#if !NATIVE_MDI

BEGIN_EVENT_TABLE(SwitchBarChild, wxPanel)
	EVT_SIZE(SwitchBarChild::OnSize)
END_EVENT_TABLE()

SwitchBarChild::SwitchBarChild(SwitchBarParent *parent)
	: wxPanel(parent->GetMDIChildWindow(), wxID_ANY, wxPoint(0, 0), wxDefaultSize),
		m_parent(parent), m_canvas(NULL)
{
	Show(false);
}

void SwitchBarChild::SetCanvas(SwitchBarCanvas *canvas)
{
	wxASSERT(!m_canvas);
	m_canvas = canvas;
	canvas->OnAttach();
}

SwitchBarChild::~SwitchBarChild()
{
	if (m_canvas)
	{
		SwitchBarParent *parent = (SwitchBarParent*)GetParent()->GetParent();
		parent->RemoveVisibleCanvas(m_canvas);
		parent->SelectLastVisibleWindow();
	}
}

void SwitchBarChild::OnSize(wxSizeEvent& WXUNUSED(event))
{
	if (GetChildren().GetCount() == 1)
	{
		GetChildren().GetFirst()->GetData()->SetSize(GetClientSize());
	}
}

#endif
