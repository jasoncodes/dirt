#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LanListFrame.cpp,v 1.1 2003-04-30 02:57:27 jason Exp $)

#include "LanListFrame.h"
#include "util.h"

#include "res/dirt.xpm"

enum
{
	ID_LIST = 1
};

BEGIN_EVENT_TABLE(LanListFrame, wxFrame)
	EVT_CLOSE(LanListFrame::OnClose)
	EVT_SIZE(LanListFrame::OnSize)
END_EVENT_TABLE()

LanListFrame::LanListFrame()
	: wxFrame(NULL, wxID_ANY, AppTitle(wxT("LAN List")), wxDefaultPosition, wxSize(600, 300), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL, wxT("Dirt"))
{

	SetIcon(wxIcon(dirt_xpm));

	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL);
	m_lstServers = new wxListCtrl(panel, ID_LIST, wxPoint(0, 0), wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxNO_BORDER);

	CenterOnScreen();
	RestoreWindowState(this, &m_config, wxT("LAN List"), true, false);

}

LanListFrame::~LanListFrame()
{
}

void LanListFrame::OnClose(wxCloseEvent &event)
{
	SaveWindowState(this, &m_config, wxT("LAN List"));
	event.Skip();
}

void LanListFrame::OnSize(wxSizeEvent &event)
{
	m_lstServers->SetSize(event.GetSize());
	event.Skip();
}