#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIFrame.cpp,v 1.51 2003-02-21 10:23:24 jason Exp $)

#include "ClientUIMDIFrame.h"
#include "SwitchBarChild.h"
#include "ClientUIMDICanvas.h"
#include "ClientDefault.h"
#include "util.h"
#include "NickListControl.h"
#include "Modifiers.h"
#include "FileTransfer.h"
#include "FileTransfers.h"

#include "res/dirt.xpm"

enum
{
	ID_FILE_EXIT = 1,
	ID_HELP_ABOUT,
	ID_FOCUSTIMER,
};

BEGIN_EVENT_TABLE(ClientUIMDIFrame, SwitchBarParent)
	EVT_MENU(ID_HELP_ABOUT, ClientUIMDIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ClientUIMDIFrame::OnFileExit)
	EVT_TIMER(ID_FOCUSTIMER, ClientUIMDIFrame::OnFocusTimer)
	EVT_ACTIVATE(ClientUIMDIFrame::OnActivate)
END_EVENT_TABLE()

ClientUIMDIFrame::ClientUIMDIFrame()
	: SwitchBarParent(NULL, -1, AppTitle(wxT("Client")),
		wxPoint(-1, -1), wxSize(500, 400),
		wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL)
{

	m_focused = true;

	SetIcon(wxIcon(dirt_xpm));

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, wxT("E&xit\tAlt-F4"), wxT("Quit the program"));
	mnu->Append(mnuFile, wxT("&File"));

	mnu->Append(GetWindowMenu(), wxT("&Window"));

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, wxT("&About\tF1"));
	mnu->Append(mnuHelp, wxT("&Help"));

	SetMenuBar(mnu);

	ResetWindowPos();

	Show();

	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxT("[Main]"), ChannelCanvas);
	NewWindow(canvas, true);
	m_lstNickList = canvas->GetNickList();

	tmrFocus = new wxTimer(this, ID_FOCUSTIMER);
	tmrFocus->Start(100);

	m_client = new ClientDefault(this);

}

ClientUIMDIFrame::~ClientUIMDIFrame()
{
	delete tmrFocus;
	delete m_client;
}

bool ClientUIMDIFrame::ResetWindowPos()
{
	wxRect rtWorkArea = ::wxGetClientDisplayRect();
	int height = 316;
	wxRect rtDefaultPos(
		rtWorkArea.x,
		rtWorkArea.GetBottom() - height + 1,
		rtWorkArea.width,
		height);
	SetSize(rtDefaultPos);
	return true;
}

void ClientUIMDIFrame::OnActivate(wxActivateEvent &event)
{
	m_focused = event.GetActive();
}

bool ClientUIMDIFrame::IsFocused()
{
	#ifdef __WXMSW__
		HWND hWnd = (HWND)GetHandle();
		return (::GetForegroundWindow() == hWnd);
	#else
		return m_focused;
	#endif
}

static inline bool IsWin32()
{
	#ifdef __WXMSW__
		return true;
	#else
		return false;
	#endif
}

void ClientUIMDIFrame::OnFocusTimer(wxTimerEvent& event)
{
	if (IsFocused() || !IsWin32())
	{
		SwitchBarChild *child = (SwitchBarChild*)GetActiveChild();
		if (child)
		{
			child->GetCanvas()->OnActivate();
		}
	}
}

void ClientUIMDIFrame::OnFileExit(wxCommandEvent& event)
{
	Close();
}

void ClientUIMDIFrame::OnHelpAbout(wxCommandEvent& event)
{
	ShowAbout();
}

ClientUIMDICanvas* ClientUIMDIFrame::GetContext(const wxString &context, bool create_if_not_exist, bool on_not_exist_return_null)
{
	wxASSERT(m_switchbar->GetButtonCount() >= 1);
	if (context.Length() > 0)
	{
		for (int i = 1; i < m_switchbar->GetButtonCount(); ++i)
		{
			if (m_switchbar->GetButtonCaption(i).CmpNoCase(context) == 0)
			{
				return (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
			}
		}
	}
	if (context.Length() == 0 || !create_if_not_exist)
	{
		if (on_not_exist_return_null && context.Length() != 0)
		{
			return NULL;
		}
		else
		{
			return (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(0);
		}
	}
	else
	{
		ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, context, QueryCanvas);
		NewWindow(canvas, false);
		return canvas;
	}
}

ClientUIMDITransferPanel* ClientUIMDIFrame::GetContext(const long transferid)
{
	wxASSERT(m_switchbar->GetButtonCount() >= 1);
	for (int i = 1; i < m_switchbar->GetButtonCount(); ++i)
	{
		ClientUIMDICanvas *canvas = (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
		ClientUIMDITransferPanel *transfer = canvas->GetTransferPanel();
		if (transfer && transfer->GetTransferId() == transferid)
		{
			return transfer;
		}
	}
	return NULL;
}

void ClientUIMDIFrame::AddLine(const wxString &context, const wxString &line, const wxColour &line_colour, bool create_if_not_exist, bool suppress_alert, bool convert_urls)
{

	ClientUIMDICanvas *canvas = GetContext(context, create_if_not_exist);
	canvas->GetLog()->AddTextLine(GetShortTimestamp() + line, line_colour, tmmParse, convert_urls);
	
	if (!suppress_alert)
	{

		bool bAlert = false;
		
		if (GetActiveChild() != canvas->GetParent())
		{
			int button_index = m_switchbar->GetIndexFromUserData(canvas);
			if (button_index > -1)
			{
				m_switchbar->SetButtonHighlight(button_index, true);
			}
			bAlert = true;
		}

		if (!IsFocused())
		{
			#ifdef __WXMSW__
				::FlashWindow((HWND)GetHandle(), TRUE);
			#endif
			bAlert = true;
		}

		if (bAlert)
		{
			wxBell();
		}

	}

}

bool ClientUIMDIFrame::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == wxT("CLEAR"))
	{
		GetContext(context)->GetLog()->Clear();
		return true;
	}
	else if (cmd == wxT("EXIT"))
	{
		Close();
		return true;
	}
	else if (cmd == wxT("TEST"))
	{
		GetContext(context)->LogControlTest();
		return true;
	}
	else if (cmd == wxT("QUERY"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		if (ht.head.Length() > 0)
		{
			FocusCanvas(GetContext(ht.head));
		}
		cmd = wxT("MSG");
		return false;
	}
	else if (cmd == wxT("RESETWINDOWPOS"))
	{
		if (!ResetWindowPos())
		{
			OnClientWarning(context, wxT("/resetwindowpos: Unavailable on this platform"));
		}
		return true;
	}
	else if (cmd == wxT("TEST2"))
	{
		m_client->GetFileTransfers()->Test();
		return true;
	}
	else if (cmd == wxT("HELP"))
	{
		OnClientInformation(context, wxT("Supported commands: CLEAR EXIT TEST TEST2 QUERY RESETWINDOWPOS"));
		return false;
	}
	else
	{
		return false;
	}
}

void ClientUIMDIFrame::OnClientDebug(const wxString &context, const wxString &text)
{
	AddLine(context, wxT("Debug: ") + text, wxColour(128,128,128));
}

void ClientUIMDIFrame::OnClientWarning(const wxString &context, const wxString &text)
{
	AddLine(context, wxT("*** ") + text, *wxRED);
}

void ClientUIMDIFrame::OnClientError(const wxString &context, const wxString &type, const wxString &text)
{
	AddLine(context, wxString() << wxT("*** Error ") << type << wxT(": ") << text, *wxRED);
}
	
void ClientUIMDIFrame::OnClientInformation(const wxString &context, const wxString &text)
{
	AddLine(context, wxT("*** ") + text, wxColour(0,0,128));
}

void ClientUIMDIFrame::OnClientStateChange()
{
	if (!m_client->IsConnected())
	{
		m_lstNickList->Clear();
	}
	GetContext(wxEmptyString)->SetPasswordMode(false);
	wxString title;
	title << AppTitle(wxT("Client"));
	if (m_client->IsConnected())
	{
		title << wxT(" - ");
		if (m_client->GetNickname().Length())
		{
			title << m_client->GetNickname();
		}
		else
		{
			title << "<NoNick>";
		}
		const URL &url = m_client->GetLastURL();
		if (url.GetHostname().Length())
		{
			title << wxT(" on ") << url.GetHostname();
		}
	}
	SetTitle(title);
}

void ClientUIMDIFrame::OnClientAuthNeeded(const wxString &text)
{
	OnClientInformation(wxEmptyString, text);
	GetContext(wxEmptyString)->SetPasswordMode(true);
}

void ClientUIMDIFrame::OnClientAuthDone(const wxString &text)
{
	if (GetContext(wxEmptyString)->GetPasswordMode())
	{
		GetContext(wxEmptyString)->SetPasswordMode(false);
	}
	if (text.Length())
	{
		OnClientInformation(wxEmptyString, text);
	}
	m_client->SetNickname(wxEmptyString, m_client->GetDefaultNick());
}

void ClientUIMDIFrame::OnClientAuthBad(const wxString &text)
{
	OnClientWarning(wxEmptyString, text);
	GetContext(wxEmptyString)->SetPasswordMode(true);
}

void ClientUIMDIFrame::OnClientMessageOut(const wxString &context, const wxString &nick, const wxString &text, bool is_action)
{
	if (GetContext(nick, false, true))
	{
		if (is_action)
		{
			AddLine(nick, wxT("* ") + m_client->GetNickname() + wxT(" ") + text, colours[6], true, true);
		}
		else
		{
			AddLine(nick, wxT("<") + m_client->GetNickname() + wxT("> ") + text, *wxBLACK, true, true);
		}
	}
	else
	{
		if (is_action)
		{
			AddLine(context, wxT("-> *") + nick + wxT("* * ") + m_client->GetNickname() + wxT(" ") + text, colours[6], true, true);
		}
		else
		{
			AddLine(context, wxT("-> *") + nick + wxT("* ") + text, *wxBLACK, true, true);
		}
	}
}

void ClientUIMDIFrame::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_action, bool is_private)
{
	wxString context = is_private ? nick : (const wxString)wxEmptyString;
	if (is_action)
	{
		AddLine(context, wxT("* ") + nick + wxT(" ") + text, colours[6]);
	}
	else
	{
		AddLine(context, wxT("<") + nick + wxT("> ") + text);
	}
}

void ClientUIMDIFrame::OnClientUserList(const wxArrayString &nicklist)
{
	m_lstNickList->Clear();
	bool self_found = false;
	for (size_t i = 0; i < nicklist.GetCount(); ++i)
	{
		if (!self_found && nicklist[i] == m_client->GetNickname())
		{
			self_found = true;
		}
		m_lstNickList->Add(nicklist[i]);
	}
	if (!self_found && m_client->GetNickname().Length())
	{
		m_lstNickList->Add(m_client->GetNickname());
	}
}

void ClientUIMDIFrame::OnClientUserJoin(const wxString &nick, const wxString &details)
{

	wxString msg;
	if (nick == m_client->GetNickname())
	{
		msg << wxT("*** Now chatting as ") << nick;
	}
	else
	{
		msg << wxT("*** ") << nick;
		if (details.Length() > 0)
		{
			msg << wxT(" (") << details << (wxChar)OriginalModifier << wxT(")");
		}
		msg << wxT(" has joined the chat");
	}
	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, false, false);

	m_lstNickList->Add(nick);

}

void ClientUIMDIFrame::OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message)
{

	wxString msg;
	msg << wxT("*** ") << nick;
	if (details.Length() > 0)
	{
		msg << wxT(" (") << details << (wxChar)OriginalModifier << wxT(")");
	}
	msg << wxT(" has left the chat");
	if (message.Length() > 0)
	{
		msg << wxT(" (") << message << (wxChar)OriginalModifier << wxT(")");
	}
	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, false, false);

	m_lstNickList->Remove(nick);

}

void ClientUIMDIFrame::OnClientUserNick(const wxString &old_nick, const wxString &new_nick)
{

	wxString msg;
	
	if (old_nick == new_nick)
	{
		msg << wxT("*** You nickname is ") << new_nick;
	}
	else if (new_nick == m_client->GetNickname())
	{
		msg << wxT("*** You are now known as ") << new_nick;
	}
	else
	{
		msg << wxT("*** ") << old_nick;
		msg << wxT(" is now known as ");
		msg << new_nick;
	}

	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, false, false);

	if (old_nick == new_nick) return;

	m_lstNickList->Remove(old_nick);
	m_lstNickList->Add(new_nick);

	ClientUIMDICanvas *old_canvas = GetContext(old_nick, false, true);
	ClientUIMDICanvas *new_canvas = GetContext(new_nick, false, true);

	if (old_canvas && new_canvas)
	{
		wxString msg;
		msg << wxT("Note: You have a query window open to both ");
		msg << old_nick << wxT(" and ") << new_nick;
		OnClientWarning(wxEmptyString, msg);
	}
	else if (old_canvas)
	{
		old_canvas->SetTitle(new_nick);
	}

}

void ClientUIMDIFrame::OnClientWhoIs(const wxString &context, const StringHashMap &details)
{
	StringHashMap details2(details);
	wxString nickname = details2["NICK"];
	AddLine(context, nickname + " is " + details2["DETAILS"]);
	AddLine(context, nickname + " is connecting from " + details2["HOSTNAME"]);
	if (details2["AWAY"].Length())
	{
		AddLine(context, nickname + " is away: " + details2["AWAY"]);
	}
	AddLine(context, nickname + " is using " + details2["AGENT"]);
	AddLine(context, nickname + " has been idle for " + details2["IDLE"] + " (" + details2["LATENCY"] + " lag)");
	AddLine(context, nickname + " signed on at " + details2["JOINTIME"]);
	details2.erase("NICK");
	details2.erase("DETAILS");
	details2.erase("HOSTNAME");
	details2.erase("AWAY");
	details2.erase("AGENT");
	details2.erase("IDLE");
	details2.erase("LATENCY");
	details2.erase("JOINTIME");
	for (StringHashMap::iterator i = details2.begin(); i != details2.end(); ++i)
	{
		AddLine(context, nickname + " " + i->first + " = " + i->second);
	}
	AddLine(context, nickname + " End of /WHOIS");
}

void ClientUIMDIFrame::OnClientTransferNew(const FileTransfer &transfer)
{
	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxEmptyString, transfer.issend ? TransferSendCanvas : TransferReceiveCanvas);
	ClientUIMDITransferPanel *pnl = canvas->GetTransferPanel();
	pnl->SetTransferId(transfer.transferid);
	pnl->Update(transfer);
	NewWindow(canvas, true);
}

void ClientUIMDIFrame::OnClientTransferDelete(const FileTransfer &transfer)
{
	ClientUIMDITransferPanel *pnl = GetContext(transfer.transferid);
	if (pnl)
	{
		pnl->SetTransferId(-1);
		pnl->SetStatus(pnl->GetStatus() + wxT(" -- OnClientTransferDelete()"));
	}
}

void ClientUIMDIFrame::OnClientTransferState(const FileTransfer &transfer)
{
	bool bIsError = ((transfer.state == ftsSendFail) || (transfer.state == ftsGetFail));
	if (bIsError)
	{
		OnClientInformation(wxEmptyString, transfer.status);
	}
	else
	{
		OnClientWarning(wxEmptyString, transfer.status);
	}
}

void ClientUIMDIFrame::OnClientTransferTimer(const FileTransfer &transfer)
{
	ClientUIMDITransferPanel *pnl = GetContext(transfer.transferid);
	wxASSERT(pnl);
	pnl->Update(transfer);
}
