#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Dirt.cpp,v 1.15 2003-02-15 03:39:34 jason Exp $)

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

	BOOL ReadBytes(HANDLE hFile, LPVOID buffer, DWORD size)
	{
		DWORD bytes;
		return (ReadFile(hFile, buffer, size, &bytes, NULL) && size == bytes);
	} 

	WORD GetImageSubsystemType(char *filename)
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

		char filename[MAX_PATH];
		::GetModuleFileName(NULL, filename, MAX_PATH);
		int subsystem = GetImageSubsystemType(filename);
		wxASSERT(subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI || subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
		bIsConsole = (subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);

	#else

		bIsConsole = false; // (default to GUI for non-Win32)

	#endif

	#ifdef __WXMSW__

		#define CANT_CHANGE_MODE \
			"Note: Changing between console and GUI mode in Win32 " \
			"requires changes to the binary.\nThe command line option " \
			"that was specified to change this mode has been ignored.\n"

		if (cmdline->Found("g") && bIsConsole)
		{
			puts(CANT_CHANGE_MODE);
		}
		else if (cmdline->Found("c") && !bIsConsole)
		{
			wxMessageBox(CANT_CHANGE_MODE, "Unsupported Option", wxICON_ERROR);
		}

	#else

		if (cmdline->Found("g"))
		{
			bIsConsole = false;
		}
		else if (cmdline->Found("c"))
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
	if (!ProcessCommandLine())
	{
		return false;
	}
	
	if (IsConsole())
	{

		switch (m_appmode)
		{

			case appClient:
			case appDefault:
				{
					ClientUIConsole *cui = new ClientUIConsole;
					m_console = cui;
					c = cui->GetClient();
				}
				break;

			case appServer:
				m_console = new ServerUIConsole;
				break;

			default:
				wxFAIL_MSG("Unknown AppMode");
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
				wxFAIL_MSG("Unknown AppMode");
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
	delete m_console;
	delete cmdline;
	return wxApp::OnExit();
}

bool DirtApp::ProcessCommandLine()
{

	cmdline = new wxCmdLineParser(argc, argv);

	cmdline->AddSwitch("c", "console", "Console Mode", 0);
	cmdline->AddSwitch("g", "gui", "GUI Mode", 0);
	cmdline->AddSwitch("l", "client", "Client Mode", 0);
	cmdline->AddSwitch("s", "server", "Server Mode", 0);
	cmdline->AddOption("h", "host", "Remote host to connect to (implies client)", wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR | wxCMD_LINE_PARAM_OPTIONAL);

	if (cmdline->Parse() != 0)
	{
		return false;
	}

	if (cmdline->Found("server"))
	{
		m_appmode = appServer;
	}
	else if (cmdline->Found("l"))
	{
		m_appmode = appClient;
	}
	else
	{
		m_appmode = appDefault;
	}

	if (cmdline->Found("host", &m_host))
	{
		if (m_appmode == appDefault)
		{
			m_appmode = appClient;
		}
		else if (m_appmode != appClient)
		{
			cmdline->Usage();
			return false;
		}
	}

	return true;

}
