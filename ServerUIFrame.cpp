#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrame.cpp,v 1.8 2003-02-14 06:04:59 jason Exp $)

#include "ServerUIFrame.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "InputControl.h"
#include "util.h"

#include "res/dirt.xpm"

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_FILE_EXIT,
	ID_HELP_ABOUT,
};

BEGIN_EVENT_TABLE(ServerUIFrame, wxFrame)
	EVT_TEXT_ENTER(ID_INPUT, ServerUIFrame::OnInput)
	EVT_MENU(ID_HELP_ABOUT, ServerUIFrame::OnHelpAbout)
	EVT_MENU(ID_FILE_EXIT, ServerUIFrame::OnFileExit)
END_EVENT_TABLE()

ServerUIFrame::ServerUIFrame()
	: wxFrame(
		NULL, -1, "Dirt Secure Chat Server " + GetProductVersion(),
		wxDefaultPosition, wxDefaultSize)
{

	SetIcon(wxIcon( dirt_xpm ));

	wxPanel *pnlLog = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN);

	m_txtInput = new InputControl(pnlLog, ID_INPUT);
	m_txtLog = new wxTextCtrl(pnlLog, ID_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);
	FixBorder(m_txtLog);
	m_txtLog->SetFont(m_txtInput->GetFont());

	wxBoxSizer *szrLog = new wxBoxSizer( wxVERTICAL );
	{
		szrLog->Add(m_txtLog, 1, wxEXPAND);
		szrLog->Add(m_txtInput, 0, wxEXPAND);
	}
	pnlLog->SetAutoLayout( TRUE );
	pnlLog->SetSizer( szrLog );

	szrLog->SetSizeHints( this );


	m_server = new ServerDefault(this);

	wxMenuBar *mnu = new wxMenuBar;

	wxMenu *mnuFile = new wxMenu;
	mnuFile->Append(ID_FILE_EXIT, "E&xit\tAlt-F4", "Quit the program");
	mnu->Append(mnuFile, "&File");

	wxMenu *mnuHelp = new wxMenu;
	mnuHelp->Append(ID_HELP_ABOUT, "&About\tF1");
	mnu->Append(mnuHelp, "&Help");

	SetMenuBar(mnu);

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
}

bool ServerUIFrame::OnServerPreprocess(wxString &cmd, wxString &params)
{
	if (cmd == "EXIT")
	{
		Close();
		return true;
	}
	else if (cmd == "HELP")
	{
		OnServerInformation("Supported commands: EXIT");
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
	int width = 500;
	int height = 350;
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
		<< "Dirt Secure Chat 3.0.0 Alpha 0\n"
		<< "\n"
		<< "Last revision date: " << GetRCSDate() << " UTC\n"
		<< "Last revision author: " << GetRCSAuthor() << "\n"
		<< "\n"
		<< "http://dirtchat.sourceforge.net/",
		"About Dirt Secure Chat", wxICON_INFORMATION);
}
