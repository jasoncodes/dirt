#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIFrame.cpp,v 1.132 2003-08-13 08:16:17 jason Exp $)

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
#include "LogWriter.h"
#include "SwitchBar.h"
#include "LogControl.h"
#include "InputControl.h"
#include "ClientUIMDITransferPanel.h"
#include "ClientUIMDITransferResumeDialog.h"
#include "ClientUIMDIConfigDialog.h"
#include <wx/filename.h>
#include "ClientUIMDIPasswordManagerDialog.h"
#include "HotKeyControl.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
	#include <wx/msw/private.h>
#endif

DECLARE_APP(DirtApp)

#include "res/dirt.xpm"
#include "res/blank.xpm"

enum
{
	ID_FILE_EXIT = 1,
	ID_TOOLS_PASSWORDS,
	ID_TOOLS_OPTIONS,
	ID_HELP_ABOUT,
	ID_FOCUSTIMER,
	ID_TRAY,
	ID_TRAYTIMER,
	ID_RESTORE,
	ID_CTRL_F,
	ID_CONFIG,
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
	EVT_MENU(ID_TOOLS_PASSWORDS, ClientUIMDIFrame::OnToolsPasswords)
	EVT_MENU(ID_TOOLS_OPTIONS, ClientUIMDIFrame::OnToolsOptions)
	EVT_MENU(ID_FILE_EXIT, ClientUIMDIFrame::OnFileExit)
	EVT_TIMER(ID_FOCUSTIMER, ClientUIMDIFrame::OnFocusTimer)
	EVT_ACTIVATE(ClientUIMDIFrame::OnActivate)
	EVT_TRAYICON_LEFT_DCLICK(ID_TRAY, ClientUIMDIFrame::OnTrayDblClick)
	EVT_TRAYICON_RIGHT_UP(ID_TRAY, ClientUIMDIFrame::OnTrayRightClick)
	EVT_ICONIZE(ClientUIMDIFrame::OnIconize)
	EVT_MENU(ID_RESTORE, ClientUIMDIFrame::OnRestore)
	EVT_TIMER(ID_TRAYTIMER, ClientUIMDIFrame::OnTrayTimer)
	EVT_MENU(ID_CTRL_F, ClientUIMDIFrame::OnCtrlF)
	EVT_MENU_RANGE(ID_BINDING_F1, ID_BINDING_F12, ClientUIMDIFrame::OnBinding)
	EVT_CLOSE(ClientUIMDIFrame::OnClose)
	EVT_CONFIG_FILE_CHANGED(ID_CONFIG, ClientUIMDIFrame::OnConfigFileChanged)
END_EVENT_TABLE()

ClientUIMDIFrame::ClientUIMDIFrame()
	: SwitchBarParent(NULL, -1, AppTitle(wxT("Client")),
		wxPoint(-1, -1), wxSize(500, 400),
		wxDEFAULT_FRAME_STYLE, wxT("Dirt"))
{

	m_client = NULL;
	m_tray = NULL;
	m_focused = true;
	m_alert = false;
	m_flash = 0;
	m_log_date_okay = false;
	UpdateCaption();

	SetIcon(wxIcon(dirt_xpm));

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, wxT("E&xit\tAlt-F4"));
	mnu->Append(mnuFile, wxT("&File"));

	wxMenu *mnuTools = new wxMenu;
	mnuTools->Append(ID_TOOLS_PASSWORDS, wxT("&Password Manager..."));
	mnuTools->Append(ID_TOOLS_OPTIONS, wxT("&Options..."));
	mnu->Append(mnuTools, wxT("&Tools"));

	mnu->Append(GetWindowMenu(), wxT("&Window"));

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, wxT("&About"));
	mnu->Append(mnuHelp, wxT("&Help"));

	SetMenuBar(mnu);

	m_client = new ClientDefault(this);

	m_client->GetConfig().SetEventHandler(this, ID_CONFIG);

	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxT("[Main]"), ChannelCanvas);
	NewWindow(canvas, true);
	m_lstNickList = canvas->GetNickList();

	int nicklist_width = canvas->GetNickList()->GetParent()->GetSize().x;
	wxConfigBase *config = GetClient()->GetConfig().GetConfig();
	config->Read(wxT("/Client/WindowState/NickList"), &nicklist_width, nicklist_width);
	canvas->GetNickList()->GetParent()->SetSize(nicklist_width, -1);
	canvas->ResizeChildren();

#ifdef __WXMSW__
	m_hotkey_keycode = 0;
	m_hotkey_mods = 0;
#endif
	SetHotKey();

	ResetWindowPos();
	RestoreWindowState(this, m_client->GetConfig().GetConfig(), wxT("Client"));

	Show();

	m_tmrFocus = new wxTimer(this, ID_FOCUSTIMER);
	m_tmrFocus->Start(100);

	m_tmrTray = new wxTimer(this, ID_TRAYTIMER);

	size_t num_bindings = 12;
	size_t num_accel = 1;
	wxAcceleratorEntry *entries = new wxAcceleratorEntry[GetAcceleratorCount()+num_bindings+num_accel];
	entries[0].Set(wxACCEL_CTRL, 'F', ID_CTRL_F);
	for (size_t i = 0; i < 12; ++i)
	{
		entries[i+num_accel].Set(0, WXK_F1+i, ID_BINDING_F1+i);
	}
	for (size_t i = num_bindings; i < GetAcceleratorCount()+num_bindings; ++i)
	{
		entries[i+num_accel] = GetAccelerators()[i-num_bindings];
	}
	wxAcceleratorTable accel(GetAcceleratorCount()+num_bindings+num_accel, entries);
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

void ClientUIMDIFrame::OnFocusTimer(wxTimerEvent &WXUNUSED(event))
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

void ClientUIMDIFrame::OnFileExit(wxCommandEvent &WXUNUSED(event))
{
	Close();
}

void ClientUIMDIFrame::OnToolsPasswords(wxCommandEvent &WXUNUSED(event))
{
	ClientUIMDIPasswordManagerDialog dlg(this);
}

void ClientUIMDIFrame::OnToolsOptions(wxCommandEvent &WXUNUSED(event))
{
	ClientUIMDIConfigDialog dlg(this);
	if (dlg.ShowModal() == wxID_OK)
	{
		SetHotKey();
		UpdateCaption();
	}
}

void ClientUIMDIFrame::OnHelpAbout(wxCommandEvent &WXUNUSED(event))
{
	ShowAbout();
}

void ClientUIMDIFrame::OnTrayDblClick(wxMouseEvent &WXUNUSED(event))
{
	RestoreFromTray();
}

void ClientUIMDIFrame::OnRestore(wxCommandEvent &WXUNUSED(event))
{
	RestoreFromTray();
}

void ClientUIMDIFrame::RestoreFromTray()
{
	ForceForegroundWindow(this);
	m_tmrTray->Stop();
	delete m_tray;
	m_tray = NULL;
}

void ClientUIMDIFrame::OnTrayTimer(wxTimerEvent &WXUNUSED(event))
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
		SetDefaultMenuItem(mnu, ID_RESTORE);
		m_tray->PopupMenu(&mnu, event.GetPosition());
	}
}

bool ClientUIMDIFrame::MinToTray()
{
	if (!m_tray)
	{
		m_tray = new TrayIcon;
		m_tray_flash = false;
		m_tray_auto_restore = wxGetApp().IsShiftDown();
		if (m_tray->Ok())
		{
			m_tray->SetEventHandler(this, ID_TRAY);
			if (m_client->GetConfig().GetSystemTrayIconMode() ==
				ClientConfig::stimAlwaysBlank)
			{
				m_tray->SetIcon(blank_xpm);
			}
			else
			{
				m_tray->SetIcon(dirt_xpm);
			}
			m_tray->SetToolTip(ConvertTitleToToolTip(m_title));
			Show(false);
			return true;
		}
		else
		{
			delete m_tray;
			m_tray = NULL;
		}
	}
	return false;
}

void ClientUIMDIFrame::OnIconize(wxIconizeEvent &event)
{
	if (event.Iconized() && wxGetApp().IsControlDown())
	{
		if (!MinToTray())
		{
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

	ClientConfig &config = m_client->GetConfig();

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
		bFlashWindow = config.GetTaskbarNotification();
	}

	wxString text = GetShortTimestamp() + line;

	canvas->GetLog()->AddTextLine(text, line_colour, tmmParse, convert_urls, true, bAlert);
	LogWriter *log = canvas->GetLogWriter();
	if (log)
	{
		if (log->Ok())
		{
			log->AddText(text, line_colour, convert_urls);
		}
		else
		{
			if (!canvas->GetLogWriterWarningShown())
			{
				canvas->SetLogWriterWarningShown(true);
				OnClientWarning(context, wxT("Error writing log file"));
			}
		}
	}
	
	if (!suppress_alert)
	{
	
		if (bAlert)
		{
			DoAlert();
		}
		
		if (bFlashWindow)
		{
			DoFlashWindow();
		}

	}

}

void ClientUIMDIFrame::DoAlert()
{

	ClientConfig &config = m_client->GetConfig();

	if (!m_client->GetContactSelf() || !m_client->GetContactSelf()->IsAway())
	{

		switch (config.GetSoundType())
		{

			case Config::tsmDefault:
				wxBell();
				break;

			case Config::tsmCustom:
				{
					#if wxUSE_WAVE
						wxString filename = config.GetActualSoundFile();
						if (wxFileName(filename).FileExists())
						{
							m_wave.Create(filename, false);
							if (m_wave.IsOk() && m_wave.Play())
							{
								break;
							}
						}
					#endif
					wxBell();
				}
				break;

			case Config::tsmNone:
			default:
				break;

		}

	}

}

void ClientUIMDIFrame::DoFlashWindow()
{
	m_alert = true;
	#ifdef __WXMSW__
		wxLongLong_t now = GetMillisecondTicks();
		if (now > m_last_flash_window + 1000)
		{
			::FlashWindow((HWND)GetHandle(), TRUE);
			m_last_flash_window = now;
		}
	#else
		m_flash = 8;
	#endif
	if (m_tray)
	{
		UpdateCaption();
		if (m_tray_auto_restore)
		{
			wxMouseEvent event;
			OnTrayDblClick(event);
		}
		else if (!m_tmrTray->IsRunning() &&
			m_client->GetConfig().GetSystemTrayIconMode() == ClientConfig::stimFlash)
		{
			m_tmrTray->Start(500);
		}
	}
}

wxArrayString ClientUIMDIFrame::OnClientSupportedCommands()
{
	return SplitString(wxT("ABOUT CLEAR CLEARALL CLOSE EXIT NEW TEST QUERY RESETWINDOWPOS RUN LOGS LANLIST SERVERS MINTOTRAY"), wxT(" "));
}

bool ClientUIMDIFrame::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == wxT("CLEAR"))
	{
		GetContext(context)->GetLog()->Clear();
		return true;
	}
	else if (cmd == wxT("CLEARALL"))
	{
		for (int i = 0; i < m_switchbar->GetButtonCount(); ++i)
		{
			ClientUIMDICanvas *canvas = (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
			if (canvas->GetLog())
			{
				canvas->GetLog()->Clear();
			}
		}
		return true;
	}
	else if (cmd == wxT("CLOSE"))
	{
		ClientUIMDICanvas *canvas = GetContext(context);
		if (canvas->IsAttached())
		{
			canvas->GetParent()->Close();
		}
		return true;
	}
	else if (cmd == wxT("EXIT"))
	{
		if (m_client->IsConnected())
		{
			m_client->Quit(params.Length() ? params : wxGetApp().GetDefaultQuitMessage());
		}
		Close();
		return true;
	}
	else if (cmd == wxT("TEST"))
	{
		GetContext(context)->LogControlTest();
		return true;
	}
	else if (cmd == wxT("MINTOTRAY"))
	{
		MinToTray();
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
	else if (cmd == wxT("LOGS") || cmd == wxT("LANLIST"))
	{
		wxASSERT(wxTheApp->argc > 0);
		wxString param;
		param << ((cmd == wxT("LOGS")) ? wxT("--logs") : wxT("--lanlist"));
		const wxChar *argv[3];
		wxString self = GetSelf();
		argv[0] = self.c_str();
		argv[1] = param.c_str();
		argv[2] = NULL;
		::wxExecute((wxChar**)argv);
		return true;
	}
	else if (cmd == wxT("NEW"))
	{
		wxArrayString param_array = SplitQuotedString(params, wxT(" "));
		if (param_array.GetCount() && param_array[0u].Left(2) != wxT("--"))
		{
			param_array[0u].Prepend(wxT("--host="));
		}
		wxASSERT(wxTheApp->argc > 0);
		const wxChar **argv = new const wxChar*[param_array.GetCount()+2];
		wxString self = GetSelf();
		argv[0] = self.c_str();
		for (size_t i = 0; i < param_array.GetCount(); ++i)
		{
			argv[i+1] = param_array[i].c_str();
		}
		argv[param_array.GetCount()+1] = NULL;
		::wxExecute((wxChar**)argv);
		delete argv;
		return true;
	}
	else if (cmd == wxT("ABOUT"))
	{
		ShowAbout();
		return true;
	}
	else if (cmd == wxT("SERVERS"))
	{
		OpenBrowser(this, PUBLIC_LIST_URL, true);
		return true;
	}
	else if (cmd == wxT("RUN"))
	{
		if (!OpenExternalResource(this, params, false))
		{
			OnClientWarning(context, wxT("Error executing: ") + params);
		}
		return true;
	}
	else if (cmd == wxT("DCC"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		wxString dcc_cmd = ht.head.Upper();
		if (dcc_cmd == wxT("SEND"))
		{
			ht = SplitQuotedHeadTail(ht.tail);
			if (m_client->GetContact(ht.head) && ht.tail.Length() == 0)
			{
				wxFileDialog dlg(
					this, wxT("Send to ") + ht.head,
					m_client->GetConfig().GetLastSendDir(), wxEmptyString,
					wxFileSelectorDefaultWildcardStr,
					wxOPEN | wxMULTIPLE | wxHIDE_READONLY | wxFILE_MUST_EXIST);
				if (dlg.ShowModal() == wxID_OK)
				{
					wxArrayString paths;
					dlg.GetPaths(paths);
					for (size_t i = 0; i < paths.GetCount(); ++i)
					{
						m_client->ProcessConsoleInput(context,
							wxT("/DCC SEND \"") + ht.head + wxT("\" \"") +
							paths[i] + wxT("\""));
					}
				}
				return true;
			}
		}
		else if (dcc_cmd == wxT("ACCEPT") || dcc_cmd == wxT("RESUME") || dcc_cmd == wxT("OVERWRITE"))
		{
			ht = SplitQuotedHeadTail(ht.tail);
			if (ht.tail.Length() == 0)
			{
				long x;
				int i;
				if (ht.head.ToLong(&x) &&
					(i = m_client->GetFileTransfers()->FindTransfer(x)) > -1)
				{
					const FileTransfer &transfer =
						m_client->GetFileTransfers()->GetTransferByIndex(i);
					if (transfer.state == ftsGetPending)
					{
						wxFileDialog dlg(
							this, wxT("Get ") + transfer.filename + wxT(" from ") + transfer.nickname,
							m_client->GetConfig().GetLastGetDir(), transfer.filename,
							wxFileSelectorDefaultWildcardStr,
							wxSAVE);
						if (dlg.ShowModal() == wxID_OK)
						{
							m_client->ProcessConsoleInput(context,
								wxT("/DCC ") + dcc_cmd + wxT(" ") + ht.head + wxT(" \"") +
								dlg.GetPath() + wxT("\""));
						}
						return true;
					}
				}
			}
		}
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
		HeadTail ht = SplitHeadTail(text, wxT(": "));
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

void ClientUIMDIFrame::SetHotKey()
{

	if (!IsHotKeySupported()) return;
	
	int keycode[2];
	wxUint8 modifiers[2];
	
	for (int i = 0; i < 2; ++i)
	{
		keycode[i] = m_client->GetConfig().GetHotKey(i, false);
		modifiers[i] = m_client->GetConfig().GetHotKey(i, true);
	}

	if (!keycode[0] && !keycode[1])
	{
		SetHotKey(0, 0);
		return;
	}

	for (int i = 0; i < 2; ++i)
	{
		if (keycode[i] && modifiers[i])
		{
			if (SetHotKey(keycode[i], modifiers[i]))
			{
				return;
			}
		}
	}
	
}

void ClientUIMDIFrame::OnHotKey()
{
	if (IsFocused())
	{
		if (!MinToTray())
		{
			Iconize();
		}
	}
	else
	{
		RestoreFromTray();
	}
}

bool ClientUIMDIFrame::IsHotKeySupported()
{
#ifdef __WXMSW__
	return true;
#else
	return false;
#endif
}

bool ClientUIMDIFrame::SetHotKey(int keycode, wxUint8 mods)
{

#ifdef __WXMSW__

	if (keycode == m_hotkey_keycode && mods == m_hotkey_mods)
	{
		return true;
	}

	if (m_hotkey_keycode || m_hotkey_mods)
	{
		if (::UnregisterHotKey(GetHwnd(), 1) == 0)
		{
			return false;
		}
		m_hotkey_keycode = 0;
		m_hotkey_mods = 0;
		UpdateCaption();
	}

	if (keycode && mods)
	{
		if (::RegisterHotKey(GetHwnd(), 1, mods, keycode) != 0)
		{
			m_hotkey_keycode = keycode;
			m_hotkey_mods = mods;
			UpdateCaption();
			return true;
		}
	}
	
	return (!keycode && !mods);

#else

	return (!keycode && !mods);

#endif

}

int ClientUIMDIFrame::GetHotKeyKeyCode() const
{
#ifdef __WXMSW__
	return m_hotkey_keycode;
#else
	return 0;
#endif
}

wxUint8 ClientUIMDIFrame::GetHotKeyModifiers() const
{
#ifdef __WXMSW__
	return m_hotkey_mods;
#else
	return 0;
#endif
}

void ClientUIMDIFrame::UpdateCaption()
{
	wxString title;
	if (m_alert && !IsWin32())
	{
		title << wxT("* ");
	}
	if (IsWin32() || !m_alert || (m_flash % 2) == 0)
	{
		title << AppTitle(wxT("Client"));
		int keycode = GetHotKeyKeyCode();
		wxUint8 mods = GetHotKeyModifiers();
		if (keycode && mods)
		{
			wxString hotkey = HotKeyControl::HotKeyToString(keycode, mods);
			hotkey.Replace(wxT(" "), wxT(""));
			title << wxT(" [") << hotkey << wxT("]");
		}
		if (m_client && m_client->IsConnected())
		{
			title << wxT(" - ");
			if (m_client->GetNickname().Length())
			{
				title << m_client->GetNickname();
			}
			else
			{
				title << wxT("<NoNick>");
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
				title << wxT(" (") << AddCommas((wxLongLong_t)m_client->GetLatency()) << wxT(" ms lag)");
			}
		}
	}
	m_title = title;
	if (m_tray)
	{
		m_tray->SetToolTip(ConvertTitleToToolTip(m_title));
	}
	SetTitle(title);
	#ifdef __WXMSW__
		if (m_alert)
		{
			::FlashWindow((HWND)GetHandle(), TRUE);
			m_last_flash_window = GetMillisecondTicks();
		}
	#endif
}

wxString ClientUIMDIFrame::ConvertTitleToToolTip(const wxString &title) const
{
	wxString tooltip;
	if (m_alert)
	{
		tooltip << wxT("* ");
	}
	int index = title.Find(wxT(" - "));
	if (index > -1)
	{
		tooltip << title.Left(index) << wxT("\n") << title.Mid(index+3);
	}
	else
	{
		tooltip << title;
	}
	return tooltip;
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
	if (!pass.Length())
	{
		pass = m_client->GetConfig().GetSavedPassword(m_client->GetServerName(), true);
	}
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

void ClientUIMDIFrame::InitLogs()
{
	for (int i = 0; i < m_switchbar->GetButtonCount(); ++i)
	{
		ClientUIMDICanvas *canvas =
			(ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
		canvas->InitLog();
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
	if (!nick.Length())
	{
		nick = m_client->GetLastNickname();
	}
	if (!nick.Length())
	{
		nick = m_client->GetConfig().GetNickname();
	}
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
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			AddLine(nick, wxT("* ") + m_client->GetNickname() + sep + text, colours[6], true, true);
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
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			AddLine(context, wxT("-> *") + nick + wxT("* * ") + m_client->GetNickname() + sep + text, colours[6], true, true);
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
		wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
		AddLine(context, wxT("* ") + nick + sep + text, colours[6], true, is_self);
	}
	else
	{
		AddLine(context, wxT("<") + nick + wxT("> ") + text, *wxBLACK, true, is_self);
	}
}

void ClientUIMDIFrame::OnClientCTCPIn(const wxString &WXUNUSED(context), const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT('[') << nick;
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	msg << wxT(']');
	if (data.Length())
	{
		msg << wxT(' ') << data;
	}
	AddLine(wxEmptyString, msg, *wxRED, true, true);
}

void ClientUIMDIFrame::OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT("-> [") << nick << wxT(']');
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	if (data.Length())
	{
		msg << wxT(' ') << data;
	}
	AddLine(context, msg, *wxRED, true, true);
}

void ClientUIMDIFrame::OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT('[') << nick;
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	msg << wxT(" reply]");
	if (data.Length())
	{
		msg << wxT(": ") << data;
	}
	AddLine(context, msg, *wxRED, false, false);
}

void ClientUIMDIFrame::OnClientCTCPReplyOut(const wxString &WXUNUSED(context), const wxString &WXUNUSED(nick), const wxString &WXUNUSED(type), const ByteBuffer &WXUNUSED(data))
{
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

void ClientUIMDIFrame::OnClientUserAway(const wxString &nick, const wxString &msg, long WXUNUSED(away_time), long away_time_diff)
{
	bool bIsSelf = (nick == m_client->GetNickname());
	wxString text;
	text << wxT("*** ") << nick;
	if (away_time_diff > 0)
	{
		text << wxT(" has been away for ") << SecondsToMMSS(away_time_diff);
	}
	else
	{
		text << wxT(" is away");
	}
	if (msg.Length())
	{
		text << wxT(": ") << msg;
	}
	AddLine(wxEmptyString, text, wxColour(0,0,128), true, bIsSelf);
	m_lstNickList->SetAway(nick, true);
}

void ClientUIMDIFrame::OnClientUserBack(const wxString &nick, const wxString &msg, long WXUNUSED(away_time), long away_time_diff)
{
	bool bIsSelf = (nick == m_client->GetNickname());
	wxString text;
	text << wxT("*** ") << nick << wxT(" has returned");
	if (msg.Length())
	{
		text << wxT(" (msg: ") << msg << (wxChar)OriginalModifier << wxT(")");
	}
	if (away_time_diff >= 0)
	{
		text << wxT(" (away for ") << SecondsToMMSS(away_time_diff) << wxT(")");
	}
	AddLine(wxEmptyString, text, wxColour(0,0,128), true, bIsSelf);
	m_lstNickList->SetAway(nick, false);
}

void ClientUIMDIFrame::OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details)
{
	ByteBufferHashMap details2(details);
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
	if (details2.find(wxT("AWAYTIMEDIFFSTRING")) != details2.end())
	{
		AddLine(context, nickname + wxT(" has been away for ") + details2[wxT("AWAYTIMEDIFFSTRING")]);
	}
	AddLine(context, nickname + wxT(" is using ") + details2[wxT("AGENT")]);
	AddLine(context, nickname + wxT(" has been idle for ") + details2[wxT("IDLESTRING")] + wxT(" (") + details2[wxT("LATENCYSTRING")] + wxT(" lag)"));
	AddLine(context, nickname + wxT(" signed on at ") + details2[wxT("JOINTIMESTRING")]);
	details2.erase(wxT("NICK"));
	details2.erase(wxT("DETAILS"));
	details2.erase(wxT("HOSTNAME"));
	details2.erase(wxT("ISADMIN"));
	details2.erase(wxT("AWAY"));
	details2.erase(wxT("AWAYTIME"));
	details2.erase(wxT("AWAYTIMEDIFF"));
	details2.erase(wxT("AWAYTIMEDIFFSTRING"));
	details2.erase(wxT("AGENT"));
	details2.erase(wxT("IDLE"));
	details2.erase(wxT("IDLESTRING"));
	details2.erase(wxT("LATENCY"));
	details2.erase(wxT("LATENCYSTRING"));
	details2.erase(wxT("JOINTIME"));
	details2.erase(wxT("JOINTIMESTRING"));
	for (ByteBufferHashMap::iterator i = details2.begin(); i != details2.end(); ++i)
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
	NewWindow(canvas, false);
}

void ClientUIMDIFrame::OnClientTransferDelete(const FileTransfer &transfer, bool user_initiated)
{
	ClientUIMDITransferPanel *pnl = GetContext(transfer.transferid);
	if (pnl)
	{
		pnl->SetTransferId(-1);
		int index = GetSwitchBar()->GetIndexFromUserData(pnl->GetParent());
		if (index > -1)
		{
			GetSwitchBar()->SetButtonHighlight(index, true);
		}
		if (user_initiated)
		{
			CloseCanvas((SwitchBarCanvas*)pnl->GetParent());
		}
	}
}

void ClientUIMDIFrame::OnClientTransferState(const FileTransfer &transfer)
{

	if (m_client->GetConfig().GetFileTransferStatus())
	{

		bool bIsError = ((transfer.state == ftsSendFail) || (transfer.state == ftsGetFail));
		
		wxColour colour = bIsError ? *wxRED : wxColour(0,0,128);
		
		bool bAlert;
		switch (transfer.state)
		{

			case ftsSendComplete:
			case ftsSendFail:
			case ftsGetPending:
			case ftsGetComplete:
			case ftsGetFail:
				bAlert = true;
				break;

			default:
				bAlert = false;
				break;

		}

		wxString text;
		text << wxT("*** ") << transfer.GetPrefixString() << transfer.status;

		AddLine(wxEmptyString, text, colour, true, !bAlert, false);

	}
	else
	{

		if (!IsFocused())
		{
			DoAlert();
			DoFlashWindow();
		}

	}
	
	OnClientTransferTimer(transfer);

}

void ClientUIMDIFrame::OnClientTransferTimer(const FileTransfer &transfer)
{
	ClientUIMDITransferPanel *pnl = GetContext(transfer.transferid);
	wxASSERT(pnl);
	pnl->Update(transfer);
}

ResumeState ClientUIMDIFrame::OnClientTransferResumePrompt(const FileTransfer &transfer, const wxString &new_filename, bool can_resume)
{
	ClientUIMDITransferResumeDialog dlg(this, transfer.nickname, transfer.filename, new_filename, can_resume);
	return (ResumeState)dlg.ShowModal();
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
				wxString() :
				child->GetCanvas()->GetTitle();
			m_client->ProcessAlias(context, m_client->GetBinding(name), wxEmptyString);
			return;
		}
	}
}

void ClientUIMDIFrame::OnCtrlF(wxCommandEvent &WXUNUSED(event))
{
	SwitchBarChild *child = (SwitchBarChild*)GetActiveChild();
	if (child)
	{
		ClientUIMDICanvas *canvas = (ClientUIMDICanvas*)child->GetCanvas();
		LogControl *txtLog = canvas->GetLog();
		txtLog->ShowFindDialog(true);
	}
}

wxDateTime ClientUIMDIFrame::GetLogDate()
{
	if (!m_log_date_okay)
	{
		wxString log_dir = m_client->GetConfig().GetActualLogDir();
		m_log_date = LogWriter::GenerateNewLogDate(log_dir, wxT("Client"));
		m_log_date_okay = true;
	}
	return m_log_date;
}

void ClientUIMDIFrame::OnConfigFileChanged(wxCommandEvent &WXUNUSED(event))
{
	SetHotKey();
}

void ClientUIMDIFrame::OnClose(wxCloseEvent &event)
{

	if (m_client)
	{

		SaveWindowState(this, m_client->GetConfig().GetConfig(), wxT("Client"));

		ClientUIMDICanvas *canvas = GetContext(wxEmptyString);
		int nicklist_width = canvas->GetNickList()->GetParent()->GetSize().x;
		wxConfigBase *config = GetClient()->GetConfig().GetConfig();
		config->Write(wxT("/Client/WindowState/NickList"), nicklist_width);

		if (m_client->IsConnected())
		{
			wxString msg = wxGetApp().GetDefaultQuitMessage();
			if (!msg.Length())
			{
				msg = wxT("Closing");
			}
			m_client->Quit(msg);
		}

	}

	event.Skip();

}

#ifdef __WXMSW__
	long ClientUIMDIFrame::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
	{
		// The following is a hook for PowerMenu's "Minimize To Tray" system menu option
		if (nMsg == WM_INITMENUPOPUP)
		{
			HMENU hMenu = GetSystemMenu((HWND)GetHandle(), FALSE);
			if (hMenu == (HMENU)wParam)
			{
				MENUITEMINFO mii;
				mii.cbSize = sizeof (MENUITEMINFO);
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
				MinToTray();
				return 0;
			}
		}
		else if (nMsg == WM_HOTKEY)
		{
			OnHotKey();
			return 0;
		}
		return SwitchBarParent::MSWWindowProc(nMsg, wParam, lParam);
	}
#endif
