#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIMDITransferPanel.h"
#include "ClientUIMDICanvas.h"
#include "util.h"

BEGIN_EVENT_TABLE(ClientUIMDITransferPanel, wxPanel)
	EVT_SIZE(ClientUIMDITransferPanel::OnSize)
END_EVENT_TABLE()

ClientUIMDITransferPanel::ClientUIMDITransferPanel(
	ClientUIMDICanvas *canvas, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	long style)
	: wxPanel(canvas, id, pos, size, style | wxCLIP_CHILDREN)
{

	m_canvas = canvas;

	m_lblType = new wxStaticText(this, wxID_ANY, "DCC Send Session", wxPoint(8,8));

	m_pnlLeft = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);
	m_pnlRight = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);

	m_szrLeft = new wxBoxSizer( wxVERTICAL );
	m_szrRight = new wxBoxSizer( wxVERTICAL );

	m_lblNickname = AddRow("Nickname:", "Jason");
	m_lblFilename = AddRow("Filename:", "D:\\Archive\\Stuff\\Dirt.exe");
	m_lblSize = AddRow("Size:", SizeToLongString(363520));

	AddRow("");

	m_lblTime = AddRow("Time:", SecondsToMMSS(133));
	m_lblLeft = AddRow("Left:", SecondsToMMSS(67));
	m_lblCPS = AddRow("CPS:", SizeToLongString(363520/200, "/sec"));
	m_lblSent = AddRow("Sent:", SizeToLongString(363520 / 3 * 2));

	m_pnlLeft->SetAutoLayout( TRUE );
	m_pnlLeft->SetSizer( m_szrLeft );
	m_szrLeft->Fit( m_pnlLeft );
	m_pnlLeft->Move(8, 32);

	m_pnlRight->SetAutoLayout( TRUE );
	m_pnlRight->SetSizer( m_szrRight );
	m_szrRight->Fit( m_pnlRight );
	m_pnlRight->Move(m_pnlLeft->GetRect().GetRight() + 8, m_pnlLeft->GetRect().GetTop());

	m_lblStatus = new wxStaticText(this, wxID_ANY, "Sending...");

	m_gauge = new wxGauge(
		this, -1, 100,
		wxPoint(32,64), wxSize(256, 24),
		wxGA_SMOOTH | wxNO_BORDER);
	m_gauge->SetValue(66);

}

wxStaticText *ClientUIMDITransferPanel::AddRow(const wxString &caption, const wxString &value)
{
	wxStaticText *lblLeft = new wxStaticText(m_pnlLeft, wxID_ANY, caption);
	m_szrLeft->Add( lblLeft, 0, wxBOTTOM, 4 );
	wxStaticText *lblRight = new wxStaticText(m_pnlRight, wxID_ANY, value, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	m_szrRight->Add( lblRight, 1, wxBOTTOM | wxEXPAND, 4 );
	return lblRight;
}

ClientUIMDITransferPanel::~ClientUIMDITransferPanel()
{
}

void ClientUIMDITransferPanel::OnSize(wxSizeEvent &event)
{
	
	wxSize size = GetSize();
	
	int gauge_height = m_gauge->GetSize().y;
	m_gauge->SetSize(8, size.y - gauge_height - 8, size.x - 16, gauge_height);
	m_lblStatus->Move(8, size.y - gauge_height - 16 - m_lblStatus->GetSize().y);
	m_pnlRight->SetSize( size.x, -1 );

}
