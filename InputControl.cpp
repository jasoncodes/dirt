#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: InputControl.cpp,v 1.10 2003-03-05 12:05:55 jason Exp $)

#include "InputControl.h"
#include "Modifiers.h"
#include <wx/minifram.h>
#include "util.h"



class InputControlColourPanel : public wxPanel
{

public:

	InputControlColourPanel(wxWindow *parent, wxWindowID id, const wxString &text, const wxPoint& pos, const wxSize size)
		: wxPanel(parent, id, pos, size, wxBORDER_SUNKEN), m_text(text)
	{
	}

protected:
	void OnPaint(wxPaintEvent& event)
	{
		
		wxPaintDC dc(this);
		
		dc.SetFont(GetParent()->GetFont());
		dc.SetTextForeground(GetForegroundColour());
		
		int w, h;
		dc.GetTextExtent(m_text, &w, &h);

		dc.DrawText(
			m_text,
			(GetClientSize().x - w) / 2,
			(GetClientSize().y - h) / 2);

	}

protected:
	wxString m_text;

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(InputControlColourPanel, wxPanel)
	EVT_PAINT(InputControlColourPanel::OnPaint)
END_EVENT_TABLE()



class InputControlColourPopup : public wxMiniFrame
{

public:
	InputControlColourPopup(InputControl* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxFrameNameStr)
		: wxMiniFrame(parent, id, title, pos, size, wxCAPTION | wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT, name)
	{
		
		wxSize static_size(20,20);
		wxPoint static_pos(0,0);

		int client_width = ((colour_count + 1) / 2) * static_size.x;

		for (int i = 0; i < colour_count; ++i)
		{
			
			InputControlColourPanel *lblTest = new InputControlColourPanel(this, 100, wxString() << i, static_pos, static_size);
			
			lblTest->SetBackgroundColour(colours[i]);
			lblTest->SetForegroundColour(colour_backgrounds[i]);
			
			static_pos.x += static_size.x;
			if (static_pos.x + static_size.x > client_width)
			{
				static_pos.x = 0;
				static_pos.y += static_size.y;
			}

		}

		SetClientSize(client_width, static_size.y * 2);
		PositionSelf();

		tmr = new wxTimer(this);
		tmr->Start(100);

	}

	~InputControlColourPopup()
	{
		delete tmr;
	}

	InputControl *GetControl()
	{
		return static_cast<InputControl*>(GetParent());
	}

	wxFrame *GetControlFrame()
	{
		wxWindow *wnd = GetParent();
		while (wnd->GetParent() != NULL)
		{
			wnd = wnd->GetParent();
		}
		return static_cast<wxFrame*>(wnd);
	}

	void CloseMe()
	{
		GetControl()->ClosePopup();
	}

	void PositionSelf()
	{
		if (GetControlFrame()->IsIconized())
		{
			CloseMe();
		}
		else
		{
			InputControl *txtInput = GetControl();
			long pos_x, pos_y;
			txtInput->PositionToXY(txtInput->GetInsertionPoint(), &pos_x, &pos_y);
			wxString text = txtInput->GetLineText(pos_y).Left(pos_x);
			int text_width, text_height;
			txtInput->GetTextExtent(text, &text_width, &text_height);
			int max_width = txtInput->GetSize().x - GetSize().x;
			if (text_width > max_width)
			{
				text_width = max_width;
			}
			Move(txtInput->ClientToScreen(wxPoint(text_width, -GetSize().y)));
		}
	}

	void OnCloseWindow(wxCloseEvent& event)
	{
		if (!event.CanVeto()) // Test if we can veto this deletion
		{
			this->Destroy(); // If not, destroy the window anyway.
		}
		else
		{
			event.Veto(); // Notify the calling code that we didn't delete the frame.
			CloseMe();
		}
	}

	void OnTimer(wxTimerEvent& event)
	{

		PositionSelf();

		wxWindow *wnd = ::wxGetActiveWindow();
		if (wnd != NULL && wnd != GetControlFrame())
		{
			CloseMe();
		}

		wnd = this;
		while (wnd != NULL)
		{
			if (!wnd->IsShown())
			{
				CloseMe();
				break;
			}
			wnd = wnd->GetParent();
		}

	}

protected:
	wxTimer *tmr;

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(InputControlColourPopup, wxMiniFrame)
	EVT_CLOSE(InputControlColourPopup::OnCloseWindow)
	EVT_TIMER(wxID_ANY, InputControlColourPopup::OnTimer)
END_EVENT_TABLE()



#include <wx/html/htmlwin.h>
#include <wx/html/winpars.h>

BEGIN_EVENT_TABLE(InputControl, wxTextCtrl)
	EVT_TEXT(wxID_ANY, InputControl::OnChange)
	EVT_TEXT_ENTER(wxID_ANY, InputControl::OnEnterPress)
	EVT_KEY_DOWN(InputControl::OnKeyDown)
	EVT_KEY_UP(InputControl::OnKeyUp)
	EVT_CHAR(InputControl::OnChar)
END_EVENT_TABLE()

InputControl::InputControl(
	wxWindow* parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size)
	: wxTextCtrl(parent, id, wxT(""), pos, size, wxTE_MULTILINE /*| wxTE_NO_VSCROLL*/),
	history(),
	history_pos(0), m_ctrl_down(false),
	popup(NULL), m_tab_completion_list(NULL), m_ignore_change(false)
{

	// a whole lot of messing around to get the same font as wxHtmlWindow uses
	{
		wxHtmlWindow *html = new wxHtmlWindow(GetParent());
		wxHtmlWinParser *parser = html->GetParser();
		SetHtmlParserFonts(parser);
		wxClientDC *pDC = new wxClientDC(html);
		parser->SetFontFixed(TRUE);
		parser->SetDC(pDC);
		wxFont *font = parser->CreateCurrentFont();
		SetFont(*font);
		delete pDC;
		html->Destroy();
	}

	txtBestSize = new wxTextCtrl(GetParent(), -1);
	txtBestSize->Show(false);
	txtBestSize->SetFont(GetFont());
	SetSize(GetBestSize());

	#ifdef __WXMSW__
	{
		HWND hWnd = (HWND)GetHandle();
		LONG dwStyle = ::GetWindowLong((HWND)GetHandle(),GWL_STYLE);
		dwStyle &= ~WS_VSCROLL;
		::SetWindowLong(hWnd,GWL_STYLE, dwStyle);
	}
	#endif
	FixBorder(this);

}

InputControl::~InputControl()
{
	ClosePopup();
}

void InputControl::ClosePopup()
{
	if (popup != NULL)
	{
		popup->Show(false);
		popup->Destroy();
		popup = NULL;
	}
}

void InputControl::ShowPopup()
{
	if (popup == NULL)
	{

		popup = new InputControlColourPopup(this, -1, wxT("Colour Index"));
		popup->Show(true);

		wxWindow *wnd = GetParent();
		while (wnd->GetParent() != NULL)
		{
			wnd = wnd->GetParent();
		}
		wnd->SetFocus();
		GetParent()->SetFocus();

	}
}

void InputControl::AddToHistory(const wxString &line)
{
	if ((history.GetCount() == 0) || (history.Last() != line))
	{
		history.Add(line);
	}
	history_pos = history.GetCount();
}

void InputControl::RemoveLastHistoryEntry()
{
	int count = history.GetCount();
	if (count > 0)
	{
		history.Remove(count - 1);
	}
}

void InputControl::SetTabCompletionList(wxArrayString *tab_completion_list)
{
	m_tab_completion_list = tab_completion_list;
}

void InputControl::OnKeyUp(wxKeyEvent& event)
{
	m_ctrl_down = event.ControlDown();
}

void InputControl::OnKeyDown(wxKeyEvent& event)
{

	m_ctrl_down = event.ControlDown();

	if (event.GetKeyCode() != WXK_TAB)
	{
		m_tab_completion_prefix = wxEmptyString;
	}

	if (event.GetKeyCode() == WXK_UP)
	{

		if (history_pos > 0)
		{
			history_pos--;
			DisplayHistory();
		}
		else
		{
			wxBell();
		}

	}
	else if (event.GetKeyCode() == WXK_DOWN)
	{

		if ((history_pos+1) < history.GetCount())
		{
			history_pos++;
			DisplayHistory();
		}
		else
		{
			if (GetValue().Length() > 0)
			{
				SetValue(wxT(""));
			}
			else
			{
				wxBell();
			}
			history_pos = history.GetCount();
		}

	}
	else if (!ModifierCheck(event))
	{

		event.Skip();

	}

}

void InputControl::DisplayHistory()
{
	SetValue(history[history_pos]);
	SetInsertionPointEnd();
}

void InputControl::MaybeClosePopup(wxKeyEvent& event)
{
	if (event.GetKeyCode() == (int)',')
	{
		// comma
	}
	else if (event.GetKeyCode() >= (int)'0' && event.GetKeyCode() <= (int)'9')
	{
		// normal numbers
	}
	else if (event.GetKeyCode() >= WXK_NUMPAD0 && event.GetKeyCode() <= WXK_NUMPAD9)
	{
		// keypad numbers
	}
	else
	{
		ClosePopup();
	}
}

#include <wx/image.h>

bool InputControl::IsPrintable(const wxString &text)
{

	int x, y;
	GetTextExtent(text, &x, &y);

	wxBitmap bitmap(x, y, 1);
	wxMemoryDC dc;
	dc.SelectObject(bitmap);
	dc.SetTextForeground(*wxWHITE);
	dc.DrawText(text, 0, 0);
	dc.SelectObject(wxNullBitmap);

	wxImage image = bitmap.ConvertToImage();
	unsigned char *data = image.GetData();

	size_t length = x * y * 3;

	bool printable = false;

	for (size_t i = 0; i < length; ++i)
	{
		if (data[i])
		{
			printable = true;
			break;
		}
	}

	return printable;

}

bool InputControl::ModifierCheck(wxKeyEvent& event)
{

	for (int i = 0; i < modifier_count; ++i)
	{

		const ModifierMapping &m = modifier_mappings[i];

		if ((m.ctrl     == event.ControlDown()) &&
		    (m.alt      == event.AltDown()) &&
		    (m.shift    == event.ShiftDown()) &&
		    (m.key_code == event.GetKeyCode()))
		{

			wxString modifier =
				IsPrintable(m.modifier_char)
				? wxString(m.modifier_char)
				: wxString(m.alternate_modifier);

			AddModifier(modifier, m.is_colour_popup);
			if (m.is_colour_popup)
			{
				ShowPopup();
			}
			else
			{
				MaybeClosePopup(event);
			}
			return true;

		}

	}

	MaybeClosePopup(event);
	return false;

}

void InputControl::AddModifier(const wxString &modifier, bool is_colour_popup)
{
	
	long from, to;
	GetSelection(&from, &to);

	if (from == to)
	{

		WriteText(modifier);

	}
	else
	{

		SetSelection(to, to);
		SetInsertionPoint(to);
		WriteText(modifier);
		SetInsertionPoint(from);
		WriteText(modifier);

		if (!is_colour_popup)
		{
			SetInsertionPoint( to + (modifier.Length() * 2) );
		}

		SetSelection(GetInsertionPoint(), GetInsertionPoint());

	}
	
}

void InputControl::RaiseEvent(const wxString &text)
{
	wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER, GetId());
	event.SetEventObject(this);
	event.SetString(text);
	ProcessEvent(event);
}

void InputControl::OnChar(wxKeyEvent& event)
{
	if ( event.KeyCode() == WXK_RETURN )
	{
		m_tab_completion_prefix = wxEmptyString;
		ProcessInput();
	}
	else if ( event.KeyCode() == WXK_TAB )
	{

		wxString val = GetValue();
		int index = val.Find(wxT(' '), true);
		if (index > -1)
		{
			val = val.Mid(index + 1);
		}

		long from, to;
		GetSelection(&from, &to);
		if (from > to)
		{
			long tmp = from;
			from = to;
			to = tmp;
		}
		if (GetLastPosition() != to)
		{
			m_tab_completion_prefix = wxEmptyString;
			wxBell();
			return;
		}

		if (val.Length() > 0)
		{
			if (m_tab_completion_prefix.Length() == 0)
			{
				m_tab_completion_prefix = val;
			}
			
			wxArrayString possibles;
			int current_index = -1;
			for (size_t i = 0; i < m_tab_completion_list->GetCount(); ++i)
			{
				if (LeftEq(m_tab_completion_list->Item(i).Lower(), m_tab_completion_prefix.Lower()))
				{
					possibles.Add(m_tab_completion_list->Item(i));
					if (current_index == -1 && m_tab_completion_list->Item(i).CmpNoCase(val) == 0)
					{
						current_index = (int)possibles.GetCount()-1;
					}
				}
			}

			if (possibles.GetCount() == 1 && current_index == 0)
			{
				m_tab_completion_prefix = wxEmptyString;
				wxBell();
				return;
			}
			if (possibles.GetCount() > 0 && (current_index > -1 || m_tab_completion_prefix == val))
			{
				current_index++;
				if (current_index >= (int)possibles.GetCount())
				{
					current_index = 0;
				}
				wxString newval = GetValue();
				newval =
					newval.Left(newval.Length()-val.Length()) +
					possibles.Item(current_index);
				m_ignore_change = true;
				SetValue(newval);
				SetInsertionPointEnd();
			}
			else
			{
				m_tab_completion_prefix = wxEmptyString;
				wxBell();
				return;
			}
			
		}
		else
		{
			m_tab_completion_prefix = wxEmptyString;
			wxBell();
			return;
		}

	}
	else
	{
		m_tab_completion_prefix = wxEmptyString;
		event.Skip();
	}
}

void InputControl::OnChange(wxCommandEvent &event)
{
	if (m_ignore_change)
	{
		m_ignore_change = false;
	}
	else
	{
		m_tab_completion_prefix = wxEmptyString;
	}
	if (GetValue().Find(wxT("\n")) > -1)
	{
		ProcessInput();
	}
}

void InputControl::OnEnterPress(wxCommandEvent &event)
{
	m_tab_completion_prefix = wxEmptyString;
	AddToHistory(event.GetString());
	event.SetString(ReplaceAlternateModifiers(event.GetString()));
	if (m_ctrl_down)
	{
		event.SetString(CtrlEnterPrefix + event.GetString());
	}
	event.Skip();
}

void InputControl::ProcessInput()
{

	wxString text(GetValue());
	SetValue(wxT(""));

	text.Replace(wxT("\r\n"), wxT("\n"));
	text.Replace(wxT("\r"), wxT("\n"));

	while ((text.Length() > 0) && (text[0] == wxT('\n')))
	{
		text.Remove(0,1);
	}
	while ((text.Length() > 0) && (text.Last() == wxT('\n')))
	{
		text.Remove(text.Length()-1,1);
	}

	if (text.Strip(wxString::both).Length() > 0)
	{

		int i;
		while ((i = text.Find(wxT("\n"))) > -1)
		{
			RaiseEvent(text.Mid(0, i));
			text = text.Mid(i+1);
		}

		if (text.Length() > 0)
		{
			RaiseEvent(text);
		}

	}

}

wxSize InputControl::DoGetBestSize() const
{
	return txtBestSize->GetBestSize();
}
