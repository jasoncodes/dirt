#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SwitchBar.cpp,v 1.10 2003-06-17 08:31:19 jason Exp $)

#include "SwitchBar.h"
#include <wx/image.h>
#include <wx/tooltip.h>
#include "FileDropTarget.h"

struct SwitchBarButton
{

	wxString caption;
	wxIcon icon;
	void *user_data;
	bool highlight;
	int progress;

	SwitchBarButton(const wxString &the_caption, const wxIcon &the_icon, void *the_user_data, bool the_highlight, int the_progress)
		: caption(the_caption), icon(the_icon), user_data(the_user_data), highlight(the_highlight), progress(the_progress)
	{
	}

};

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(SwitchBarButtonArray);

BEGIN_EVENT_TABLE(SwitchBar, wxPanel)
	EVT_ERASE_BACKGROUND(SwitchBar::OnErase)
	EVT_PAINT(SwitchBar::OnPaint)
	EVT_LEFT_DOWN(SwitchBar::OnMouse)
	EVT_LEFT_DCLICK(SwitchBar::OnMouse)
	EVT_RIGHT_UP(SwitchBar::OnMouse)
	EVT_MOTION(SwitchBar::OnMouse)
	EVT_FILE_DROP(wxID_ANY, SwitchBar::OnFileDrop)
END_EVENT_TABLE()

SwitchBar::SwitchBar(
	wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	long style, const wxString& name)
	: wxPanel(parent, id, pos, size, style, name), m_selected(-1)
{
	if (size == wxDefaultSize)
	{
		SetSize(100,25);
	}
	SetDropTarget(new FileDropTarget(this, wxID_ANY));
}

SwitchBar::~SwitchBar()
{
}

void SwitchBar::OnFileDrop(FileDropEvent &event)
{
	int index = HitTest(ScreenToClient(wxGetMousePosition()));
	if (index > -1 && index != GetSelectedIndex())
	{
		SimulateClick(index);
	}
	event.Accept(false);
}

int SwitchBar::GetButtonWidth()
{
	int max_width = GetClientRect().GetWidth();
	int button_width = 128;
	if ((button_width+2) * GetButtonCount() > max_width)
	{
		button_width = (max_width / GetButtonCount()) - 2;
		if (button_width < 32)
		{
			button_width = 32;
		}
	}
	return button_width;
}

wxRect SwitchBar::GetButtonRect(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return wxRect(2*(button_index+1) + GetButtonWidth()*button_index,2,GetButtonWidth(),22);
}

static void IndentLeft(wxRect &r, int amount)
{
	r.x += amount;
	r.width -= amount;
}

static void IndentTop(wxRect &r, int amount)
{
	r.y += amount;
	r.height -= amount;
}

void SwitchBar::OnErase(wxEraseEvent &event)
{

	wxDC *dc = event.GetDC();
	bool own_dc = false;

	if (!dc)
	{
		own_dc = true;
		dc = new wxClientDC(this);
	}

	dc->SetBrush(wxBrush(GetBackgroundColour(), wxSOLID));
	dc->SetPen(*wxTRANSPARENT_PEN);

	wxSize size = GetSize();

	dc->DrawRectangle(0, 0, size.x, 2);
	dc->DrawRectangle(0, 23, size.x, 4);

	for (int i = 0; i < GetButtonCount(); ++i)
	{
		wxRect rct = GetButtonRect(i);
		dc->DrawRectangle(rct.x - 2, 2, 2, 21);
	}

	int x;
	if (GetButtonCount() > 0)
	{
		wxRect rct = GetButtonRect(GetButtonCount() - 1);
		x = rct.x + rct.width;
	}
	else
	{
		x = 0;
	}
	dc->DrawRectangle(x, 2, size.x - x, 21);

	if (own_dc)
	{
		delete dc;
	}

}

void SwitchBar::OnPaint(wxPaintEvent &event)
{
	
	wxPaintDC dc(this);

	dc.SetOptimization(false);
	dc.SetFont(GetFont());

	for (int i = 0; i < GetButtonCount(); ++i)
	{

		SwitchBarButton &button = m_buttons.Item(i);

		wxRect rct = GetButtonRect(i);

		bool bSelected = (m_selected == i);

		if (bSelected)
		{
			dc.SetPen(*wxTRANSPARENT_PEN);
			wxBitmap stipple("\x0aa\x055\x0aa\x055\x0aa\x055\x0aa\x055", 8, 8, 1);
			dc.SetTextBackground(GetBackgroundColour());
			dc.SetTextForeground(*wxWHITE);
			stipple.SetMask(new wxMask(stipple));
			dc.SetBrush(wxBrush(stipple));
		}
		else
		{
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush(GetBackgroundColour(), wxSOLID));
		}
		dc.DrawRectangle(rct.GetLeft(), rct.GetTop(), rct.GetWidth(), rct.GetHeight() - 1);

		dc.SetTextForeground(GetForegroundColour());
		dc.SetTextBackground(GetBackgroundColour());
		if (bSelected)
		{
			wxBitmap un_stipple("\0\0\0\0\0\0\0\0", 8, 8, 1);
			un_stipple.SetMask(new wxMask(un_stipple));
			dc.SetBrush(wxBrush(un_stipple));
		}

		dc.SetPen( bSelected ? *wxGREY_PEN : *wxWHITE_PEN);
		dc.DrawLine( rct.GetLeft(), rct.GetTop(), rct.GetRight() + 1, rct.GetTop() );
		dc.DrawLine( rct.GetLeft(), rct.GetTop(), rct.GetLeft(), rct.GetBottom() );

		dc.SetPen( bSelected ? *wxWHITE_PEN : *wxGREY_PEN);
		dc.DrawLine( rct.GetLeft() + 1, rct.GetBottom() - 1, rct.GetRight() + 1, rct.GetBottom() - 1 );
		dc.DrawLine( rct.GetRight(), rct.GetTop() + 1, rct.GetRight(), rct.GetBottom() );

		if (bSelected)
		{
			IndentLeft(rct, 1);
			IndentTop(rct, 1);
		}

		wxRect button_pos = rct;

		if (button.icon.Ok())
		{
			IndentLeft(rct, button.icon.GetWidth() + 2);
		}

		int x, y;
		dc.GetTextExtent(button.caption, &x, &y);

		IndentLeft(rct, 2);
		if (bSelected)
		{
			IndentTop(rct, -1);
		}
		IndentTop(rct, (rct.GetHeight() - y) / 2);
		if (bSelected)
		{
			IndentTop(rct, 1);
		}

		wxString tmp = button.caption;
		dc.GetTextExtent(tmp, &x, &y);

		if (x >= rct.GetWidth())
		{
			do
			{
				tmp = tmp.Left(tmp.Length() - 1);
				dc.GetTextExtent(tmp + wxT("..."), &x, &y);
			}
			while (x >= rct.GetWidth());
			tmp += wxT("...");
		}

		if (button.highlight)
		{
			dc.SetTextForeground(*wxRED);
		}
		dc.DrawText(tmp, rct.x, rct.y);

		if (button.progress > 0)
		{
			wxRect progress_rect = GetButtonRect(i);
			progress_rect.Offset(1, 1);
			progress_rect.width -= 2;
			progress_rect.height -= 3;
			progress_rect.width = (int)(progress_rect.width * (button.progress / 100.00));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(*wxWHITE_BRUSH);
			dc.SetLogicalFunction(wxINVERT);
			dc.DrawRectangle(progress_rect);
			dc.SetLogicalFunction(wxCOPY);
		}

		if (button.icon.Ok())
		{
			dc.DrawBitmap(button.icon, button_pos.x + 2, button_pos.y + 2, true);
		}

	}

}

void SwitchBar::RaiseEvent(int triggering_button, bool is_right_click)
{
	wxASSERT(triggering_button > -1 && triggering_button < GetButtonCount());
	SwitchBarButton &button = m_buttons.Item(triggering_button);
	wxCommandEvent cmd(is_right_click?wxEVT_COMMAND_MENU_SELECTED:wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	cmd.SetInt(m_selected);
	cmd.SetString(button.caption);
	cmd.SetClientData(button.user_data);
	cmd.SetExtraLong(triggering_button);
	ProcessEvent(cmd);
}

void SwitchBar::SimulateClick(int button_index)
{
	SelectButton((m_selected == button_index) ? -1 : button_index);
	RaiseEvent(button_index, false);
}

int SwitchBar::HitTest(const wxPoint& pt)
{
	for (int i = 0; i < GetButtonCount(); ++i)
	{
		if (GetButtonRect(i).Inside(pt))
		{
			return i;
		}
	}
	return -1;
}

void SwitchBar::OnMouse(wxMouseEvent &event)
{
	if (event.GetEventType() == wxEVT_MOTION)
	{
		#if wxUSE_TOOLTIPS
			int button_index = HitTest(event.GetPosition());
			wxString tooltip = wxEmptyString;
			if (button_index > -1)
			{
				tooltip = GetButtonCaption(button_index);
			}
			SetToolTip(new wxToolTip(tooltip)); 
		#endif
	}
	else if (event.GetEventType() == wxEVT_RIGHT_UP)
	{
		int button_index = HitTest(event.GetPosition());
		if (button_index > -1)
		{
			RaiseEvent(button_index, true);
		}
	}
	else
	{
		if (event.ButtonDown(1) || event.LeftDClick())
		{
			int button_index = HitTest(event.GetPosition());
			if (button_index > -1)
			{
				SimulateClick(button_index);
			}
		}
	}
}

void SwitchBar::SelectButton(int button_index)
{
	wxASSERT(button_index >= -1 && button_index < GetButtonCount());
	if (m_selected != button_index)
	{
		m_selected = button_index;
		Refresh();
	}
}

int SwitchBar::GetButtonCount()
{
	return m_buttons.GetCount();
}

wxIcon SwitchBar::PrepareIcon(const wxIcon &icon)
{
	wxIcon new_icon = icon;
	if (new_icon.Ok() && (new_icon.GetWidth() != 16 || new_icon.GetHeight() != 16))
	{
		wxFAIL_MSG(wxT("Icon must be 16x16"));
	}
	return new_icon;
}

int SwitchBar::AddButton(const wxString &caption, const wxIcon &icon, void *user_data)
{
	m_buttons.Add(SwitchBarButton(caption, PrepareIcon(icon), user_data, false, 0));
	Refresh();
	return GetButtonCount() - 1;
}

void SwitchBar::RemoveButton(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	m_buttons.RemoveAt(button_index);
	if (button_index == m_selected)
	{
		m_selected = -1;
	}
	else if (button_index < m_selected)
	{
		m_selected--;
	}
	Refresh();
}

void SwitchBar::SetButtonCaption(int button_index, const wxString &caption)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	if (m_buttons.Item(button_index).caption != caption)
	{
		m_buttons.Item(button_index).caption = caption;
		Refresh();
	}
}

wxString SwitchBar::GetButtonCaption(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return m_buttons.Item(button_index).caption;
}

void SwitchBar::SetButtonIcon(int button_index, const wxIcon &icon)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	m_buttons.Item(button_index).icon = PrepareIcon(icon);
	Refresh();
}

wxIcon SwitchBar::GetButtonIcon(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return m_buttons.Item(button_index).icon;
}

void SwitchBar::SetButtonHighlight(int button_index, bool new_value)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	if (m_buttons.Item(button_index).highlight != new_value)
	{
		m_buttons.Item(button_index).highlight = new_value;
		Refresh();
	}
}

bool SwitchBar::GetButtonHighlight(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return m_buttons.Item(button_index).highlight;
}

void SwitchBar::SetButtonProgress(int button_index, int new_value)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	wxASSERT(new_value >= 0 && new_value <= 100);
	if (m_buttons.Item(button_index).progress != new_value)
	{
		m_buttons.Item(button_index).progress = new_value;
		Refresh();
	}
}

int SwitchBar::GetButtonProgress(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return m_buttons.Item(button_index).progress;
}

int SwitchBar::GetIndexFromUserData(void *user_data)
{
	for (int i = 0; i < GetButtonCount(); ++i)
	{
		if (m_buttons.Item(i).user_data == user_data)
		{
			return i;
		}
	}
	return -1;
}

void *SwitchBar::GetUserDataFromIndex(int button_index)
{
	wxASSERT(button_index > -1 && button_index < GetButtonCount());
	return m_buttons.Item(button_index).user_data;
}

int SwitchBar::GetSelectedIndex()
{
	return m_selected;
}
