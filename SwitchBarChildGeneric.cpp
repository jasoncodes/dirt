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
RCS_ID($Id: SwitchBarChildGeneric.cpp,v 1.3 2004-06-11 12:07:50 jason Exp $)

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
	wxWindowList &list = GetChildren();
	for (wxWindowListNode *node = list.GetFirst(); node; node = node->GetNext())
	{
		wxWindow *child = node->GetData();
		if (child->IsShown())
		{
			child->SetSize(GetClientSize());
		}
	}
}

#endif
