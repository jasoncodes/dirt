#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Dirt.cpp,v 1.41 2003-04-30 02:57:27 jason Exp $)

#include "Dirt.h"
#include "ClientUIConsole.h"
#include "ServerUIConsole.h"
#include "ClientUIMDIFrame.h"
#include "ServerUIFrame.h"
#include "LogViewerFrame.h"
#include "LanListFrame.h"
#include "Splash.h"
#include <stdio.h>
#include <wx/cmdline.h>

IMPLEMENT_APP(DirtApp)

#ifdef __WIN32__

	#include <windows.h>
	#include <wx/msw/winundef.h>

	// Why is it that people don't run late versions of Platform SDK? :)
	#ifndef INVALID_SET_FILE_POINTER
		#define INVALID_SET_FILE_POINTER ((DWORD)-1)
	#endif

	BOOL ReadBytes(HANDLE hFile, LPVOID buffer, DWORD size)
	{
		DWORD bytes;
		return (ReadFile(hFile, buffer, size, &bytes, NULL) && size == bytes);
	} 

	WORD GetImageSubsystemType(wxChar *filename)
	{ 

		HANDLE hImage;

		DWORD  SectionOffset;
		DWORD  CoffHeaderOffset;
		DWORD  MoreDosHeader[16];

		ULONG  ntSignature;

		IMAGE_DOS_HEADER      image_dos_header;
		IMAGE_FILE_HEADER     image_file_header;
		IMAGE_OPTIONAL_HEADER image_optional_header;

		DWORD offset;

		hImage = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == hImage)
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		if (!ReadBytes(hImage, &image_dos_header, sizeof(IMAGE_DOS_HEADER)))
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		if (IMAGE_DOS_SIGNATURE != image_dos_header.e_magic)
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		if (!ReadBytes(hImage, MoreDosHeader, sizeof(MoreDosHeader)))
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		offset = SetFilePointer(hImage, image_dos_header.e_lfanew, NULL, FILE_BEGIN);

		if (offset == INVALID_SET_FILE_POINTER)
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		CoffHeaderOffset = offset + sizeof(ULONG);

		if (!ReadBytes(hImage, &ntSignature, sizeof(ULONG)))
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		if (IMAGE_NT_SIGNATURE != ntSignature)
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		SectionOffset = CoffHeaderOffset + IMAGE_SIZEOF_FILE_HEADER + IMAGE_SIZEOF_NT_OPTIONAL_HEADER;

		if (!ReadBytes(hImage, &image_file_header, IMAGE_SIZEOF_FILE_HEADER))
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		if (!ReadBytes(hImage, &image_optional_header, IMAGE_SIZEOF_NT_OPTIONAL_HEADER))
		{
			return IMAGE_SUBSYSTEM_UNKNOWN;
		}

		return image_optional_header.Subsystem;

	}

#endif

bool DirtApp::IsConsole()
{

	bool bIsConsole;

	#ifdef __WXMSW__

		wxChar filename[MAX_PATH];
		::GetModuleFileName(NULL, filename, MAX_PATH);
		int subsystem = GetImageSubsystemType(filename);
		wxASSERT(subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI || subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
		bIsConsole = (subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);

	#else

		bIsConsole = false; // (default to GUI for non-Win32)

	#endif

	#ifdef __WXMSW__

		#define CANT_CHANGE_MODE \
			wxT("Note: Changing between console and GUI mode in Win32 ") \
			wxT("requires changes to the binary.\nThe command line option ") \
			wxT("that was specified to change this mode has been ignored.\n")

		if (m_cmdline->Found(wxT("gui")) && bIsConsole)
		{
			wxPuts(CANT_CHANGE_MODE);
		}
		else if (m_cmdline->Found(wxT("console")) && !bIsConsole)
		{
			wxMessageBox(CANT_CHANGE_MODE, wxT("Unsupported Option"), wxICON_ERROR);
		}

	#else

		if (m_cmdline->Found(wxT("gui")))
		{
			bIsConsole = false;
		}
		else if (m_cmdline->Found(wxT("console")))
		{
			bIsConsole = true;
		}

	#endif

	return bIsConsole;

}

bool DirtApp::OnInit()
{

	Client *c = NULL;
	LogViewerFrame *l = NULL;
	m_console = NULL;
	m_cmdline = NULL;
	m_control_down = false;
	m_alt_down = false;
	m_shift_down = false;

	#ifdef __WXMSW__
		::timeBeginPeriod(1);
	#endif

	if (!ProcessCommandLine())
	{
		return false;
	}

	RegisterDirtProtocol();

	if (IsConsole())
	{

		switch (m_appmode)
		{

			case appClient:
			case appDefault:
				{
					ClientUIConsole *cui = new ClientUIConsole(m_no_input);
					m_console = cui;
					c = cui->GetClient();
				}
				break;

			case appServer:
				m_console = new ServerUIConsole(m_no_input);
				break;

			case appLog:
			case appLanList:
				m_cmdline->Usage();
				return false;

			default:
				wxFAIL_MSG(wxT("Unknown AppMode"));
				return false;

		}

	}
	else
	{

		switch (m_appmode)
		{

			case appDefault:
				new Splash;
				break;

			case appClient:
				{
					ClientUIMDIFrame *cui = new ClientUIMDIFrame;
					c = cui->GetClient();
				}
				break;

			case appServer:
				new ServerUIFrame;
				break;

			case appLog:
				l = new LogViewerFrame;
				break;

			case appLanList:
				new LanListFrame;
				break;

			default:
				wxFAIL_MSG(wxT("Unknown AppMode"));
				return false;

		}
		

	}

	if (c && m_host.Length())
	{
		c->Connect(m_host);
	}
	if (l && m_logfile.length())
	{
		l->ViewLogFile(m_logfile);
	}

	#ifdef __WXMSW__
		s_hMsgHookProc = SetWindowsHookEx(WH_GETMESSAGE, &DirtApp::MsgHookProc, NULL, GetCurrentThreadId());
	#endif

	return true;

}

int DirtApp::OnExit()
{

	#ifdef __WXMSW__
		UnhookWindowsHookEx(s_hMsgHookProc);
	#endif

	#ifdef __WXMSW__
		::timeEndPeriod(1);
	#endif

	delete m_console;
	delete m_cmdline;
	return wxApp::OnExit();

}

#ifdef __WXMSW__

HHOOK DirtApp::s_hMsgHookProc;

LRESULT CALLBACK DirtApp::MsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG*)lParam;
	switch (msg->message)
	{
		case WM_NULL:
			wxTheApp->ProcessPendingEvents();
			break;
	}
	return CallNextHookEx(s_hMsgHookProc, nCode, wParam, lParam);
}

#endif


bool DirtApp::ProcessCommandLine()
{

	m_cmdline = new wxCmdLineParser(argc, argv);

	m_cmdline->AddSwitch(wxEmptyString, wxT("gui"), wxT("GUI Mode"), 0);
	m_cmdline->AddSwitch(wxEmptyString, wxT("console"), wxT("Console Mode"), 0);
	m_cmdline->AddSwitch(wxEmptyString, wxT("no-input"), wxT("Disable reading from the console (for background tasks, applies to console only)"), 0);
	m_cmdline->AddSwitch(wxEmptyString, wxT("server"), wxT("Server Mode"), 0);
	m_cmdline->AddSwitch(wxEmptyString, wxT("client"), wxT("Client Mode"), 0);
	m_cmdline->AddOption(wxEmptyString, wxT("host"), wxT("Remote host to connect to (implies client)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR | wxCMD_LINE_PARAM_OPTIONAL);
	m_cmdline->AddSwitch(wxEmptyString, wxT("logs"), wxT("Log Mode (GUI only)"), 0);
	m_cmdline->AddOption(wxEmptyString, wxT("log"), wxT("Log file to view (implies log mode)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR | wxCMD_LINE_PARAM_OPTIONAL);
	m_cmdline->AddSwitch(wxEmptyString, wxT("lanlist"), wxT("LAN List (GUI only)"), 0);
	m_cmdline->AddOption(wxEmptyString, wxT("config"), wxT("Alternate config filename"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR | wxCMD_LINE_PARAM_OPTIONAL);

	if (m_cmdline->Parse() != 0)
	{
		return false;
	}

	if (m_cmdline->Found(wxT("server")))
	{
		m_appmode = appServer;
	}
	else if (m_cmdline->Found(wxT("client")))
	{
		m_appmode = appClient;
	}
	else if (m_cmdline->Found(wxT("logs")))
	{
		m_appmode = appLog;
	}
	else if (m_cmdline->Found(wxT("lanlist")))
	{
		m_appmode = appLanList;
	}
	else
	{
		m_appmode = appDefault;
	}

	if (m_cmdline->Found(wxT("host"), &m_host))
	{
		if (m_appmode == appDefault)
		{
			m_appmode = appClient;
		}
		else if (m_appmode != appClient)
		{
			m_cmdline->Usage();
			return false;
		}
		if (!m_host.Length())
		{
			m_cmdline->Usage();
			return false;
		}
	}

	if (m_cmdline->Found(wxT("log"), &m_logfile))
	{
		if (m_appmode == appDefault)
		{
			m_appmode = appLog;
		}
		else if (m_appmode != appLog)
		{
			m_cmdline->Usage();
			return false;
		}
		if (!m_logfile.Length())
		{
			m_cmdline->Usage();
			return false;
		}
	}

	m_no_input = m_cmdline->Found(wxT("no-input"));

	if (m_cmdline->Found(wxT("config"), &m_config_filename))
	{
		if (!m_config_filename.Length())
		{
			m_cmdline->Usage();
			return false;
		}
	}
	else
	{
		m_config_filename = wxEmptyString;
	}

	return true;

}

#ifdef __WXMSW__

	#include <wx/msw/registry.h>

	wxString GetEXEName()
	{
		wxChar path[MAX_PATH];
		GetModuleFileName(NULL, path, MAX_PATH);
		return path;
	}

	static bool MaybeSet(wxRegKey &reg, const wxChar *key, const wxString& value)
	{
		wxString val;
		if (!reg.HasValue(key) || !reg.QueryValue(key, val) || value != val)
		{
			return reg.SetValue(key, value);
		}
		return false;
	}

	static bool MaybeSet(wxRegKey &reg, const wxChar *key, long value)
	{
		long val;
		if (!reg.HasValue(key) || !reg.QueryValue(key, &val) || value != val)
		{
			return reg.SetValue(key, value);
		}
		return false;
	}

#else

	#include <wx/filename.h>
	#include <wx/wfstream.h>
	#include <wx/txtstrm.h>

#endif

void DirtApp::RegisterDirtProtocol()
{

	#ifdef __WXMSW__

		wxLogNull supress_log;
		wxString exe = GetEXEName();
		int num_actions = 0;
		wxRegKey reg;
		reg.SetName(wxRegKey::HKCR, wxT("dirt"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxT("URL:Dirt Secure Chat Protocol"));
		num_actions += !!MaybeSet(reg, wxT("EditFlags"), 2);
		num_actions += !!MaybeSet(reg, wxT("URL Protocol"), wxEmptyString);
		reg.SetName(wxRegKey::HKCR, wxT("dirt\\DefaultIcon"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, exe + wxT(",0"));
		reg.SetName(wxRegKey::HKCR, wxT("dirt\\shell\\open\\command"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxString() << wxT("\"") << exe << wxT("\" --host=\"%1\""));
		reg.SetName(wxRegKey::HKCR, wxT(".dirtlog"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxT("DirtSecureChat.LogFile"));
		reg.SetName(wxRegKey::HKCR, wxT("DirtSecureChat.LogFile"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxT("Dirt Secure Chat Log File"));
		reg.SetName(wxRegKey::HKCR, wxT("DirtSecureChat.LogFile\\shell"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxT("open"));
		reg.SetName(wxRegKey::HKCR, wxT("DirtSecureChat.LogFile\\shell\\open\\command"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxString() << wxT("\"") << exe << wxT("\" --log=\"%1\""));

		if (num_actions)
		{
			SendMessageTimeout(HWND_BROADCAST, WM_WININICHANGE, 0, 0, 0, 500, 0);
		}

	#else

	wxFileName kde_dirt_protocol(wxFileName::GetHomeDir() + wxT("/.kde/share/services/"));
	if (kde_dirt_protocol.DirExists())
	{
		kde_dirt_protocol.SetFullName(wxT("dirt.protocol"));
		wxChar self[4096];
		if (realpath(argv[0], self))
		{
			wxFileOutputStream file(kde_dirt_protocol.GetFullPath());
			wxTextOutputStream text(file);
			text << wxT("[Protocol]\n");
			text << wxT("exec=") << self << wxT(" --host=%u\n");
			text << wxT("protocol=dirt\n");
			text << wxT("input=none\n");
			text << wxT("output=none\n");
			text << wxT("helper=true\n");
			text << wxT("listing=false\n");
			text << wxT("reading=false\n");
			text << wxT("writing=false\n");
			text << wxT("makedir=false\n");
			text << wxT("deleting=false\n");
		}
	}
	
	#endif

}

int DirtApp::FilterEvent(wxEvent& event)
{

	wxEventType type = event.GetEventType();

	if (type == wxEVT_KEY_DOWN || type == wxEVT_KEY_UP)
	{

		wxKeyEvent &key_event = (wxKeyEvent&)event;
		m_control_down = key_event.ControlDown() | (key_event.GetKeyCode() == WXK_CONTROL);
		m_alt_down = key_event.AltDown() | (key_event.GetKeyCode() == WXK_ALT);
		m_shift_down = key_event.ShiftDown() | (key_event.GetKeyCode() == WXK_SHIFT);

	}
	else if (type == wxEVT_ACTIVATE || type == wxEVT_ACTIVATE_APP)
	{

		m_control_down = false;
		m_alt_down = false;
		m_shift_down = false;

	}
	else if (type == wxEVT_QUERY_END_SESSION || type == wxEVT_END_SESSION)
	{

		wxCloseEvent &close_event = (wxCloseEvent&)event;

		if (close_event.GetLoggingOff())
		{
			m_default_quit_message = wxT("Logging Off");
		}
		else
		{
			m_default_quit_message = wxT("Session Ending");
		}

	}

	return -1;

}
