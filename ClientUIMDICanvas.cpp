#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIMDICanvas.h"
#include "SwitchBarChild.h"
#include "SwitchBarParent.h"
#include "ClientUIMDIFrame.h"
#include "util.h"

#include "res/channel.xpm"
#include "res/query.xpm"
#include "res/send.xpm"
#include "res/receive.xpm"

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_NICKLIST,
	ID_TRANSFER
};

BEGIN_EVENT_TABLE(ClientUIMDICanvas, SwitchBarCanvas)
	EVT_SIZE(ClientUIMDICanvas::OnSize)
	EVT_SET_FOCUS(ClientUIMDICanvas::OnFocus)
	EVT_TEXT_ENTER(ID_INPUT, ClientUIMDICanvas::OnInputEnter)
	EVT_BUTTON(ID_LOG, ClientUIMDICanvas::OnLinkClicked)
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_SET_FOCUS, ID_LOG, ID_LOG, (wxObjectEventFunction)(wxFocusEventFunction)&ClientUIMDICanvas::OnFocus, NULL),
END_EVENT_TABLE()

ClientUIMDICanvas::ClientUIMDICanvas(SwitchBarParent *parent, const wxString &title, CanvasType type)
	: SwitchBarCanvas(parent, wxDefaultPosition, wxDefaultSize)
{

	wxIcon icon = wxNullIcon;

	m_type = type;

	switch (type)
	{

		case ChannelCanvas:
			icon = wxIcon(channel_xpm);
			break;

		case QueryCanvas:
			icon = wxIcon(query_xpm);
			break;

		case TransferSendCanvas:
			icon = wxIcon(send_xpm);
			break;

		case TransferReceiveCanvas:
			icon = wxIcon(receive_xpm);
			break;

		default:
			wxFAIL_MSG("Invalid CanvasType");

	}

	SetTitle(title);
	SetIcon(icon);

	if (type == ChannelCanvas || type == QueryCanvas)
	{
		m_txtLog = new LogControl(this, ID_LOG);
		m_txtInput = new InputControl(this, ID_INPUT);
	}
	else
	{
		m_txtLog = NULL;
		m_txtInput = NULL;
	}

	if (type == ChannelCanvas)
	{
		m_lstNickList = new wxListBox(this, ID_NICKLIST);
		FixBorder(m_lstNickList);
	}
	else
	{
		m_lstNickList = NULL;
	}

	if (type == TransferSendCanvas || type == TransferReceiveCanvas)
	{
		m_pnlTransfer = new ClientUIMDITransferPanel(this, ID_TRANSFER);
	}
	else
	{
		m_pnlTransfer = NULL;
	}

}

ClientUIMDICanvas::~ClientUIMDICanvas()
{
}

void ClientUIMDICanvas::DoGotFocus()
{
	if (m_txtInput)
	{
		m_txtInput->SetFocus();
	}
	SwitchBar *switchbar = m_parent->GetSwitchBar();
	int button_index = switchbar->GetIndexFromUserData(this);
	if (button_index > -1)
	{
		switchbar->SetButtonHighlight(button_index, false);
	}
}

void ClientUIMDICanvas::OnFocus(wxFocusEvent &event)
{
	DoGotFocus();
}

void ClientUIMDICanvas::OnAttach()
{
	DoGotFocus();
}

void ClientUIMDICanvas::OnActivate()
{
	DoGotFocus();
}

void ClientUIMDICanvas::ResizeChildren()
{

	
	wxSize size = GetClientSize();

	if (m_pnlTransfer != NULL)
	{

		m_pnlTransfer->SetSize(0, 0, size.x, size.y);

	}
	else if (m_txtLog != NULL && m_txtInput != NULL)
	{

		int input_height = 
			m_txtInput->GetBestSize().GetHeight();

		int nicklist_width = (m_lstNickList != NULL) ? 128 : 0;

		int log_width =
			size.GetWidth() - nicklist_width;

		int log_height =
			size.GetHeight() -
			input_height;

		m_txtLog->SetSize(0, 0, log_width, log_height);
		m_txtInput->SetSize(0, log_height, size.GetWidth(), input_height);
		if (m_lstNickList)
		{
			m_lstNickList->SetSize(log_width, 0, nicklist_width, log_height);
		}

	}

}

void ClientUIMDICanvas::OnSize(wxSizeEvent& event)
{
	ResizeChildren();
}

void ClientUIMDICanvas::OnInputEnter(wxCommandEvent& event)
{
	wxString context = wxEmptyString;
	if (m_parent->GetSwitchBar()->GetIndexFromUserData(this) > 0)
	{
		context = GetTitle();
	}
	((ClientUIMDIFrame*)m_parent)->GetClient()->ProcessInput(context, event.GetString());
}

void ClientUIMDICanvas::OnLinkClicked(wxCommandEvent& event)
{
	wxMessageBox("User clicked on link: " + event.GetString(), "Dirt Secure Chat", wxICON_INFORMATION);
}

void ClientUIMDICanvas::LogControlTest()
{
	for (int i = 0; i < 10; i++)
	{
		wxString tmp;
		tmp << "Line " << i << " ";
		tmp = tmp + tmp + tmp + tmp + tmp + tmp + tmp;
		tmp.Trim();
		m_txtLog->AddTextLine(tmp);
	}
	m_txtLog->AddTextLine("Testing 1 2 3. http://www.test.com/ is a test.");
	char ctrl_b = '\x002';
	char ctrl_c = '\x003';
	char ctrl_r = '\x016';
	char ctrl_u = '\x01f';
	m_txtLog->AddTextLine(wxString() << "this " << ctrl_b << "is" << ctrl_b << " " << ctrl_u << "a " << ctrl_c << "9,1test" << ctrl_c << " line");
	m_txtLog->AddHtmlLine("alpha <font color=green>beta</font> <span style='background: yellow;'>delta</span> gamma -- green white black yellow");
	m_txtLog->AddHtmlLine("alpha <span style='background: yellow'>beta</span> <font color=green>delta</font></span> gamma -- black yellow green white");
	m_txtLog->AddHtmlLine("alpha <font color=green>beta <span style='background: yellow'>delta</span></font></span> gamma -- green white green yellow");
	m_txtLog->AddHtmlLine("<span style='background: #e0e0e0'><font color='#000080'>these words should be on a single line</font></span>");
	m_txtLog->AddTextLine(wxString() << ctrl_c << "9,1green black " << ctrl_c << "4red black");
	m_txtLog->AddTextLine(wxString() << ctrl_c << "9,1green black" << ctrl_c << " black white");
	m_txtLog->AddTextLine(wxString() << ctrl_c << "3,green");
	m_txtLog->AddHtmlLine("no <span style='background: yellow'></span> colour<span style='background: #e0e0e0'></span> on <b></b>this <font color=red></font>line");
	m_txtLog->AddHtmlLine("a single 'x' with yellow bg --&gt; <span style='background: yellow'>x</span> &lt;--");
	m_txtLog->AddTextLine(wxString() << ctrl_c << "2,15blue-grey " << ctrl_r << "reverse" << ctrl_r << " blue-grey " << ctrl_c << "4red-grey " << ctrl_r << "rev" << ctrl_c << ctrl_c << "2erse" << ctrl_r << " blue-white " << ctrl_c << "black-white " << ctrl_r << "reverse");
}
