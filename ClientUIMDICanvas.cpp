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

enum
{
	ID_LOG = 1,
	ID_INPUT,
};

BEGIN_EVENT_TABLE(ClientUIMDICanvas, SwitchBarCanvas)
	EVT_SIZE(ClientUIMDICanvas::OnSize)
	EVT_SET_FOCUS(ClientUIMDICanvas::OnFocus)
	EVT_TEXT_ENTER(ID_INPUT, ClientUIMDICanvas::OnInputEnter)
	EVT_BUTTON(ID_LOG, ClientUIMDICanvas::OnLinkClicked)
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_SET_FOCUS, ID_LOG, ID_LOG, (wxObjectEventFunction)(wxFocusEventFunction)&ClientUIMDICanvas::OnFocus, NULL),
END_EVENT_TABLE()

int ClientUIMDICanvas::s_num = 0;

ClientUIMDICanvas::ClientUIMDICanvas(SwitchBarParent *parent, wxIcon icon, const wxPoint& pos, const wxSize& size)
	: SwitchBarCanvas(parent, pos, size)
{

	m_txtLog = new LogControl(this, ID_LOG);
	m_txtInput = new InputControl(this, ID_INPUT);

	SetTitle(wxString::Format("Canvas %d", ++s_num));
	SetIcon(icon);

	closable = true;

}

ClientUIMDICanvas::~ClientUIMDICanvas()
{
}

void ClientUIMDICanvas::DoGotFocus()
{
	m_txtInput->SetFocus();
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

	if (m_txtLog != NULL && m_txtInput != NULL)
	{

		wxSize size = GetClientSize();

		int input_height = 
			m_txtInput->GetBestSize().GetHeight();

		int log_width =
			size.GetWidth();

		int log_height =
			size.GetHeight() -
			input_height;

		m_txtLog->SetSize(0, 0, log_width, log_height);
		m_txtInput->SetSize(0, log_height, log_width, input_height);

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
	wxMessageBox("User clicked on link: " + event.GetString(), GetTitle(), wxICON_INFORMATION);
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
