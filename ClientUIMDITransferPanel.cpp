#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDITransferPanel.cpp,v 1.31 2004-04-25 17:06:01 jason Exp $)

#include "ClientUIMDITransferPanel.h"
#include "ClientUIMDICanvas.h"
#include "util.h"
#include <wx/filename.h>
#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"
#include "SwitchBarMDI.h"

enum
{
	ID_TRANSFER_ACCEPT = 13000,
	ID_TRANSFER_ACCEPT_LAST_DIR,
	ID_TRANSFER_OPEN,
	ID_TRANSFER_OPEN_COMPLETE,
	ID_TRANSFER_OPEN_FOLDER,
	ID_TRANSFER_OPEN_FOLDER_COMPLETE
};

BEGIN_EVENT_TABLE(ClientUIMDITransferPanel, wxPanel)
	EVT_SIZE(ClientUIMDITransferPanel::OnSize)
END_EVENT_TABLE()

ClientUIMDITransferPanel::ClientUIMDITransferPanel(
	ClientUIMDICanvas *canvas, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	long style)
	: wxPanel(canvas, id, pos, size, style | wxSTATIC_BORDER | wxCLIP_CHILDREN)
{

	FixBorder(this);

	m_canvas = canvas;

	m_open_file_when_complete = false;
	m_open_folder_when_complete = false;

	UpdateCaption();

	m_lblType = new wxStaticText(
		this, wxID_ANY,
		wxString() << wxT("DCC ") << GetTypeString() << wxT(" Session"),
		wxPoint(8,8));

	m_pnlLeft = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxNO_BORDER);
	m_pnlRight = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxNO_BORDER);

	m_szrLeft = new wxBoxSizer( wxVERTICAL );
	m_szrRight = new wxBoxSizer( wxVERTICAL );

	AddRow(wxT(""));

	m_lblNickname = AddRow(wxT("Nickname:"));
	m_lblFilename = AddRow(wxT("Filename:"));
	m_lblSize = AddRow(wxT("Size:"));

	AddRow(wxT(""));

	m_lblTime = AddRow(wxT("Time:"));
	m_lblLeft = AddRow(wxT("Left:"));
	m_lblCPS = AddRow(wxT("CPS:"));
	m_lblSent = AddRow(wxT("Sent:"));

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

	m_transferid = -1;
	m_nickname = wxEmptyString;
	m_filename = wxEmptyString;
	m_filesize = 0;
	m_time = 0;
	m_timeleft = 0;
	m_cps = 0;
	m_filesent = 0;
	m_unack = 0;
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

void ClientUIMDITransferPanel::OnSize(wxSizeEvent &WXUNUSED(event))
{
	
	wxSize size = GetClientSize();
	int gauge_height = m_gauge->GetSize().y;
	int status_height = m_lblStatus->GetSize().y;

	m_pnlLeft->Move(8, m_lblType->GetRect().GetBottom());
	m_pnlRight->Move(m_pnlLeft->GetRect().GetRight() + 8, m_pnlLeft->GetRect().GetTop());
	m_pnlRight->SetSize( size.x, -1 );
	
	int gauge_y = size.y - gauge_height - 8;
	int min_gauge_y = m_pnlLeft->GetRect().GetBottom() + 8 + status_height;
	gauge_y = wxMax(gauge_y, min_gauge_y);

	m_lblStatus->Move(8, gauge_y - 8 - status_height);

	m_gauge->SetSize(8, gauge_y, size.x - 16, gauge_height);

}

static bool IsCompleteOrFail(FileTransferState state)
{
	return
		state == ftsGetComplete || state == ftsSendComplete ||
		state == ftsGetFail || state == ftsSendFail;
}

void ClientUIMDITransferPanel::Update(const FileTransfer &transfer)
{
	wxASSERT(GetTransferId() == transfer.transferid);
	wxASSERT(IsSend() == transfer.issend);
	if ((GetState() != ftsGetComplete && GetState() != ftsSendComplete) &&
		(transfer.state == ftsGetComplete || transfer.state == ftsSendComplete))
	{
		if (m_open_file_when_complete)
		{
			m_open_file_when_complete = false;
			OpenFile();
		}
		if (m_open_folder_when_complete)
		{
			m_open_folder_when_complete = false;
			OpenFolder();
		}
	}
	SetState(transfer.state);
	SetNickname(transfer.nickname);
	SetFilename(transfer.filename);
	SetFileSize(transfer.filesize);
	SetTime(transfer.time);
	SetTimeleft(IsCompleteOrFail(transfer.state)?0:transfer.timeleft);
	long cps;
	if (IsCompleteOrFail(transfer.state))
	{
		if (transfer.time)
		{
			cps = long(
				double(transfer.filesent - transfer.resume) /
				double(transfer.time) );
		}
		else
		{
			cps = -1;
		}
	}
	else
	{
		cps = transfer.cps;
	}
	SetCPS(cps);
	SetFileSent(transfer.filesent);
	if (transfer.issend)
	{
		SetUnackCount(transfer.GetUnacknowledgedCount());
	}
	SetStatus(transfer.status);
	int index = m_canvas->GetSwitchBar()->GetIndexFromUserData(m_canvas);
	if (index > -1)
	{
		bool b =
			(transfer.state == ftsSendComplete) ||
			(transfer.state == ftsSendFail) ||
			(transfer.state == ftsGetPending) ||
			(transfer.state == ftsGetComplete) ||
			(transfer.state == ftsGetFail);
		m_canvas->GetSwitchBar()->SetButtonHighlight(index, b);
	}
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
			wxFAIL_MSG(wxT("Unrecognized Canvas Type"));
			return false;
	}
}

void ClientUIMDITransferPanel::UpdateCaption()
{
	wxString caption;
	caption << GetTypeString() << wxT(' ') << m_nickname << wxT(' ') << GetShortFilename();
	m_canvas->SetTitle(caption);
}

void ClientUIMDITransferPanel::UpdateProgress()
{
	int progress = (int)((double)m_filesent / (double)m_filesize * 100.0);
	m_gauge->SetValue(progress);
	int button_index = m_canvas->GetSwitchBar()->GetIndexFromUserData(m_canvas);
	m_canvas->GetSwitchBar()->SetButtonProgress(button_index, progress);
}

void ClientUIMDITransferPanel::OnClose()
{
	if (GetTransferId() > -1)
	{
		m_canvas->GetClient()->GetFileTransfers()->DeleteTransfer(GetTransferId(), false);
	}
}

wxFileName ClientUIMDITransferPanel::GetFilenameObject()
{
	if (IsSend())
	{
		return wxFileName(GetFilename());
	}
	else
	{
		ClientConfig &config = m_canvas->GetClient()->GetConfig();
		return wxFileName(config.GetLastGetDir(), wxFileName(GetFilename()).GetFullName());
	}
}

bool ClientUIMDITransferPanel::OnPopupMenu(wxMenu &menu)
{
	
	ClientConfig &config = m_canvas->GetClient()->GetConfig();
	
	bool ok = ((m_state != ftsGetFail) && (m_state != ftsSendFail));

	int pos = 0;
	
	wxFileName fn = GetFilenameObject();
	
	if (!IsSend())
	{
		
		menu.Insert(pos++, ID_TRANSFER_ACCEPT, wxT("&Accept..."));
		menu.Enable(ID_TRANSFER_ACCEPT, m_state == ftsGetPending);
		
		menu.Insert(pos++, ID_TRANSFER_ACCEPT_LAST_DIR, wxT("Accept &to ") + fn.GetFullPath());
		menu.Enable(ID_TRANSFER_ACCEPT_LAST_DIR, m_state == ftsGetPending && wxFileName::DirExists(config.GetLastGetDir()));

	}

	if (pos)
	{
		menu.InsertSeparator(pos++);
	}

	menu.Insert(pos++, ID_TRANSFER_OPEN, wxT("&Open file"));
	menu.Enable(ID_TRANSFER_OPEN, (m_state == ftsGetComplete || IsSend()) && fn.FileExists());
	
	menu.Insert(pos++, ID_TRANSFER_OPEN_COMPLETE, wxT("Open file when &complete"), wxEmptyString, true);
	menu.Enable(ID_TRANSFER_OPEN_COMPLETE, ok && m_state != ftsGetComplete && m_state != ftsSendComplete);
	menu.Check(ID_TRANSFER_OPEN_COMPLETE, menu.IsEnabled(ID_TRANSFER_OPEN_COMPLETE) && m_open_file_when_complete);
	
	menu.Insert(pos++, ID_TRANSFER_OPEN_FOLDER, wxT("Open &folder"));
	menu.Enable(ID_TRANSFER_OPEN_FOLDER, fn.FileExists());
	
	menu.Insert(pos++, ID_TRANSFER_OPEN_FOLDER_COMPLETE, wxT("Open folder when co&mplete"), wxEmptyString, true);
	menu.Enable(ID_TRANSFER_OPEN_FOLDER_COMPLETE, ok && m_state != ftsGetComplete && m_state != ftsSendComplete);
	menu.Check(ID_TRANSFER_OPEN_FOLDER_COMPLETE, menu.IsEnabled(ID_TRANSFER_OPEN_FOLDER_COMPLETE) && m_open_folder_when_complete);

	menu.InsertSeparator(pos++);

	return true;
}

bool ClientUIMDITransferPanel::OnPopupMenuItem(wxCommandEvent &event)
{
	if (event.GetId() == ID_TRANSFER_ACCEPT)
	{
		m_canvas->ProcessInput(wxString() << wxT("/dcc accept ") << m_transferid);
		return false;
	}
	else if (event.GetId() == ID_TRANSFER_ACCEPT_LAST_DIR)
	{
		wxFileName fn = GetFilenameObject();
		m_canvas->ProcessInput(wxString() << wxT("/dcc accept ") << m_transferid << wxT(" \"") << fn.GetFullPath() << wxT("\""));
		return false;
	}
	else if (event.GetId() == ID_TRANSFER_OPEN)
	{
		OpenFile();
		return false;
	}
	else if (event.GetId() == ID_TRANSFER_OPEN_COMPLETE)
	{
		m_open_file_when_complete = !m_open_file_when_complete;
		return false;
	}
	else if (event.GetId() == ID_TRANSFER_OPEN_FOLDER)
	{
		OpenFolder();
		return false;
	}
	else if (event.GetId() == ID_TRANSFER_OPEN_FOLDER_COMPLETE)
	{
		m_open_folder_when_complete = !m_open_folder_when_complete;
		return false;
	}
	else
	{
		return true;
	}
}

wxString ClientUIMDITransferPanel::GetShortFilename()
{
	wxFileName fn = GetFilenameObject();
	wxString result = fn.GetName();
	if (fn.HasExt())
	{
		result << wxT('.') << fn.GetExt();
	}
	return result;
}

long ClientUIMDITransferPanel::GetTransferId()
{
	return m_transferid;
}

FileTransferState ClientUIMDITransferPanel::GetState()
{
	return m_state;
}

wxString ClientUIMDITransferPanel::GetNickname()
{
	return m_nickname;
}

wxString ClientUIMDITransferPanel::GetFilename()
{
	return m_filename;
}

wxLongLong_t ClientUIMDITransferPanel::GetFileSize()
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

wxLongLong_t ClientUIMDITransferPanel::GetFileSent()
{
	return m_filesent;
}

wxLongLong_t ClientUIMDITransferPanel::GetUnackCount()
{
	return m_unack;
}

wxString ClientUIMDITransferPanel::GetStatus()
{
	return m_status;
}

void ClientUIMDITransferPanel::SetTransferId(long id)
{
	m_transferid = id;
}

void ClientUIMDITransferPanel::SetState(FileTransferState state)
{
	m_state = state;
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

void ClientUIMDITransferPanel::SetFileSize(wxLongLong_t bytes)
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
	if (seconds >= 0)
	{
		m_lblLeft->SetLabel(SecondsToMMSS(seconds));
	}
	else
	{
		m_lblLeft->SetLabel(wxT("??:??"));
	}
}

void ClientUIMDITransferPanel::SetCPS(long cps)
{
	m_cps = cps;
	if (cps > -1)
	{
		m_lblCPS->SetLabel(SizeToLongString(cps, wxT("/sec")));
	}
	else
	{
		m_lblCPS->SetLabel(wxT("??? bytes/sec"));
	}
}

void ClientUIMDITransferPanel::SetFileSent(wxLongLong_t bytes)
{
	m_filesent = bytes;
	UpdateSentLabel();
	UpdateProgress();
}

void ClientUIMDITransferPanel::SetUnackCount(wxLongLong_t bytes)
{
	m_unack = bytes;
	UpdateSentLabel();
}

void ClientUIMDITransferPanel::UpdateSentLabel()
{
	wxString str;
	str = SizeToLongString(m_filesent);
	if (m_unack)
	{
		str << wxT(" (unack: ") << SizeToString(m_unack) << wxT(")");
	}
	m_lblSent->SetLabel(str);
}

void ClientUIMDITransferPanel::SetStatus(const wxString &status)
{
	m_status = status;
	m_lblStatus->SetLabel(status);
}

void ClientUIMDITransferPanel::OpenFile()
{
	::OpenFile(m_canvas->GetSwitchBar()->GetParent(), GetFilenameObject().GetFullPath());
}

void ClientUIMDITransferPanel::OpenFolder()
{
	::OpenFolder(m_canvas->GetSwitchBar()->GetParent(), GetFilenameObject().GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
}
