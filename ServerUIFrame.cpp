#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrame.cpp,v 1.49 2003-04-02 03:03:43 jason Exp $)

#include "ServerUIFrame.h"
#include "ServerUIFrameConfig.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "InputControl.h"
#include "util.h"
#include "TrayIcon.h"

#include "res/dirt.xpm"
#include "res/disabled.xpm"
#include "res/enabled.xpm"
#include "res/active_1.xpm"
#include "res/active_2.xpm"
#include "res/active_3.xpm"
#include "res/active_4.xpm"
#include "res/active_5.xpm"
#include "res/active_6.xpm"
#include "res/active_7.xpm"
#include "res/active_8.xpm"
#include "res/active_9.xpm"
#include "res/active_many.xpm"

const char **xpms[13] = {
	dirt_xpm,
	active_1_xpm,
	active_2_xpm,
	active_3_xpm,
	active_4_xpm,
	active_5_xpm,
	active_6_xpm,
	active_7_xpm,
	active_8_xpm,
	active_9_xpm,
	active_many_xpm,
	disabled_xpm,
	enabled_xpm
};

enum XPMs
{
	xpmDirt = 0,
	xpmActive1,
	xpmActive2,
	xpmActive3,
	xpmActive4,
	xpmActive5,
	xpmActive6,
	xpmActive7,
	xpmActive8,
	xpmActive9,
	xpmActiveMany,
	xpmDisabled,
	xpmEnabled
};

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_CONNECTIONS,
	ID_FILE_EXIT,
	ID_HELP_ABOUT,
	ID_STARTSTOP,
	ID_CONFIGURATION,
	ID_CLIENT,
	ID_CLEAR,
	ID_TIMER_UPDATECONNECTIONS,
	ID_TRAY,
	ID_RESTORE,
	ID_CONNECTION_KICK
};

BEGIN_EVENT_TABLE(ServerUIFrame, wxFrame)
	EVT_SIZE(ServerUIFrame::OnSize)
	EVT_CLOSE(ServerUIFrame::OnClose)
	EVT_TEXT_ENTER(ID_INPUT, ServerUIFrame::OnInput)
	EVT_MENU(ID_HELP_ABOUT, ServerUIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ServerUIFrame::OnFileExit)
	EVT_BUTTON(ID_STARTSTOP, ServerUIFrame::OnStartStop)
	EVT_BUTTON(ID_CONFIGURATION, ServerUIFrame::OnConfiguration)
	EVT_BUTTON(ID_CLIENT, ServerUIFrame::OnClient)
	EVT_BUTTON(ID_CLEAR, ServerUIFrame::OnClear)
	EVT_TIMER(ID_TIMER_UPDATECONNECTIONS, ServerUIFrame::OnTimerUpdateConnections)
	EVT_TRAYICON_LEFT_DCLICK(ID_TRAY, ServerUIFrame::OnTrayDblClick)
	EVT_TRAYICON_RIGHT_UP(ID_TRAY, ServerUIFrame::OnTrayRightClick)
	EVT_MENU(ID_RESTORE, ServerUIFrame::OnRestore)
	EVT_MENU(ID_STARTSTOP, ServerUIFrame::OnStartStop)
	EVT_MENU(ID_CLIENT, ServerUIFrame::OnClient)
	EVT_ICONIZE(ServerUIFrame::OnIconize)
	EVT_IDLE(ServerUIFrame::OnIdle)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_CONNECTIONS, ServerUIFrame::OnConnectionRClick)
	EVT_MENU(ID_CONNECTION_KICK, ServerUIFrame::OnConnectionKick)
END_EVENT_TABLE()

ServerUIFrame::ServerUIFrame()
	: wxFrame(
		NULL, -1, AppTitle(wxT("Server")),
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL), m_hide_self(false)
{

	SetIcon(wxIcon(dirt_xpm));

	m_tray = new TrayIcon;
	if (m_tray->Ok())
	{
		m_tray->SetEventHandler(this, ID_TRAY);
		m_last_xpm = xpmDisabled;
		m_last_tooltip = wxT("Initializing");
		m_tray->SetIcon(xpms[xpmDisabled]);
		m_tray->SetToolTip(AppTitle(wxT("Server")) + wxT("\n") + wxT("Initializing"));
	}

	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	m_lstConnections = new wxListCtrl(panel, ID_CONNECTIONS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	FixBorder(m_lstConnections);
	m_lstConnections->InsertColumn(0, wxT("Nickname"),     wxLIST_FORMAT_LEFT, 96);
	m_lstConnections->InsertColumn(1, wxT("Host"),         wxLIST_FORMAT_LEFT, 96);
	m_lstConnections->InsertColumn(2, wxT("User Details"), wxLIST_FORMAT_LEFT, 160);
	m_lstConnections->InsertColumn(3, wxT("Type"),         wxLIST_FORMAT_LEFT, 48);
	m_lstConnections->InsertColumn(4, wxT("Away Message"), wxLIST_FORMAT_LEFT, 96);
	m_lstConnections->InsertColumn(5, wxT("Idle Time"),    wxLIST_FORMAT_LEFT, 60);
	m_lstConnections->InsertColumn(6, wxT("Latency"),      wxLIST_FORMAT_LEFT, 60);
	m_lstConnections->InsertColumn(7, wxT("User Agent"),   wxLIST_FORMAT_LEFT, 96);
	m_lstConnections->InsertColumn(8, wxT("Join Time"),    wxLIST_FORMAT_LEFT, 128);

	wxStaticBox *boxConnections = new wxStaticBox(panel, -1, wxT("Connections"));
	wxBoxSizer *szrConnections = new wxStaticBoxSizer(boxConnections, wxVERTICAL);
	{
		szrConnections->Add(m_lstConnections, 1, wxEXPAND);
	}

	m_txtInput = new InputControl(panel, ID_INPUT);
	m_txtLog = new wxTextCtrl(panel, ID_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);
	FixBorder(m_txtLog);
	m_txtLog->SetFont(m_txtInput->GetFont());

	wxStaticBox *boxConsole = new wxStaticBox(panel, -1, wxT("Console"));
	wxBoxSizer *szrConsole = new wxStaticBoxSizer(boxConsole, wxVERTICAL);
	{
		szrConsole->Add(m_txtLog, 1, wxEXPAND);
		szrConsole->Add(m_txtInput, 0, wxEXPAND);
	}

	m_cmdStartStop = new wxButton(panel, ID_STARTSTOP, wxT("&Start"));
	m_cmdConfiguration = new wxButton(panel, ID_CONFIGURATION, wxT("&Configuration"));
	m_cmdClient = new wxButton(panel, ID_CLIENT, wxT("&Launch Client"));
	m_cmdClear = new wxButton(panel, ID_CLEAR, wxT("Clear Lo&g"));

	wxBoxSizer *szrButtons = new wxBoxSizer(wxVERTICAL);
	{
		szrButtons->Add(m_cmdStartStop, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdConfiguration, 0, wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdClient, 0, wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdClear, 0, wxBOTTOM | wxEXPAND, 8);
	}

	wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
	{
		szrLeft->Add(szrConnections, 1, wxEXPAND);
		szrLeft->Add(szrConsole, 1, wxEXPAND);
	}

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{
		szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND, 8);
		szrAll->Add(szrButtons, 0, wxALL | wxEXPAND, 8);
	}

	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints( this );

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, wxT("E&xit\tAlt-F4"), wxT("Quit the program"));
	mnu->Append(mnuFile, wxT("&File"));

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, wxT("&About\tF1"));
	mnu->Append(mnuHelp, wxT("&Help"));

	SetMenuBar(mnu);

	m_server = new ServerDefault(this);
	OnServerStateChange();
	m_server->Start();

	m_size_set = false;

	if (!m_server->IsRunning() || !m_tray->Ok())
	{
		SetPositionAndShow();
	}

	m_txtInput->SetFocus();

	m_tmrUpdateConnections = new wxTimer(this, ID_TIMER_UPDATECONNECTIONS);
	m_tmrUpdateConnections->Start(1000);

}

ServerUIFrame::~ServerUIFrame()
{

	wxTimer *tmr = m_tmrUpdateConnections;
	Server *srv = m_server;
	TrayIcon *tray = m_tray;

	m_tmrUpdateConnections = NULL;
	m_server = NULL;
	m_tray = NULL;

	delete tmr;
	tray->SetEventHandler(NULL);
	delete tray;
	delete srv;

}

void ServerUIFrame::OnSize(wxSizeEvent &event)
{
	m_txtLog->ShowPosition(m_txtLog->GetLastPosition());
	event.Skip();
}

void ServerUIFrame::OnClose(wxCloseEvent &event)
{
	if (m_size_set && m_server)
	{
		SaveWindowState(this, m_server->GetConfig().GetConfig(), wxT("Server"));
	}
	event.Skip();
}

void ServerUIFrame::SetPositionAndShow()
{
	if (m_size_set)
	{
		ForceForegroundWindow(this);
	}
	else
	{
		ResetWindowPos();
		RestoreWindowState(this, m_server->GetConfig().GetConfig(), wxT("Server"), true);
		m_size_set = true;
	}
	m_txtLog->ShowPosition(m_txtLog->GetLastPosition());
}

void ServerUIFrame::OnTrayDblClick(wxMouseEvent &event)
{
	SetPositionAndShow();
}

void ServerUIFrame::OnIconize(wxIconizeEvent &event)
{
	if (event.Iconized() && m_tray && m_tray->Ok())
	{
		Show(false);
	}
	else
	{
		m_txtLog->ShowPosition(m_txtLog->GetLastPosition());
		event.Skip();
	}
}

void ServerUIFrame::OnTrayRightClick(wxMouseEvent &event)
{
	wxMenu mnu;
	mnu.Append(ID_RESTORE, wxT("&Open"));
	mnu.AppendSeparator();
	mnu.Append(ID_STARTSTOP, m_cmdStartStop->GetLabel());
	mnu.Append(ID_CLIENT, wxT("Launch &Client"));
	mnu.Enable(ID_CLIENT, m_cmdClient->IsEnabled());
	mnu.AppendSeparator();
	mnu.Append(ID_FILE_EXIT, wxT("E&xit"));
	m_tray->PopupMenu(&mnu, event.GetPosition());
}

void ServerUIFrame::OnRestore(wxCommandEvent &event)
{
	SetPositionAndShow();
}

void ServerUIFrame::OnIdle(wxIdleEvent &event)
{
	event.Skip();
}

void ServerUIFrame::OnInput(wxCommandEvent &event)
{
	if (m_server)
	{
		m_server->ProcessConsoleInput(event.GetString());
	}
}

void ServerUIFrame::OnServerInformation(const wxString &line)
{
	Output(line);
}

void ServerUIFrame::OnServerWarning(const wxString &line)
{
	Output(line);
}

void ServerUIFrame::Output(const wxString &line)
{
	if (m_txtLog->GetValue().Length() > 16000)
	{
		m_txtLog->Remove(0, 1000);
		int i = m_txtLog->GetLineLength(0);
		if (i > 0)
		{
			m_txtLog->Remove(0, i + 2);
		}
	}
	wxString tmp = GetLongTimestamp() + LogControl::ConvertModifiersIntoHtml(line, true);
	if (m_txtLog->GetValue().Length() > 0)
	{
		m_txtLog->AppendText(wxT("\n") + tmp);
	}
	else
	{
		m_txtLog->AppendText(tmp);
	}
	// this may need some optimization
	//(see man page for GetNumberOfLines() for details)
	int i = m_txtLog->GetNumberOfLines();
	i = m_txtLog->GetLineLength(i);
	m_txtLog->SetInsertionPoint(m_txtLog->GetInsertionPoint() - i);
}

bool ServerUIFrame::OnServerPreprocess(wxString &cmd, wxString &params)
{
	if (cmd == wxT("CLEAR"))
	{
		m_txtLog->Clear();
		return true;
	}
	else if (cmd == wxT("EXIT"))
	{
		Close();
		return true;
	}
	else
	{
		return false;
	}
}

wxArrayString ServerUIFrame::OnServerSupportedCommands()
{
	return SplitString(wxT("CLEAR EXIT"), wxT(" "));
}

bool ServerUIFrame::ResetWindowPos()
{
	wxRect rtWorkArea = ::wxGetClientDisplayRect();
	int width = 576;
	int height = 384;
	wxRect rtDefaultPos(
		rtWorkArea.GetRight() - width + 1,
		rtWorkArea.GetBottom() - height + 1,
		width,
		height);
	SetSize(rtDefaultPos);
	return true;
}

void ServerUIFrame::OnFileExit(wxCommandEvent& event)
{
	Close();
}

void ServerUIFrame::OnHelpAbout(wxCommandEvent& event)
{
	ShowAbout();
}

void ServerUIFrame::OnStartStop(wxCommandEvent& event)
{
	if (!m_server) return;
	if (m_server->IsRunning())
	{
		m_server->Stop();
	}
	else
	{
		m_server->Start();
	}
}

void ServerUIFrame::OnConfiguration(wxCommandEvent& event)
{
	if (m_server)
	{
		ServerUIFrameConfig dlg(this, m_server);
		if (m_server && m_server->IsRunning() && m_server->GetConfig().GetListenPort() != m_server->GetListenPort())
		{
			if (wxMessageBox(wxT("For the listen port change to take effect you need to restart the server.\n\nWould you like to restart the server now?"), wxT("Listen Port Changed"), wxOK|wxCANCEL|wxICON_INFORMATION, this) == wxOK)
			{
				m_server->Stop();
				m_server->Start();
			}
		}
	}
}

void ServerUIFrame::OnClient(wxCommandEvent& event)
{
	wxASSERT(m_server && m_server->IsRunning());
	wxASSERT(wxTheApp->argc > 0);
	wxString param;
	param << wxT("--host=dirt://localhost:") << m_server->GetListenPort() << wxT("/");
	const wxChar *argv[3];
	argv[0] = wxTheApp->argv[0];
	argv[1] = param.c_str();
	argv[2] = NULL;
	::wxExecute((wxChar**)argv);
}

void ServerUIFrame::OnClear(wxCommandEvent& event)
{
	m_txtLog->Clear();
}

void ServerUIFrame::OnServerStateChange()
{
	m_cmdStartStop->SetLabel((m_server && m_server->IsRunning()) ? wxT("&Stop") : wxT("&Start"));
	m_cmdClient->Enable(m_server && m_server->IsRunning());
	UpdateTrayIcon();
}

void ServerUIFrame::OnServerConnectionChange()
{
	if (!m_server) return;
	while (m_lstConnections->GetItemCount() < (int)m_server->GetConnectionCount())
	{
		m_lstConnections->InsertItem(m_lstConnections->GetItemCount(), wxT("<NULL>"));
	}
	while (m_lstConnections->GetItemCount() > (int)m_server->GetConnectionCount())
	{
		m_lstConnections->DeleteItem(m_lstConnections->GetItemCount() - 1);
	}
	for (size_t i = 0; i < m_server->GetConnectionCount(); ++i)
	{
		m_lstConnections->SetItemData(i, (long)(const void*)m_server->GetConnection(i));
	}
	UpdateConnectionList();
	UpdateTrayIcon();
}

void ServerUIFrame::UpdateTrayIcon()
{

	if (m_tray->Ok())
	{

		XPMs new_xpm;
		wxString new_tooltip;

		if (!m_server || !m_server->IsRunning())
		{
			new_xpm = xpmDisabled;
			new_tooltip = wxT("Server stopped");
		}
		else
		{
			size_t count = m_server->GetConnectionCount();
			if (count == 0)
			{
				new_xpm = xpmEnabled;
			}
			else if (count < 10)
			{
				new_xpm = (XPMs)(xpmActive1+count-1);
			}
			else
			{
				new_xpm = xpmActiveMany;
			}
			new_tooltip.Empty();
			new_tooltip << count << wxT(" users connected");
		}

		if (new_xpm != m_last_xpm)
		{
			m_tray->SetIcon(xpms[new_xpm]);
			m_last_xpm = new_xpm;
		}
		if (new_tooltip != m_last_tooltip)
		{
			m_tray->SetToolTip(AppTitle(wxT("Server")) + wxT("\n") + new_tooltip);
			m_last_tooltip = new_tooltip;
		}

	}

}

static void SetItemText(wxListCtrl *ctl, int index, int col, wxString value)
{
	ctl->SetItem(index, col, value);
}

void ServerUIFrame::UpdateConnectionList()
{
	if (m_server)
	{
		for (int i = 0; i < m_lstConnections->GetItemCount(); ++i)
		{
			const ServerConnection *conn = (const ServerConnection*)(const void*)m_lstConnections->GetItemData(i);	
			SetItemText(m_lstConnections, i, 0, conn->GetNickname());
			SetItemText(m_lstConnections, i, 1, conn->GetRemoteHost());
			SetItemText(m_lstConnections, i, 2, conn->GetUserDetails());
			SetItemText(m_lstConnections, i, 3, ByteBuffer(conn->IsAuthenticated()?(conn->IsAdmin()?wxT("Admin"):wxT("User")):wxT("N/A")));
			SetItemText(m_lstConnections, i, 4, conn->IsAway()?(conn->GetAwayMessage().Length()?conn->GetAwayMessage():ByteBuffer(wxT("N/A"))):ByteBuffer());
			SetItemText(m_lstConnections, i, 5, conn->GetIdleTimeString());
			SetItemText(m_lstConnections, i, 6, conn->GetLatencyString());
			SetItemText(m_lstConnections, i, 7, conn->GetUserAgent());
			SetItemText(m_lstConnections, i, 8, conn->GetJoinTimeString());
		}
	}
}

void ServerUIFrame::OnTimerUpdateConnections(wxTimerEvent &event)
{
	UpdateConnectionList();
}

void ServerUIFrame::OnConnectionRClick(wxListEvent &event)
{
	int flags;
	long index = m_lstConnections->HitTest(event.GetPoint(), flags);
	if (index > -1 && index < m_lstConnections->GetItemCount())
	{
		long data = m_lstConnections->GetItemData(index);
		m_right_click_conn = (ServerConnection*)(void*)data;
		if (m_right_click_conn)
		{
			wxPoint pt = event.GetPoint();
			pt = ScreenToClient(m_lstConnections->ClientToScreen(pt));
			wxMenu menu;
			menu.Append(ID_CONNECTION_KICK, wxT("Kick"));
			PopupMenu(&menu, pt);
		}
	}
}

void ServerUIFrame::OnConnectionKick(wxCommandEvent &event)
{
	wxTextEntryDialog dialog(this, wxT("Kick message for ") + m_right_click_conn->GetId() + wxT(":"), wxT("Dirt Secure Chat"));
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString msg = dialog.GetValue();
		if (msg.Length())
		{
			msg = wxT("Kicked: ") + msg;
		}
		else
		{
			msg = wxT("Kicked");
		}
		m_right_click_conn->Terminate(msg);
	}
}

#ifdef __WXMSW__
	long ServerUIFrame::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
	{
		// The following is a hook for PowerMenu's "Minimize To Tray" system menu option
		if (nMsg == WM_INITMENUPOPUP)
		{
			HMENU hMenu = GetSystemMenu((HWND)GetHandle(), FALSE);
			if (hMenu == (HMENU)wParam)
			{
				MENUITEMINFO mii;
				mii.cbSize = sizeof MENUITEMINFO;
				mii.fMask = 0;
				if (GetMenuItemInfo(hMenu, 0x1400, FALSE, &mii))
				{
					DeleteMenu(hMenu, 0x1400+768, MF_BYCOMMAND);
				}
				mii.fMask = MIIM_ID;
				mii.wID = 0x1400+768;
				SetMenuItemInfo(hMenu, 0x1400, FALSE, &mii);
			}
		}
		else if (nMsg == WM_SYSCOMMAND)
		{
			if (wParam == 0x1400+768)
			{
				wParam = SC_MINIMIZE;
			}
		}
		return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
	}
#endif
