#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIFrame.cpp,v 1.71 2003-03-13 02:04:25 jason Exp $)

#include "ClientUIMDIFrame.h"
#include "SwitchBarChild.h"
#include "ClientUIMDICanvas.h"
#include "ClientDefault.h"
#include "util.h"
#include "NickListControl.h"
#include "Modifiers.h"
#include "FileTransfer.h"
#include "FileTransfers.h"
#include "TrayIcon.h"
#include "Dirt.h"

DECLARE_APP(DirtApp)

#include "res/dirt.xpm"
#include "res/blank.xpm"

enum
{
	ID_FILE_EXIT = 1,
	ID_HELP_ABOUT,
	ID_FOCUSTIMER,
	ID_TRAY,
	ID_TRAYTIMER,
	ID_RESTORE,
	ID_BINDING_F1,
	ID_BINDING_F2,
	ID_BINDING_F3,
	ID_BINDING_F4,
	ID_BINDING_F5,
	ID_BINDING_F6,
	ID_BINDING_F7,
	ID_BINDING_F8,
	ID_BINDING_F9,
	ID_BINDING_F10,
	ID_BINDING_F11,
	ID_BINDING_F12,
};

BEGIN_EVENT_TABLE(ClientUIMDIFrame, SwitchBarParent)
	EVT_MENU(ID_HELP_ABOUT, ClientUIMDIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ClientUIMDIFrame::OnFileExit)
	EVT_TIMER(ID_FOCUSTIMER, ClientUIMDIFrame::OnFocusTimer)
	EVT_ACTIVATE(ClientUIMDIFrame::OnActivate)
	EVT_TRAYICON_LEFT_DCLICK(ID_TRAY, ClientUIMDIFrame::OnTrayDblClick)
	EVT_TRAYICON_RIGHT_UP(ID_TRAY, ClientUIMDIFrame::OnTrayRightClick)
	EVT_ICONIZE(ClientUIMDIFrame::OnIconize)
	EVT_MENU(ID_RESTORE, ClientUIMDIFrame::OnRestore)
	EVT_TIMER(ID_TRAYTIMER, ClientUIMDIFrame::OnTrayTimer)
	EVT_MENU_RANGE(ID_BINDING_F1, ID_BINDING_F12, ClientUIMDIFrame::OnBinding)
END_EVENT_TABLE()

ClientUIMDIFrame::ClientUIMDIFrame()
	: SwitchBarParent(NULL, -1, AppTitle(wxT("Client")),
		wxPoint(-1, -1), wxSize(500, 400),
		wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL)
{

	m_client = NULL;
	m_tray = NULL;
	m_focused = true;
	m_alert = false;
	m_flash = 0;
	UpdateCaption();

	SetIcon(wxIcon(dirt_xpm));

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, wxT("E&xit\tAlt-F4"), wxT("Quit the program"));
	mnu->Append(mnuFile, wxT("&File"));

	mnu->Append(GetWindowMenu(), wxT("&Window"));

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, wxT("&About"));
	mnu->Append(mnuHelp, wxT("&Help"));

	SetMenuBar(mnu);

	ResetWindowPos();

	Show();

	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxT("[Main]"), ChannelCanvas);
	NewWindow(canvas, true);
	m_lstNickList = canvas->GetNickList();

	m_tmrFocus = new wxTimer(this, ID_FOCUSTIMER);
	m_tmrFocus->Start(100);

	m_tmrTray = new wxTimer(this, ID_TRAYTIMER);

	m_client = new ClientDefault(this);

	size_t num_accel = 12;
	wxAcceleratorEntry *entries = new wxAcceleratorEntry[GetAcceleratorCount()+num_accel];
	for (size_t i = 0; i < 12; ++i)
	{
		entries[i].Set(0, WXK_F1+i, ID_BINDING_F1+i);
	}
	for (size_t i = num_accel; i < GetAcceleratorCount()+num_accel; ++i)
	{
		entries[i] = GetAccelerators()[i-num_accel];
	}
	wxAcceleratorTable accel(GetAcceleratorCount()+num_accel, entries);
	SetAcceleratorTable(accel);
	delete[] entries;

}

ClientUIMDIFrame::~ClientUIMDIFrame()
{
	delete m_tmrFocus;
	delete m_tmrTray;
	delete m_tray;
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
	if (m_focused)
	{
		m_alert = false;
		UpdateCaption();
	}
	else
	{
		ResetRedLines();
	}
}

void ClientUIMDIFrame::ResetRedLines()
{
	for (int i = 0; i < m_switchbar->GetButtonCount(); ++i)
	{
		bool b = !m_switchbar->GetButtonHighlight(i);
		if (b && IsFocused())
		{
			b = m_switchbar->GetSelectedIndex() != i;
		}
		if (b)
		{
			ClientUIMDICanvas *canvas =
				(ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
			LogControl *txtLog = canvas->GetLog();
			if (txtLog)
			{
				txtLog->ResetRedLine();
			}
		}
	}
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
	if (m_alert && m_flash > 0)
	{
		m_flash--;
		UpdateCaption();
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

void ClientUIMDIFrame::OnTrayDblClick(wxMouseEvent &event)
{
	ForceForegroundWindow(this);
	m_tmrTray->Stop();
	delete m_tray;
	m_tray = NULL;
}

void ClientUIMDIFrame::OnRestore(wxCommandEvent &event)
{
	ForceForegroundWindow(this);
	m_tmrTray->Stop();
	delete m_tray;
	m_tray = NULL;
}

void ClientUIMDIFrame::OnTrayTimer(wxTimerEvent &event)
{
	if (m_tray && m_tray->Ok())
	{
		m_tray_flash = !m_tray_flash;
		if (m_tray_flash)
		{
			m_tray->SetIcon(blank_xpm);
		}
		else
		{
			m_tray->SetIcon(dirt_xpm);
		}
	}
	else
	{
		m_tmrTray->Stop();
	}
}
void ClientUIMDIFrame::OnTrayRightClick(wxMouseEvent &event)
{
	if (m_tray && m_tray->Ok())
	{
		wxMenu mnu;
		mnu.Append(ID_RESTORE, wxT("&Restore"));
		mnu.AppendSeparator();
		mnu.Append(ID_FILE_EXIT, wxT("E&xit"));
		m_tray->PopupMenu(&mnu, event.GetPosition());
	}
}

void ClientUIMDIFrame::OnIconize(wxIconizeEvent &event)
{
	if (event.Iconized() && !m_tray && wxGetApp().IsControlDown())
	{
		m_tray = new TrayIcon;
		m_tray_flash = false;
		m_tray_auto_restore = wxGetApp().IsShiftDown();
		if (m_tray->Ok())
		{
			m_tray->SetEventHandler(this, ID_TRAY);
			m_tray->SetIcon(dirt_xpm);
			m_tray->SetToolTip(m_title);
			Show(false);
		}
		else
		{
			delete m_tray;
			m_tray = NULL;
			event.Skip();
		}
	}
	else
	{
		event.Skip();
	}
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

	bool bAlert = false;
	bool bFlashWindow = false;

	if (!suppress_alert && GetActiveChild() != canvas->GetParent())
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
		bAlert = true;
		bFlashWindow = true;
	}

	canvas->GetLog()->AddTextLine(GetShortTimestamp() + line, line_colour, tmmParse, convert_urls, true, bAlert);
	
	if (!suppress_alert)
	{
	
		if (bAlert)
		{
			wxBell();
		}
		
		if (bFlashWindow)
		{
			#ifdef __WXMSW__
				::FlashWindow((HWND)GetHandle(), TRUE);
			#else
				m_alert = true;
				m_flash = 8;
				UpdateCaption();
			#endif
			if (m_tray && m_tray_auto_restore)
			{
				wxMouseEvent event;
				OnTrayDblClick(event);
			}
			else if (m_tray && !m_tmrTray->IsRunning())
			{
				m_tmrTray->Start(250);
			}
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
			int index = m_lstNickList->GetNickIndex(ht.head);
			if (index > -1)
			{
				ht.head = m_lstNickList->GetNick(index);
				params = wxT("\"") + ht.head + wxT("\" ") + ht.tail;
			}
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
	if (type == wxT("NICK") && context == wxEmptyString)
	{
		HeadTail ht = SplitHeadTail(text, ": ");
		if (ht.tail.Length())
		{
			NickPrompt(ht.tail);
		}
	}
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
		GetContext(wxEmptyString)->SetPasswordMode(false);
	}
	UpdateCaption();
}

void ClientUIMDIFrame::UpdateCaption()
{
	wxString title;
	if (m_alert)
	{
		title << "* ";
	}
	if (!m_alert || (m_flash % 2) == 0)
	{
		title << AppTitle(wxT("Client"));
		if (m_client && m_client->IsConnected())
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
			wxString hostname = url.GetHostname();
			if (hostname.Length())
			{
				wxString server_name = m_client->GetServerName();
				if (server_name.Length())
				{
					if (server_name.CmpNoCase(hostname) == 0 || hostname == wxT("localhost"))
					{
						title << wxT(" on ") << server_name;
					}
					else
					{
						title << wxT(" on ") << server_name << wxT(" (") << hostname << wxT(")");
					}
				}
				else
				{
					title << wxT(" on ") << hostname;
				}
			}
			if (m_client->GetLatency() >= 0)
			{
				title << wxT(" (") << AddCommas((off_t)m_client->GetLatency()) << wxT(" ms lag)");
			}
		}
	}
	m_title = title;
	if (m_tray)
	{
		m_tray->SetToolTip(m_title);
	}
	SetTitle(title);
}

void ClientUIMDIFrame::NickPrompt(const wxString &nick)
{
	InputControl *txtInput = GetContext(wxEmptyString)->GetInput();
	txtInput->SetValue(wxT("/nick ") + nick);
	txtInput->SetInsertionPoint(6);
	txtInput->SetSelection(6, txtInput->GetValue().Length());
}

void ClientUIMDIFrame::OnClientAuthNeeded(const wxString &text)
{
	wxString pass = m_client->GetLastURL().GetPassword();
	if (pass.Length())
	{
		m_client->Authenticate(pass);
	}
	else
	{
		OnClientInformation(wxEmptyString, text);
		GetContext(wxEmptyString)->SetPasswordMode(true);
	}
}

void ClientUIMDIFrame::OnClientAuthDone(const wxString &text)
{
	GetContext(wxEmptyString)->SetPasswordMode(false);
	if (text.Length())
	{
		OnClientInformation(wxEmptyString, text);
	}
	wxString nick = m_client->GetLastURL().GetUsername();
	if (nick.Length())
	{
		m_client->SetNickname(wxEmptyString, nick);
	}
	else
	{
		NickPrompt(m_client->GetDefaultNick());
	}
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
	bool is_self = (nick == m_client->GetNickname());
	if (is_action)
	{
		AddLine(context, wxT("* ") + nick + wxT(" ") + text, colours[6], true, is_self);
	}
	else
	{
		AddLine(context, wxT("<") + nick + wxT("> ") + text, *wxBLACK, true, is_self);
	}
}

void ClientUIMDIFrame::OnClientUserList(const wxArrayString &nicklist)
{
	m_lstNickList->Clear();
	m_nicklist = nicklist;
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
		m_nicklist.Add(m_client->GetNickname());
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
	m_nicklist.Add(nick);

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
	m_nicklist.Remove(nick);

}

void ClientUIMDIFrame::OnClientUserNick(const wxString &old_nick, const wxString &new_nick)
{

	wxString msg;
	bool bIsSelf = (new_nick == m_client->GetNickname());
	
	if (old_nick == new_nick)
	{
		msg << wxT("*** You nickname is ") << new_nick;
	}
	else if (bIsSelf)
	{
		msg << wxT("*** You are now known as ") << new_nick;
	}
	else
	{
		msg << wxT("*** ") << old_nick;
		msg << wxT(" is now known as ");
		msg << new_nick;
	}

	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, bIsSelf, false);

	if (old_nick == new_nick) return;

	m_lstNickList->Remove(old_nick);
	m_lstNickList->Add(new_nick);
	m_nicklist.Remove(old_nick);
	m_nicklist.Add(new_nick);

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

void ClientUIMDIFrame::OnClientUserAway(const wxString &nick, const wxString &msg)
{
	bool bIsSelf = (nick == m_client->GetNickname());
	AddLine(wxEmptyString,
		wxT("*** ") + nick + wxT(" is away: ") + msg,
		wxColour(0,0,128), true, bIsSelf);
	m_lstNickList->SetAway(nick, true);
}

void ClientUIMDIFrame::OnClientUserBack(const wxString &nick, const wxString &msg)
{
	bool bIsSelf = (nick == m_client->GetNickname());
	AddLine(wxEmptyString,
		wxT("*** ") + nick + wxT(" has returned (msg: ") + msg + (wxChar)OriginalModifier + wxT(")"),
		wxColour(0,0,128), true, bIsSelf);
	m_lstNickList->SetAway(nick, false);
}

void ClientUIMDIFrame::OnClientWhoIs(const wxString &context, const StringHashMap &details)
{
	StringHashMap details2(details);
	wxString nickname = details2[wxT("NICK")];
	AddLine(context, nickname + wxT(" is ") + details2[wxT("DETAILS")]);
	AddLine(context, nickname + wxT(" is connecting from ") + details2[wxT("HOSTNAME")]);
	if (details2.find(wxT("ISADMIN")) != details2.end())
	{
		AddLine(context, nickname + wxT(" is a server administrator"));
	}
	if (details2.find(wxT("AWAY")) != details2.end())
	{
		AddLine(context, nickname + wxT(" is away: ") + details2[wxT("AWAY")]);
	}
	AddLine(context, nickname + wxT(" is using ") + details2[wxT("AGENT")]);
	AddLine(context, nickname + wxT(" has been idle for ") + details2[wxT("IDLESTRING")] + wxT(" (") + details2[wxT("LATENCYSTRING")] + wxT(" lag)"));
	AddLine(context, nickname + wxT(" signed on at ") + details2[wxT("JOINTIMESTRING")]);
	details2.erase(wxT("NICK"));
	details2.erase(wxT("DETAILS"));
	details2.erase(wxT("HOSTNAME"));
	details2.erase(wxT("ISADMIN"));
	details2.erase(wxT("AWAY"));
	details2.erase(wxT("AGENT"));
	details2.erase(wxT("IDLE"));
	details2.erase(wxT("IDLESTRING"));
	details2.erase(wxT("LATENCY"));
	details2.erase(wxT("LATENCYSTRING"));
	details2.erase(wxT("JOINTIME"));
	details2.erase(wxT("JOINTIMESTRING"));
	for (StringHashMap::iterator i = details2.begin(); i != details2.end(); ++i)
	{
		AddLine(context, nickname + wxT(" ") + i->first + wxT(" = ") + i->second);
	}
	AddLine(context, nickname + wxT(" End of /WHOIS"));
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

void ClientUIMDIFrame::OnBinding(wxCommandEvent &event)
{
	wxString name;
	if (event.GetId() >= ID_BINDING_F1 && event.GetId() <= ID_BINDING_F12)
	{
		name = wxString()<<wxT("F")<<(event.GetId()-ID_BINDING_F1+1);
	}
	if (name.Length())
	{
		SwitchBarChild *child = (SwitchBarChild*)GetActiveChild();
		if (child)
		{
			wxString context =
				(GetSwitchBar()->GetSelectedIndex() == 0) ?
				wxEmptyString :
				child->GetCanvas()->GetTitle();
			m_client->ProcessAlias(context, m_client->GetBinding(name), wxEmptyString);
			return;
		}
	}
	event.Skip();
}
