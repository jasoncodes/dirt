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
#include "NickListControl.h"

#include "res/channel.xpm"
#include "res/query.xpm"
#include "res/send.xpm"
#include "res/receive.xpm"

class MySplitterWindow : public wxSplitterWindow
{

public:

	MySplitterWindow(wxWindow *parent, int id)
		: wxSplitterWindow(
			parent, id,
			wxDefaultPosition, wxDefaultSize,
			wxSP_NOBORDER | wxSP_FULLSASH /*| wxSP_LIVE_UPDATE*/ | wxCLIP_CHILDREN)
	{
		SetMinimumPaneSize(96);
	}

	virtual ~MySplitterWindow()
	{
	}

	virtual void DrawSash(wxDC& dc)
	{
		int w, h;
		GetClientSize(&w, &h);
        dc.SetBrush(wxBrush(GetBackgroundColour(), wxSOLID));
        dc.SetPen(wxPen(GetBackgroundColour(), 1, wxSOLID));
        dc.DrawRectangle(0, 0, w-1, h-1);
		dc.SetPen(wxNullPen);
		dc.SetBrush(wxNullBrush);
	}

protected:

	void OnSize(wxSizeEvent &event)
	{

		if (GetSize().x > 0 && GetSize().y > 0)
		{
		
			int old_pos = GetSashPosition();
			int old_size = GetWindow2()->GetSize().x;

			wxSplitterWindow::OnSize(event);

			if (old_pos > 0)
			{
				int new_size = GetWindow2()->GetSize().x;
				int size_diff = new_size - old_size;
				int new_pos = old_pos + size_diff;
				SetSashPosition(new_pos);
			}

		}

	}

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(MySplitterWindow, wxSplitterWindow)
	EVT_SIZE(MySplitterWindow::OnSize)
END_EVENT_TABLE()

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_TRANSFER,
	ID_NICKLIST,
	ID_NICKLIST_NICK,
	ID_NICKLIST_MESSAGE,
	ID_NICKLIST_QUERY
};

BEGIN_EVENT_TABLE(ClientUIMDICanvas, SwitchBarCanvas)
	EVT_SIZE(ClientUIMDICanvas::OnSize)
	EVT_SET_FOCUS(ClientUIMDICanvas::OnFocus)
	EVT_TEXT_ENTER(ID_INPUT, ClientUIMDICanvas::OnInputEnter)
	EVT_BUTTON(ID_LOG, ClientUIMDICanvas::OnLinkClicked)
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_SET_FOCUS, ID_LOG, ID_LOG, (wxObjectEventFunction)(wxFocusEventFunction)&ClientUIMDICanvas::OnFocus, NULL),
	EVT_LISTBOX_DCLICK(ID_NICKLIST, ClientUIMDICanvas::OnNickListDblClick)
	EVT_MENU(ID_NICKLIST, ClientUIMDICanvas::OnNickListMenu)
	EVT_MENU(ID_NICKLIST_MESSAGE, ClientUIMDICanvas::OnNickListMenuItem)
	EVT_MENU(ID_NICKLIST_QUERY, ClientUIMDICanvas::OnNickListMenuItem)
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
		m_splitter = new MySplitterWindow(this, wxID_ANY);
		m_lstNickList = new NickListControl(m_splitter, ID_NICKLIST);
		m_lstNickList->SetFont(m_txtInput->GetFont());
		m_txtLog->Reparent(m_splitter);
		m_splitter->SplitVertically(m_txtLog, m_lstNickList, -128);
	}
	else
	{
		m_splitter = NULL;
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

		m_txtInput->SetSize(0, log_height, size.GetWidth(), input_height);
		if (m_splitter)
		{
			m_splitter->SetSize(0, 0, size.GetWidth(), log_height);
		}
		else
		{
			m_txtLog->SetSize(0, 0, log_width, log_height);
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
	GetClient()->ProcessInput(context, event.GetString());
}

Client *ClientUIMDICanvas::GetClient()
{
	return ((ClientUIMDIFrame*)m_parent)->GetClient();
}

void ClientUIMDICanvas::OnLinkClicked(wxCommandEvent& event)
{

	#ifdef __WXMSW__

		::wxBeginBusyCursor();
		HINSTANCE hInstance = ::ShellExecute((HWND)GetHandle(), "open", event.GetString(), NULL, NULL, SW_NORMAL);
		::wxEndBusyCursor();
		bool success = ((int)hInstance > 32);
		if (!success)
		{
			wxMessageBox("Unable to navigate to " + event.GetString(), "Dirt Secure Chat", wxICON_ERROR);
		}

	#else

		const char *browsers[2] = { "mozilla", "netscape" };
		const size_t num_browsers = ((sizeof browsers) / (sizeof browsers[0]));

		bool success = false;

		for (int i = 0; i < num_browsers && !success; ++i)
		{
			wxString cmdline;
			cmdline << browsers[i] << ' ' << event.GetString();
			::wxBeginBusyCursor();
			long pid = ::wxExecute(cmdline, wxEXEC_ASYNC);
			::wxEndBusyCursor();
			success = (pid != 0);
		}

		if (!success)
		{
			wxMessageBox("Unable to navigate to " + event.GetString() + "\n\nPlease check that Mozilla in your path", "Dirt Secure Chat", wxICON_ERROR);
		}

	#endif

}

void ClientUIMDICanvas::OnNickListDblClick(wxCommandEvent &event)
{
	event.SetId(ID_NICKLIST_MESSAGE);
	OnNickListMenuItem(event);
}

void ClientUIMDICanvas::OnNickListMenu(wxCommandEvent &event)
{

	wxString nick = m_lstNickList->GetSelectedNick();

	wxMenu menu;

	menu.Append(ID_NICKLIST_NICK, nick);
	menu.Enable(ID_NICKLIST_NICK, false);
	menu.AppendSeparator();
	menu.Append(ID_NICKLIST_MESSAGE, "Send &Message");
	menu.Append(ID_NICKLIST_QUERY, "&Query");

	wxPoint pos = ScreenToClient(wxGetMousePosition());
	PopupMenu(&menu, pos);

}

void ClientUIMDICanvas::OnNickListMenuItem(wxCommandEvent &event)
{

	wxString nick = m_lstNickList->GetSelectedNick();

	switch (event.GetId())
	{

		case ID_NICKLIST_MESSAGE:
		{

			if (nick.Length() > 0)
			{

				wxString msg = ::wxGetTextFromUser("Message to send to " + nick + ":", "Dirt Secure Chat");
				
				if (msg.Length() > 0)
				{
					GetClient()->SendMessage(nick, msg);
				}

			}

		}
		break;

		case ID_NICKLIST_QUERY:
		{
			GetClient()->ProcessInput(wxEmptyString, "/query " + nick);
		}
		break;

		default:
			wxFAIL_MSG("Unknown event ID");

	}

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
