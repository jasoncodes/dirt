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

#include "res/dirt.xpm"
#include "res/channel.xpm"
#include "res/query.xpm"

enum
{
	ID_HELP_ABOUT = 1,
	ID_FILE_NEW_WINDOW,
	ID_FILE_NEW_WINDOW_NO_FOCUS,
	ID_FILE_EXIT,
	ID_FOCUSINPUTCONTROLTIMER
};

BEGIN_EVENT_TABLE(ClientUIMDIFrame, SwitchBarParent)
	EVT_MENU(ID_HELP_ABOUT, ClientUIMDIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_NEW_WINDOW, ClientUIMDIFrame::OnFileNewWindow)
	EVT_MENU(ID_FILE_NEW_WINDOW_NO_FOCUS, ClientUIMDIFrame::OnFileNewWindowNoFocus)
	EVT_MENU(ID_FILE_EXIT, ClientUIMDIFrame::OnFileExit)
	EVT_TIMER(ID_FOCUSINPUTCONTROLTIMER, ClientUIMDIFrame::OnFocusInputControlTimer)
END_EVENT_TABLE()

ClientUIMDIFrame::ClientUIMDIFrame()
	: SwitchBarParent(NULL, -1, "Dirt Secure Chat 3.0.0 Alpha 0",
		wxPoint(-1, -1), wxSize(500, 400),
		wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL)
{

	SetIcon(wxIcon( dirt_xpm ));

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_NEW_WINDOW, "&New window\tCtrl-N", "Create a new child window");
	mnuFile->Append(ID_FILE_NEW_WINDOW_NO_FOCUS, "New window (No &focus)\tCtrl-F", "Create a new child window (without focusing)");
	mnuFile->Append(ID_FILE_EXIT, "&Exit\tAlt-X", "Quit the program");
	mnu->Append(mnuFile, "&File");

	mnu->Append(GetWindowMenu(), "&Window");

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, "&About\tF1");
	mnu->Append(mnuHelp, "&Help");

	SetMenuBar(mnu);

	Show();

	ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxIcon(channel_xpm));
	NewWindow(canvas, true);
	canvas->SetTitle("[Main]");
	canvas->closable = false;

	tmrFocusInputControl = new wxTimer(this, ID_FOCUSINPUTCONTROLTIMER);
	tmrFocusInputControl->Start(100);

	m_client = new ClientDefault(this);

}

ClientUIMDIFrame::~ClientUIMDIFrame()
{
	delete tmrFocusInputControl;
	delete m_client;
}

void ClientUIMDIFrame::OnFocusInputControlTimer(wxTimerEvent& event)
{
	SwitchBarChild *child = (SwitchBarChild*)GetActiveChild();
	if (child)
	{
		child->GetCanvas()->OnActivate();
	}
}

void ClientUIMDIFrame::OnFileExit(wxCommandEvent& event)
{
	Close();
}

void ClientUIMDIFrame::OnHelpAbout(wxCommandEvent& event)
{
	wxMessageBox("Dirt Secure Chat 3.0.0 Alpha 0");
}

void ClientUIMDIFrame::OnFileNewWindow(wxCommandEvent& event)
{
	NewWindow(new ClientUIMDICanvas(this, wxIcon(query_xpm)), true);
}

void ClientUIMDIFrame::OnFileNewWindowNoFocus(wxCommandEvent& event)
{
	NewWindow(new ClientUIMDICanvas(this, wxIcon(query_xpm)), false);
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
		ClientUIMDICanvas *canvas = new ClientUIMDICanvas(this, wxIcon(query_xpm));
		canvas->SetTitle(context);
		NewWindow(canvas, false);
		return canvas;
	}
}

void ClientUIMDIFrame::AddLine(const wxString &context, const wxString &line, const wxColour &line_colour, bool create_if_not_exist, bool suppress_alert)
{
	ClientUIMDICanvas *canvas = GetContext(context, create_if_not_exist);
	canvas->GetLog()->AddTextLine(Timestamp() + line, line_colour);
	if (!suppress_alert && GetActiveChild() != canvas->GetParent())
	{
		int button_index = m_switchbar->GetIndexFromUserData(canvas);
		if (button_index > -1)
		{
			m_switchbar->SetButtonHighlight(button_index, true);
		}
		wxBell();
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
