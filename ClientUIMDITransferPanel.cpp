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
#include <wx/filename.h>

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

	UpdateCaption();

	m_lblType = new wxStaticText(
		this, wxID_ANY,
		wxString() << "DCC " << GetTypeString() << " Session",
		wxPoint(8,8));

	m_pnlLeft = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);
	m_pnlRight = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);

	m_szrLeft = new wxBoxSizer( wxVERTICAL );
	m_szrRight = new wxBoxSizer( wxVERTICAL );

	AddRow("");

	m_lblNickname = AddRow("Nickname:");
	m_lblFilename = AddRow("Filename:");
	m_lblSize = AddRow("Size:");

	AddRow("");

	m_lblTime = AddRow("Time:");
	m_lblLeft = AddRow("Left:");
	m_lblCPS = AddRow("CPS:");
	m_lblSent = AddRow("Sent:");

	m_pnlLeft->SetAutoLayout( TRUE );
	m_pnlLeft->SetSizer( m_szrLeft );
	m_szrLeft->Fit( m_pnlLeft );

	m_pnlRight->SetAutoLayout( TRUE );
	m_pnlRight->SetSizer( m_szrRight );
	m_szrRight->Fit( m_pnlRight );

	m_lblStatus = new wxStaticText(this, wxID_ANY, wxEmptyString);

	m_gauge = new wxGauge(
		this, -1, 100,
		wxDefaultPosition, wxSize(256, 24),
		wxGA_SMOOTH | wxNO_BORDER);

	m_nickname = wxEmptyString;
	m_filename = wxEmptyString;
	m_filesize = 0;
	m_time = 0;
	m_timeleft = 0;
	m_cps = 0;
	m_filesent = 0;
	m_status = wxEmptyString;

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
	int status_height = m_lblStatus->GetSize().y;

	m_pnlLeft->Move(8, m_lblType->GetRect().GetBottom());
	m_pnlRight->Move(m_pnlLeft->GetRect().GetRight() + 8, m_pnlLeft->GetRect().GetTop());
	m_pnlRight->SetSize( size.x, -1 );
	
	int gauge_y = size.y - gauge_height - 8;
	int min_gauge_y = m_pnlLeft->GetRect().GetBottom() + 16 + status_height;
	gauge_y = wxMax(gauge_y, min_gauge_y);

	m_lblStatus->Move(8, gauge_y - 8 - status_height);

	m_gauge->SetSize(8, gauge_y, size.x - 16, gauge_height);

}

bool ClientUIMDITransferPanel::IsSend()
{
	switch (m_canvas->GetType())
	{
		case TransferSendCanvas:
			return true;
		case TransferReceiveCanvas:
			return false;
		default:
			wxFAIL_MSG("Unrecognized Canvas Type");
			return false;
	}
}

void ClientUIMDITransferPanel::UpdateCaption()
{
	wxString caption;
	caption << GetTypeString() << ' ' << m_nickname << ' ' << GetShortFilename();
	m_canvas->SetTitle(caption);
}

void ClientUIMDITransferPanel::UpdateProgress()
{
	int progress = (int)((double)m_filesent / (double)m_filesize * 100.0);
	m_gauge->SetValue(progress);
	int button_index = m_canvas->GetSwitchBar()->GetIndexFromUserData(m_canvas);
	m_canvas->GetSwitchBar()->SetButtonProgress(button_index, progress);
}

wxString ClientUIMDITransferPanel::GetShortFilename()
{
	wxFileName fn(m_filename);
	wxString result = fn.GetName();
	if (fn.HasExt())
	{
		result << '.' << fn.GetExt();
	}
	return result;
}

wxString ClientUIMDITransferPanel::GetNickname()
{
	return m_nickname;
}

wxString ClientUIMDITransferPanel::GetFilename()
{
	return m_filename;
}

off_t ClientUIMDITransferPanel::GetFileSize()
{
	return m_filesize;
}

long ClientUIMDITransferPanel::GetTime()
{
	return m_time;
}

long ClientUIMDITransferPanel::GetTimeleft()
{
	return m_timeleft;
}

long ClientUIMDITransferPanel::GetCPS()
{
	return m_cps;
}

off_t ClientUIMDITransferPanel::GetFileSent()
{
	return m_filesent;
}

wxString ClientUIMDITransferPanel::GetStatus()
{
	return m_status;
}

void ClientUIMDITransferPanel::SetNickname(const wxString &nickname)
{
	m_nickname = nickname;
	m_lblNickname->SetLabel(nickname);
	UpdateCaption();
}

void ClientUIMDITransferPanel::SetFilename(const wxString &filename)
{
	m_filename = filename;
	m_lblFilename->SetLabel(filename);
	UpdateCaption();
}

void ClientUIMDITransferPanel::SetFileSize(off_t bytes)
{
	m_filesize = bytes;
	m_lblSize->SetLabel(SizeToLongString(bytes));
	UpdateProgress();
}

void ClientUIMDITransferPanel::SetTime(long seconds)
{
	m_time = seconds;
	m_lblTime->SetLabel(SecondsToMMSS(seconds));
}

void ClientUIMDITransferPanel::SetTimeleft(long seconds)
{
	m_timeleft = seconds;
	m_lblLeft->SetLabel(SecondsToMMSS(seconds));
}

void ClientUIMDITransferPanel::SetCPS(long cps)
{
	m_cps = cps;
	m_lblCPS->SetLabel(SizeToLongString(cps, "/sec"));
}

void ClientUIMDITransferPanel::SetFileSent(off_t bytes)
{
	m_filesent = bytes;
	m_lblSent->SetLabel(SizeToLongString(bytes));
	UpdateProgress();
}

void ClientUIMDITransferPanel::SetStatus(const wxString &status)
{
	m_status = status;
	m_lblStatus->SetLabel(status);
}
