#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Dirt.cpp,v 1.24 2003-03-05 10:24:23 jason Exp $)

#include "Dirt.h"
#include "ClientUIConsole.h"
#include "ServerUIConsole.h"
#include "ClientUIMDIFrame.h"
#include "ServerUIFrame.h"
#include "Splash.h"
#include <stdio.h>
#include <wx/cmdline.h>

IMPLEMENT_APP(DirtApp)

#ifdef __WIN32__

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
			wxT("Note: Changing between console and GUI mode in Win32 " \
			"requires changes to the binary.\nThe command line option " \
			"that was specified to change this mode has been ignored.\n")

		if (m_cmdline->Found(wxT("gui")) && bIsConsole)
		{
			puts(CANT_CHANGE_MODE);
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
	m_console = NULL;
	m_cmdline = NULL;

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

			default:
				wxFAIL_MSG(wxT("Unknown AppMode"));
				return false;

		}
		

	}

	if (m_host.Length() > 0)
	{
		c->Connect(m_host);
	}

	return true;

}

int DirtApp::OnExit()
{

	#ifdef __WXMSW__
		::timeEndPeriod(1);
	#endif

	delete m_console;
	delete m_cmdline;
	return wxApp::OnExit();

}

bool DirtApp::ProcessCommandLine()
{

	m_cmdline = new wxCmdLineParser(argc, argv);

	m_cmdline->AddSwitch(wxT("c"), wxT("console"), wxT("Console Mode"), 0);
	m_cmdline->AddSwitch(wxT("g"), wxT("gui"), wxT("GUI Mode"), 0);
	m_cmdline->AddSwitch(wxT("l"), wxT("client"), wxT("Client Mode"), 0);
	m_cmdline->AddSwitch(wxT("s"), wxT("server"), wxT("Server Mode"), 0);
	m_cmdline->AddOption(wxT("h"), wxT("host"), wxT("Remote host to connect to (implies client)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR | wxCMD_LINE_PARAM_OPTIONAL);
	m_cmdline->AddSwitch(wxT("n"), wxT("no-input"), wxT("Disable reading from the console (for background tasks, applies to console only)"), 0);

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
	}

	m_no_input = m_cmdline->Found("no-input");

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

#endif

void DirtApp::RegisterDirtProtocol()
{

	#ifdef __WXMSW__

		wxString exe = GetEXEName();
		int num_actions = 0;
		wxRegKey reg;
		reg.SetName(wxRegKey::HKCR, wxT("dirt"));
		num_actions += !!reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxT("URL:Dirt Secure Chat Protocol"));
		num_actions += !!MaybeSet(reg, wxT("EditFlags"), 2);
		num_actions += !!MaybeSet(reg, wxT("URL Protocol"), wxEmptyString);
		reg.SetName(wxRegKey::HKCR, wxT("dirt\\DefaultIcon"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, exe + wxT(",0"));
		reg.SetName(wxRegKey::HKCR, wxT("dirt\\shell\\open\\command"));
		reg.Create(false);
		num_actions += !!MaybeSet(reg, NULL, wxString() << wxT("\"") << exe << wxT("\" --host=\"%1\""));
		if (num_actions)
		{
			SendMessageTimeout(HWND_BROADCAST, WM_WININICHANGE, 0, 0, 0, 500, 0);
		}

	#else

		// only available on Win32

	#endif

}
