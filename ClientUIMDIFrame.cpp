#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIMDIFrame.h"
#include "SwitchBarChild.h"
#include "ClientUIMDICanvas.h"
#include "ClientDefault.h"
#include "util.h"
#include "NickListControl.h"
#include "Modifiers.h"

#include "res/dirt.xpm"

enum
{
	ID_FILE_EXIT = 1,
	ID_HELP_ABOUT,
	ID_FOCUSTIMER
};

BEGIN_EVENT_TABLE(ClientUIMDIFrame, SwitchBarParent)
	EVT_MENU(ID_HELP_ABOUT, ClientUIMDIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ClientUIMDIFrame::OnFileExit)
	EVT_TIMER(ID_FOCUSTIMER, ClientUIMDIFrame::OnFocusTimer)
	EVT_ACTIVATE(ClientUIMDIFrame::OnActivate)
END_EVENT_TABLE()

ClientUIMDIFrame::ClientUIMDIFrame()
	: SwitchBarParent(NULL, -1, "Dirt Secure Chat 3.0.0 Alpha 0",
		wxPoint(-1, -1), wxSize(500, 400),
		wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL)
{

	m_focused = true;

	SetIcon(wxIcon( dirt_xpm ));

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, "E&xit\tAlt-F4", "Quit the program");
	mnu->Append(mnuFile, "&File");

	mnu->Append(GetWindowMenu(), "&Window");

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, "&About\tF1");
	mnu->Append(mnuHelp, "&Help");

	SetMenuBar(mnu);

	#ifdef __WXMSW__
		
		RECT WorkArea;
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);
		RECT DefaultPos;
		DefaultPos.bottom = WorkArea.bottom;
		DefaultPos.left = WorkArea.left;
		DefaultPos.right = WorkArea.right;
		DefaultPos.top = DefaultPos.bottom - 326;//(WorkArea.bottom-WorkArea.top)/2.5;
		SetSize(
			DefaultPos.left,
			DefaultPos.top,
			DefaultPos.right - DefaultPos.left,
			DefaultPos.bottom - DefaultPos.top);

	#endif

	Show();

	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, "[Main]", ChannelCanvas);
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
	wxMessageBox(
		"Dirt Secure Chat 3.0.0 Alpha 0\n\nhttp://dirtchat.sourceforge.net/",
		"About", wxICON_INFORMATION);
}

ClientUIMDICanvas* ClientUIMDIFrame::GetContext(const wxString &context, bool create_if_not_exist)
{
	wxASSERT(m_switchbar->GetButtonCount() >= 1);
	if (context.Length() > 0)
	{
		for (int i = 1; i < m_switchbar->GetButtonCount(); ++i)
		{
			if (m_switchbar->GetButtonCaption(i) == context)
			{
				return (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(i);
			}
		}
	}
	if (context.Length() == 0 || !create_if_not_exist)
	{
		return (ClientUIMDICanvas*)m_switchbar->GetUserDataFromIndex(0);
	}
	else
	{
		ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, context, QueryCanvas);
		NewWindow(canvas, false);
		return canvas;
	}
}

void ClientUIMDIFrame::AddLine(const wxString &context, const wxString &line, const wxColour &line_colour, bool create_if_not_exist, bool suppress_alert, bool convert_urls)
{

	ClientUIMDICanvas *canvas = GetContext(context, create_if_not_exist);
	canvas->GetLog()->AddTextLine(Timestamp() + line, line_colour, tmmParse, convert_urls);
	
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
	if (cmd == "CLEAR")
	{
		GetContext(context)->GetLog()->Clear();
		return true;
	}
	else if (cmd == "EXIT")
	{
		Close();
		return true;
	}
	else if (cmd == "TEST")
	{
		GetContext(context)->LogControlTest();
		return true;
	}
	else if (cmd == "TEST2")
	{
		ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, "Test Send Transfer", TransferSendCanvas);
		NewWindow(canvas, true);
		return true;
	}
	else if (cmd == "QUERY")
	{
		HeadTail ht = SplitHeadTail(params);
		if (ht.head.Length() > 0)
		{
			FocusCanvas(GetContext(ht.head));
		}
		cmd = "MSG";
		return false;
	}
	else if (cmd == "HELP")
	{
		OnClientInformation(context, "Supported commands: CLEAR EXIT TEST QUERY");
		return false;
	}
	else
	{
		return false;
	}
}

void ClientUIMDIFrame::OnClientDebug(const wxString &context, const wxString &text)
{
	AddLine(context, "Debug: " + text, wxColour(128,128,128));
}

void ClientUIMDIFrame::OnClientWarning(const wxString &context, const wxString &text)
{
	AddLine(context, "* " + text, *wxRED);
}

void ClientUIMDIFrame::OnClientInformation(const wxString &context, const wxString &text)
{
	AddLine(context, "* " + text, wxColour(0,0,128));
}

void ClientUIMDIFrame::OnClientMessageOut(const wxString &nick, const wxString &text)
{
	AddLine(nick, "<" + m_client->GetNickname() + "> " + text, *wxBLACK, true, true);
}

void ClientUIMDIFrame::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private)
{
	wxString context = is_private ? nick : (const wxString)wxEmptyString;
	AddLine(context, "<" + nick + "> " + text);
}

void ClientUIMDIFrame::OnClientUserList(const wxArrayString &nicklist)
{
	m_lstNickList->Clear();
	for (size_t i = 0; i < nicklist.GetCount(); ++i)
	{
		m_lstNickList->Add(nicklist[i]);
	}
}

void ClientUIMDIFrame::OnClientUserJoin(const wxString &nick, const wxString &details)
{

	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << (char)OriginalModifier << ")";
	}
	msg << " has joined the chat";
	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, false, false);

	m_lstNickList->Add(nick);

}

void ClientUIMDIFrame::OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message)
{

	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << (char)OriginalModifier << ")";
	}
	msg << " has left the chat";
	if (message.Length() > 0)
	{
		msg << " (" << message << (char)OriginalModifier << ")";
	}
	AddLine(wxEmptyString, msg, wxColour(0, 128, 0), true, false, false);

	m_lstNickList->Remove(nick);

}
