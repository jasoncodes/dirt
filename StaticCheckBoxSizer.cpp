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
RCS_ID($Id: StaticCheckBoxSizer.cpp,v 1.3 2004-05-16 04:42:48 jason Exp $)

#include "StaticCheckBoxSizer.h"

class StaticCheckBoxSizerEventHandler : public wxEvtHandler
{

public:
	StaticCheckBoxSizerEventHandler(wxStaticBox *box, wxCheckBox *chk)
		: wxEvtHandler(), m_box(box), m_chk(chk)
	{
		Connect(wxID_ANY, wxID_ANY, wxEVT_PAINT, (wxObjectEventFunction)(wxEventFunction)(wxPaintEventFunction)&StaticCheckBoxSizerEventHandler::OnPaint);
	}

	virtual ~StaticCheckBoxSizerEventHandler()
	{
	}

protected:
	void OnPaint(wxPaintEvent &event)
	{
		m_chk->Refresh();
		event.Skip();
	}

protected:
	wxStaticBox *m_box;
	wxCheckBox *m_chk;

	DECLARE_NO_COPY_CLASS(StaticCheckBoxSizerEventHandler)

};

StaticCheckBoxSizer::StaticCheckBoxSizer(wxStaticBox *box, wxCheckBox *chk, int orient)
	: wxStaticBoxSizer(box, orient), m_box(box), m_chk(chk)
{
	m_evt = new StaticCheckBoxSizerEventHandler(box, chk);
	m_box->PushEventHandler(m_evt);
};

StaticCheckBoxSizer::~StaticCheckBoxSizer()
{
	delete m_evt;
}

void StaticCheckBoxSizer::RecalcSizes()
{
	wxStaticBoxSizer::RecalcSizes();
	if (m_chk)
	{
		wxPoint pos = m_box->GetPosition();
		#ifdef __WXGTK__
			if (m_box->GetLabel().Length())
			{
				pos.y -= 8;
			}
			else
			{
				pos.y -= 12;
			}
		#endif
		#ifdef __WXMSW__
			pos.x += 12;
		#else
			pos.x += 8;
		#endif
		m_chk->Move(pos);
		m_box->Lower();
		m_chk->Raise();
		m_chk->SetSize(m_chk->GetBestSize());
	}
}
