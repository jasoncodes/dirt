#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrame.cpp,v 1.5 2003-02-14 04:50:12 jason Exp $)

#include "ServerUIFrame.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "InputControl.h"
#include "util.h"

#include "res/dirt.xpm"

enum
{
	ID_LOG = 1,
	ID_INPUT
};

BEGIN_EVENT_TABLE(ServerUIFrame, wxFrame)
	EVT_SIZE(ServerUIFrame::OnSize)
	EVT_TEXT_ENTER(ID_INPUT, ServerUIFrame::OnInput)
END_EVENT_TABLE()

ServerUIFrame::ServerUIFrame()
	: wxFrame(
		NULL, -1, "Dirt Secure Chat Server " + GetProductVersion(),
		wxDefaultPosition, wxDefaultSize)
{
        SetIcon(wxIcon( dirt_xpm ));
	m_txtInput = new InputControl(this, ID_INPUT);
	m_txtLog = new wxTextCtrl(this, ID_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);
	FixBorder(m_txtLog);
	m_txtLog->SetFont(m_txtInput->GetFont());
	m_server = new ServerDefault(this);
	ResetWindowPos();
	Show();
	m_txtInput->SetFocus();
}

ServerUIFrame::~ServerUIFrame()
{
	delete m_server;
}

void ServerUIFrame::OnSize(wxSizeEvent &event)
{

	wxSize size = GetClientSize();

	if (m_txtLog != NULL && m_txtInput != NULL)
	{

		int input_height = 
			m_txtInput->GetBestSize().y;

		int log_height =
			size.y -
			input_height;

		m_txtInput->SetSize(0, log_height, size.x, input_height);

		m_txtLog->SetSize(0, 0, size.x, log_height);

	}

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
