#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrame.cpp,v 1.13 2003-02-15 03:39:35 jason Exp $)

#include "ServerUIFrame.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "InputControl.h"
#include "util.h"

#include "Dirt.h"
DECLARE_APP(DirtApp)

#include "res/dirt.xpm"

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_FILE_EXIT,
	ID_HELP_ABOUT,
	ID_STARTSTOP,
	ID_CONFIGURATION,
	ID_CLIENT,
	ID_CLEAR
};

BEGIN_EVENT_TABLE(ServerUIFrame, wxFrame)
	EVT_TEXT_ENTER(ID_INPUT, ServerUIFrame::OnInput)
	EVT_MENU(ID_HELP_ABOUT, ServerUIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ServerUIFrame::OnFileExit)
	EVT_BUTTON(ID_STARTSTOP, ServerUIFrame::OnStartStop)
	EVT_BUTTON(ID_CONFIGURATION, ServerUIFrame::OnConfiguration)
	EVT_BUTTON(ID_CLIENT, ServerUIFrame::OnClient)
	EVT_BUTTON(ID_CLEAR, ServerUIFrame::OnClear)
END_EVENT_TABLE()

ServerUIFrame::ServerUIFrame()
	: wxFrame(
		NULL, -1, "Dirt Secure Chat Server " + GetProductVersion(),
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{

	SetIcon(wxIcon(dirt_xpm));

	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	m_txtInput = new InputControl(panel, ID_INPUT);
	m_txtLog = new wxTextCtrl(panel, ID_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);
	FixBorder(m_txtLog);
	m_txtLog->SetFont(m_txtInput->GetFont());

	wxStaticBox *boxConsole = new wxStaticBox(panel, -1, "Console");
	wxBoxSizer *szrConsole = new wxStaticBoxSizer(boxConsole, wxVERTICAL);
	{
		szrConsole->Add(m_txtLog, 1, wxEXPAND);
		szrConsole->Add(m_txtInput, 0, wxEXPAND);
	}

	m_cmdStartStop = new wxButton(panel, ID_STARTSTOP, "&Start");
	m_cmdConfiguration = new wxButton(panel, ID_CONFIGURATION, "&Configuration");
	m_cmdClient = new wxButton(panel, ID_CLIENT, "&Launch Client");
	m_cmdClear = new wxButton(panel, ID_CLEAR, "Clear Lo&g");

	wxBoxSizer *szrButtons = new wxBoxSizer(wxVERTICAL);
	{
		szrButtons->Add(m_cmdStartStop, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdConfiguration, 0, wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdClient, 0, wxBOTTOM | wxEXPAND, 8);
		szrButtons->Add(m_cmdClear, 0, wxBOTTOM | wxEXPAND, 8);
	}

	wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
	szrLeft->Add(szrConsole, 1, wxEXPAND);

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND, 8);
	szrAll->Add(szrButtons, 0, wxALL | wxEXPAND, 8);

	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints( this );

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, "E&xit\tAlt-F4", "Quit the program");
	mnu->Append(mnuFile, "&File");

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, "&About\tF1");
	mnu->Append(mnuHelp, "&Help");

	SetMenuBar(mnu);

	m_server = new ServerDefault(this);
	OnServerStateChange();
	m_server->Start();

	ResetWindowPos();
	Show();

	m_txtInput->SetFocus();

}

ServerUIFrame::~ServerUIFrame()
{
	delete m_server;
}

void ServerUIFrame::OnInput(wxCommandEvent &event)
{
	m_server->ProcessInput(event.GetString());
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
		m_txtLog->AppendText("\n" + tmp);
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
	if (cmd == "CLEAR")
	{
		m_txtLog->Clear();
		return true;
	}
	else if (cmd == "EXIT")
	{
		Close();
		return true;
	}
	else if (cmd == "HELP")
	{
		OnServerInformation("Supported commands: CLEAR EXIT");
		return false;
	}
	else
	{
		return false;
	}
}

bool ServerUIFrame::ResetWindowPos()
{
	wxRect rtWorkArea = ::wxGetClientDisplayRect();
	int width = 576;
	int height = 352;
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
	wxMessageBox(wxString()
		<< wxT("Dirt Secure Chat 3.0.0 Alpha 0\n")
		<< wxT("\n")
		<< wxT("Last revision date: ") << GetRCSDate() << wxT(" UTC\n")
		<< wxT("Last revision author: ") << GetRCSAuthor() << wxT("\n")
		<< wxT("\n")
		<< wxT("http://dirtchat.sourceforge.net/"),
		wxT("About Dirt Secure Chat"), wxICON_INFORMATION);
}

void ServerUIFrame::OnStartStop(wxCommandEvent& event)
{
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
	// not implemented
}

void ServerUIFrame::OnClient(wxCommandEvent& event)
{
	wxASSERT(m_server->IsRunning());
	wxASSERT(wxGetApp().argc > 0);
	wxString param;
	param << wxT("--host=dirt://localhost:") << m_server->GetListenPort() << wxT("/");
	const wxChar *argv[3];
	argv[0] = wxGetApp().argv[0];
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
	m_cmdStartStop->SetLabel(m_server->IsRunning() ? wxT("&Stop") : wxT("&Start"));
	m_cmdConfiguration->Enable(false); // not implemented
	m_cmdClient->Enable(m_server->IsRunning());
}
