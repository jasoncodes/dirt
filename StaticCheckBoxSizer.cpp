#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: StaticCheckBoxSizer.cpp,v 1.2 2003-08-01 07:48:03 jason Exp $)

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
