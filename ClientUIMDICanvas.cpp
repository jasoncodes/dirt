#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDICanvas.cpp,v 1.41 2003-03-18 06:36:38 jason Exp $)

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

enum
{
	ID_LOG = 1,
	ID_INPUT,
	ID_PASSWORD,
	ID_TRANSFER,
	ID_SASH,
	ID_NICKLIST,
	ID_NICKLIST_NICK,
	ID_NICKLIST_MESSAGE,
	ID_NICKLIST_QUERY,
	ID_NICKLIST_WHOIS
};

BEGIN_EVENT_TABLE(ClientUIMDICanvas, SwitchBarCanvas)
	EVT_SIZE(ClientUIMDICanvas::OnSize)
	EVT_SET_FOCUS(ClientUIMDICanvas::OnFocus)
	EVT_TEXT_ENTER(ID_INPUT, ClientUIMDICanvas::OnInputEnter)
	EVT_TEXT_ENTER(ID_PASSWORD, ClientUIMDICanvas::OnPasswordEnter)
	EVT_BUTTON(ID_LOG, ClientUIMDICanvas::OnLinkClicked)
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_SET_FOCUS, ID_LOG, ID_LOG, (wxObjectEventFunction)(wxFocusEventFunction)&ClientUIMDICanvas::OnFocus, NULL),
	EVT_SASH_DRAGGED(ID_SASH, ClientUIMDICanvas::OnSashDragged)
	EVT_LISTBOX_DCLICK(ID_NICKLIST, ClientUIMDICanvas::OnNickListDblClick)
	EVT_MENU(ID_NICKLIST, ClientUIMDICanvas::OnNickListMenu)
	EVT_MENU(ID_NICKLIST_MESSAGE, ClientUIMDICanvas::OnNickListMenuItem)
	EVT_MENU(ID_NICKLIST_QUERY, ClientUIMDICanvas::OnNickListMenuItem)
	EVT_MENU(ID_NICKLIST_WHOIS, ClientUIMDICanvas::OnNickListMenuItem)
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
			wxFAIL_MSG(wxT("Invalid CanvasType"));

	}

	SetTitle(title);
	SetIcon(icon);

	if (type == ChannelCanvas || type == QueryCanvas)
	{
		m_txtInput = new InputControl(this, ID_INPUT);
		m_txtInput->SetTabCompletionList(((ClientUIMDIFrame*)parent)->GetNicklist());
		m_txtLog = new LogControl(this, ID_LOG);
		m_txtPassword = NULL;
	}
	else
	{
		m_txtInput = NULL;
		m_txtLog = NULL;
		m_txtPassword = NULL;
	}

	if (type == ChannelCanvas)
	{
		m_sash = new wxSashWindow(
			this, ID_SASH,
			wxDefaultPosition, wxSize(112, 112),
			wxSW_3DSASH | wxCLIP_CHILDREN);
		m_sash->SetSashVisible(wxSASH_LEFT, true );
		m_lstNickList = new NickListControl(m_sash, ID_NICKLIST);
		m_lstNickList->SetFont(m_txtInput->GetFont());
	}
	else
	{
		m_sash = NULL;
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
	if (m_txtPassword)
	{
		m_txtPassword->SetFocus();
	}
	else if (m_txtInput)
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

void ClientUIMDICanvas::OnDetach()
{
	if (m_txtLog)
	{
		m_txtLog->ShowFindDialog(false);
	}
}

void ClientUIMDICanvas::OnActivate()
{
	if (((ClientUIMDIFrame*)m_parent)->IsFocused())
	{
		((ClientUIMDIFrame*)m_parent)->ResetRedLines();
	}
	DoGotFocus();
}

void ClientUIMDICanvas::OnClose()
{
	if (GetTransferPanel())
	{
		GetTransferPanel()->OnClose();
	}
}

void ClientUIMDICanvas::OnSashDragged(wxSashEvent &event)
{
	if (event.GetDragStatus() == wxSASH_STATUS_OK)
	{
		m_sash->SetSize(event.GetDragRect());
		ResizeChildren();
	}
}

bool ClientUIMDICanvas::GetPasswordMode() const
{
	return (m_txtPassword != NULL);
}

void ClientUIMDICanvas::SetPasswordMode(bool value)
{
	if (value && !m_txtPassword)
	{
		m_txtInput->ClosePopup();
		m_txtInput->SetValue(wxEmptyString);
		m_txtInput->Show(false);
		m_txtPassword = new wxTextCtrl(this, ID_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_PASSWORD);
		FixBorder(m_txtPassword);
		ResizeChildren();
		DoGotFocus();
	}
	else if (!value && m_txtPassword)
	{
		m_txtInput->SetValue(wxEmptyString);
		m_txtInput->Show(true);
		m_txtPassword->Destroy();
		m_txtPassword = NULL;
		ResizeChildren();
		DoGotFocus();
	}
}

void ClientUIMDICanvas::OnPasswordEnter(wxCommandEvent &event)
{
	wxString context = wxEmptyString;
	if (m_parent->GetSwitchBar()->GetIndexFromUserData(this) > 0)
	{
		context = GetTitle();
	}
	wxString text = m_txtPassword->GetValue();
	m_txtPassword->SetValue(wxEmptyString);
	SetPasswordMode(false);
	GetClient()->Authenticate(text);
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
			m_txtInput->GetBestSize().y;

		int log_height =
			size.y -
			input_height;

		(m_txtPassword?m_txtPassword:m_txtInput)->SetSize(0, log_height, size.x, input_height);

		if (m_sash)
		{

			static const int min_width_nick = 96;
			static const int min_width_log = 64;
			
			m_sash->SetMinimumSizeX(min_width_nick);
			m_sash->SetMaximumSizeX(size.x - min_width_log);

			int nick_width = m_sash->GetSize().x;
			int log_width = size.x - nick_width;
			
			if (log_width < min_width_log)
			{
				log_width = min_width_log;
				nick_width = size.x - log_width;
			}

			if (nick_width < min_width_nick)
			{
				nick_width = min_width_nick;
				log_width = size.x - nick_width;
			}

			if (log_width < min_width_log)
			{
				log_width = min_width_log;
			}

			int nick_left = size.x - nick_width;

			m_txtLog->SetSize(nick_left - log_width, 0, log_width, log_height);
			m_sash->SetSize(nick_left, 0, nick_width, log_height);

		}
		else
		{

			m_txtLog->SetSize(0, 0, size.x, log_height);

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
	m_txtLog->ResetRedLine();
	if (LeftEq(event.GetString().Lower(), wxT("/oper ")))
	{
		m_txtInput->RemoveLastHistoryEntry();
	}
	GetClient()->ProcessConsoleInput(context, event.GetString());
}

Client *ClientUIMDICanvas::GetClient()
{
	return ((ClientUIMDIFrame*)m_parent)->GetClient();
}

void ClientUIMDICanvas::OnLinkClicked(wxCommandEvent& event)
{
	OpenBrowser(this, event.GetString());
}

void ClientUIMDICanvas::OnNickListDblClick(wxCommandEvent &event)
{
	event.SetId(ID_NICKLIST_QUERY);
	OnNickListMenuItem(event);
}

void ClientUIMDICanvas::OnNickListMenu(wxCommandEvent &event)
{

	wxString nick = m_lstNickList->GetSelectedNick();

	wxMenu menu;

	menu.Append(ID_NICKLIST_NICK, nick);
	menu.Enable(ID_NICKLIST_NICK, false);
	menu.AppendSeparator();
	menu.Append(ID_NICKLIST_MESSAGE, wxT("Send &Message"));
	menu.Append(ID_NICKLIST_QUERY, wxT("&Query"));
	menu.Append(ID_NICKLIST_WHOIS, wxT("&Who Is"));

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

				wxString msg = ::wxGetTextFromUser(wxT("Message to send to ") + nick + wxT(":"), wxT("Dirt Secure Chat"));
				
				if (msg.Length() > 0)
				{
					GetClient()->SendMessage(wxEmptyString, nick, msg, false);
				}

			}

		}
		break;

		case ID_NICKLIST_QUERY:
		{
			wxCommandEvent evt(wxEVT_COMMAND_TEXT_ENTER, ID_INPUT);
			evt.SetString(wxT("/query ") + nick);
			AddPendingEvent(evt);
		}
		break;

		case ID_NICKLIST_WHOIS:
		{
			GetClient()->WhoIs(wxEmptyString, nick);
		}
		break;

		default:
			wxFAIL_MSG(wxT("Unknown event ID"));

	}

}

void ClientUIMDICanvas::LogControlTest()
{
	for (int i = 0; i < 10; i++)
	{
		wxString tmp;
		tmp << wxT("Line ") << i << wxT(" ");
		tmp = tmp + tmp + tmp + tmp + tmp + tmp + tmp;
		tmp.Trim();
		m_txtLog->AddTextLine(tmp);
	}
	m_txtLog->AddTextLine(wxT("Testing 1 2 3. http://www.test.com/ is a test."));
	wxChar ctrl_b = wxT('\x002');
	wxChar ctrl_c = wxT('\x003');
	wxChar ctrl_r = wxT('\x016');
	wxChar ctrl_u = wxT('\x01f');
	m_txtLog->AddTextLine(wxString() << wxT("this ") << ctrl_b << wxT("is") << ctrl_b << wxT(" ") << ctrl_u << wxT("a ") << ctrl_c << wxT("9,1test") << ctrl_c << wxT(" line"));
	m_txtLog->AddHtmlLine(wxT("alpha <font color=green>beta</font> <span style='background: yellow;'>delta</span> gamma -- green white black yellow"));
	m_txtLog->AddHtmlLine(wxT("alpha <span style='background: yellow'>beta</span> <font color=green>delta</font></span> gamma -- black yellow green white"));
	m_txtLog->AddHtmlLine(wxT("alpha <font color=green>beta <span style='background: yellow'>delta</span></font></span> gamma -- green white green yellow"));
	m_txtLog->AddHtmlLine(wxT("<span style='background: #e0e0e0'><font color='#000080'>these words should be on a single line</font></span>"));
	m_txtLog->AddTextLine(wxString() << ctrl_c << wxT("9,1green black ") << ctrl_c << wxT("4red black"));
	m_txtLog->AddTextLine(wxString() << ctrl_c << wxT("9,1green black") << ctrl_c << wxT(" black white"));
	m_txtLog->AddTextLine(wxString() << ctrl_c << wxT("3,green"));
	m_txtLog->AddHtmlLine(wxT("no <span style='background: yellow'></span> colour<span style='background: #e0e0e0'></span> on <b></b>this <font color=red></font>line"));
	m_txtLog->AddHtmlLine(wxT("a single 'x' with yellow bg --&gt; <span style='background: yellow'>x</span> &lt;--"));
	m_txtLog->AddTextLine(wxString() << ctrl_c << wxT("2,15blue-grey ") << ctrl_r << wxT("reverse") << ctrl_r << wxT(" blue-grey ") << ctrl_c << wxT("4red-grey ") << ctrl_r << wxT("rev") << ctrl_c << ctrl_c << wxT("2erse") << ctrl_r << wxT(" blue-white ") << ctrl_c << wxT("black-white ") << ctrl_r << wxT("reverse"));
	m_txtLog->AddTextLine(wxString() << wxT("Should have two spaces between letters: ") << ctrl_c << wxT("1t ") << ctrl_c << wxT("1 ") << ctrl_c << wxT("1e ") << ctrl_c << wxT(" ") << ctrl_c << wxT("1s  t !"));
	m_txtLog->AddTextLine(wxT("Space Test: 1 2  3   4    . exclamation line up -> !"));
}
