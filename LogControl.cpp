#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogControl.cpp,v 1.26 2003-03-10 08:26:07 jason Exp $)

#include <wx/image.h>
#include <wx/sysopt.h>
#include <wx/html/winpars.h>
#include <wx/tokenzr.h>
#include <ctype.h>
#include <wx/clipbrd.h>
#include "util.h"

#include "LogControl.h"
#include "SpanTag.h"
#include "Modifiers.h"


struct ModifierParserTag
{
	bool active;
	const wxString name;

	ModifierParserTag(const wxString &the_name, bool initially_active = false)
		: name(the_name), active(initially_active)
	{
	}
};

struct ModifierParserTagEntry
{
	
	ModifierParserTag &tag;
	wxString attribs;

	ModifierParserTagEntry(ModifierParserTag &the_tag, const wxString &the_attribs)
		: tag(the_tag), attribs(the_attribs)
	{
	}
};

#include <wx/dynarray.h>
#include <wx/arrimpl.cpp>

WX_DECLARE_OBJARRAY(ModifierParserTagEntry, ModifierParserTagEntryArray);
WX_DEFINE_OBJARRAY(ModifierParserTagEntryArray);

class ModifierParserTagEntryStack
{

public:
	ModifierParserTagEntryStack(size_t count = 10)
	{
		m_entries.Alloc(count);
	}

	void Push(ModifierParserTagEntry item)
	{
		m_entries.Add(item);
	}

	ModifierParserTagEntry Pop()
	{
		wxASSERT_MSG(!m_entries.IsEmpty(), wxT("Stack is empty"));
		ModifierParserTagEntry item = m_entries.Last();
		m_entries.RemoveAt(m_entries.GetCount() - 1);
		return item;
	}

	bool IsEmpty()
	{
		return m_entries.IsEmpty();
	}

	void Empty()
	{
		m_entries.Empty();
	}

protected:
	ModifierParserTagEntryArray m_entries;

};

static wxString ColourToString(const wxColour &clr)
{
	return wxString::Format(
		wxT("#%02x%02x%02x"),
		clr.Red(), clr.Green(), clr.Blue());
}

static wxString ColourToString(int colour_number)
{
	return ColourToString(colours[colour_number % colour_count]);
}

const bool debug_tags = false;

class ModifierParser
{

public:
	ModifierParser()
		: tagFont(wxT("font")), tagSpan(wxT("span")), tagBold(wxT("b")), tagUnderline(wxT("u"))
	{
		strip_mode = false;
		ResetAllState();
	}
	
public:
	bool strip_mode;

protected:
	wxString result;
	int colour_pos;
	bool had_comma;
	bool last_was_comma;
	int colour_number[2];
	bool colour_number_valid[2];

	ModifierParserTagEntryStack tag_stack;

	ModifierParserTag tagFont;
	ModifierParserTag tagSpan;
	ModifierParserTag tagBold;
	ModifierParserTag tagUnderline;

	ModifierParserTagEntryArray reverse_tags;
	bool reverse_mode;

protected:
	void ResetAllState()
	{
		result.Empty();
		ResetColourState();
		tag_stack.Empty();
		tagFont.active = false;
		tagSpan.active = false;
		tagBold.active = false;
		tagUnderline.active = false;
		reverse_tags.Empty();
		reverse_mode = false;
	}

	void ResetColourState()
	{
		colour_pos = 0;
		had_comma = false;
		last_was_comma = false;
		colour_number[0] = 0;
		colour_number[1] = 0;
		colour_number_valid[0] = false;
		colour_number_valid[1] = false;
	}

	void end_tag_helper(ModifierParserTag &t)
	{
		if (!strip_mode)
		{
			if (debug_tags)
			{
				result << wxT("&lt;/") << t.name << wxT("&gt;");
			}
			else
			{
				result << wxT("</") << t.name << wxT(">");
			}
		}
	}

	void start_tag_helper(ModifierParserTag &t, const wxString &attribs)
	{
		if (!strip_mode)
		{
			if (debug_tags)
			{
				result << wxT("&lt;") << t.name << wxT(" ") << attribs << wxT("&gt;");
			}
			else
			{
				result << wxT("<") << t.name << wxT(" ") << attribs << wxT(">");
			}
		}
	}

	wxString end_tag(ModifierParserTag &t)
	{
		wxString last_attribs = wxEmptyString;
		if (t.active)
		{
			ModifierParserTagEntryStack undo_stack;
			while (true)
			{
				ModifierParserTagEntry entry = tag_stack.Pop();
				if (entry.tag.name == t.name)
				{
					last_attribs = entry.attribs;
					break;
				}
				undo_stack.Push(entry);
				end_tag_helper(entry.tag);
			}
			end_tag_helper(t);
			t.active = false;
			while (!undo_stack.IsEmpty())
			{
				ModifierParserTagEntry entry = undo_stack.Pop();
				tag_stack.Push(entry);
				start_tag_helper(entry.tag, entry.attribs);
			}
		}
		return last_attribs;
	}

	wxString start_tag(ModifierParserTag &t, const wxString attribs = wxEmptyString)
	{
		wxString last_attribs = end_tag(t);
		t.active = true;
		tag_stack.Push(ModifierParserTagEntry(t, attribs));
		start_tag_helper(t, attribs);
		return last_attribs;
	}

	void toggle_tag(ModifierParserTag &t)
	{
		if (t.active)
		{
			end_tag(t);
		}
		else
		{
			start_tag(t);
		}
	}

	void CleanupStack()
	{
		while (!tag_stack.IsEmpty())
		{
			ModifierParserTagEntry entry = tag_stack.Pop();
			entry.tag.active = false;
			end_tag_helper(entry.tag);
		}
	}

	void start_colour_tag(ModifierParserTag &t, wxString attribs)
	{
		if (reverse_mode)
		{
			reverse_tags.Add(ModifierParserTagEntry(t, attribs));
		}
		else
		{
			start_tag(t, attribs);
		}
	}

	void end_colour_tag(ModifierParserTag &t)
	{
		if (reverse_mode)
		{
			reverse_tags.Add(ModifierParserTagEntry(t, wxT("")));
		}
		else
		{
			end_tag(t);
		}
	}

	void EndOfColourCode()
	{
		if (colour_pos == 1 && !colour_number_valid[0])
		{
			end_colour_tag(tagSpan);
			end_colour_tag(tagFont);
		}
		else if (colour_number_valid[0])
		{
			start_colour_tag(tagFont, wxT("color=\"") + ColourToString(colour_number[0]) + wxT("\""));
			if (colour_number_valid[1])
			{
				start_colour_tag(tagSpan, wxT("style=\"background: ") + ColourToString(colour_number[1]) + wxT("\""));
			}
		}
		colour_number_valid[0] = false;
		colour_number_valid[1] = false;
		colour_pos = 0;
	}

	void ReverseTag()
	{
		reverse_mode = !reverse_mode;
		if (reverse_mode)
		{
			
			wxString font_attribs = start_tag(tagFont, wxT("color=white"));
			wxString span_attribs = start_tag(tagSpan, wxT("style='background: black'"));

			reverse_tags.Empty();

			if (font_attribs.Length() > 0)
			{
				reverse_tags.Add(ModifierParserTagEntry(tagFont, font_attribs));
			}
			if (span_attribs.Length() > 0)
			{
				reverse_tags.Add(ModifierParserTagEntry(tagSpan, span_attribs));
			}

		}
		else
		{
			end_tag(tagSpan);
			end_tag(tagFont);

			for (size_t i = 0; i < reverse_tags.GetCount(); ++i)
			{
				ModifierParserTagEntry entry = reverse_tags.Item(i);
				if (entry.attribs.Length() > 0)
				{
					start_tag(entry.tag, entry.attribs);
				}
				else
				{
					end_tag(entry.tag);
				}
			}
		}
	}

public:
	wxString Parse(const wxString &text)
	{

		wxCOMPILE_TIME_ASSERT(modifier_count == 5, UnexpectedNumberOfModifiers);

		ResetAllState();
		result.Alloc(text.Length() * 3);
		reverse_tags.Alloc(10);

		for (size_t i = 0; i < text.Length(); ++i)
		{

			if (colour_pos == 0)
			{
				ResetColourState();
			}

			char c = text[i];

			if ( colour_pos > 0 && // is inside a colour code and
				( (c == wxT(',') && had_comma) || // is either a 2nd comma
				  (c != wxT(',') && !wxIsdigit(c)) ) // or a non-valid character
				)
			{
				EndOfColourCode();
				had_comma = false;
				if (last_was_comma)
				{
					result << wxT(',');
				}
			}

			switch (c)
			{

				case BoldModifier:
					EndOfColourCode();
					toggle_tag(tagBold);
					break;

				case OriginalModifier:
					EndOfColourCode();
					CleanupStack();
					break;

				case ReverseModifier: // not implemented yet
					EndOfColourCode();
					ReverseTag();
					break;

				case UnderlineModifier:
					EndOfColourCode();
					toggle_tag(tagUnderline);
					break;

				case ColourModifier:
					ResetColourState();
					colour_pos = 1;
					break;

				case wxT('1'): case wxT('2'): case wxT('3'):
				case wxT('4'): case wxT('5'): case wxT('6'):
				case wxT('7'): case wxT('8'): case wxT('9'):
				case wxT('0'): // isdigit()
					if (colour_pos > 0)
					{
						colour_pos++;
						if (colour_pos > 3)
						{
							EndOfColourCode();
						}
						else
						{
							int x = had_comma ? 1 : 0;
							if (!colour_number_valid[x])
							{
								colour_number[x] = 0;
								colour_number_valid[x] = true;
							}
							colour_number[x] *= 10;
							colour_number[x] += (c - wxT('0'));
						}
					}
					if (colour_pos == 0)
					{
						result << c;
					}
					break;

				case wxT(','):
					if (colour_pos > 0)
					{
						if (had_comma)
						{
							EndOfColourCode();
							result << c << c;
						}
						else
						{
							colour_pos = 1;
							had_comma = true;
						}
					}
					else
					{
						result << c;
					}
					break;

				default:
					EndOfColourCode();
					result << c;
					break;

			}

			last_was_comma = (c == wxT(','));

		}

		EndOfColourCode();

		CleanupStack();

		return result;

	}

};


bool LogControl::s_bInitDone = false;

BEGIN_EVENT_TABLE(LogControl, wxHtmlWindow)
	EVT_SIZE(LogControl::OnSize)
	EVT_ERASE_BACKGROUND(LogControl::OnErase)
	EVT_IDLE(LogControl::OnIdle)
	EVT_MOUSE_EVENTS(LogControl::OnMouseEvent)
END_EVENT_TABLE()

LogControl::LogControl(wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	bool align_bottom)
	: wxHtmlWindow(parent, id, pos, size, wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER ),
	m_align_bottom(align_bottom)
{

	if (!s_bInitDone)
	{
	
		#if wxUSE_SYSTEM_OPTIONS
			wxSystemOptions::SetOption(wxT("no-maskblt"), 1);
		#endif

		wxInitAllImageHandlers();

		s_bInitDone = true;

	}

	m_tmpMouseMoved = FALSE;
	m_cur_hand = NULL;
	m_cur_arrow = NULL;
	m_tmpLastLink = NULL;
	m_tmpLastCell = NULL;

	last_start_cell = NULL;
	last_end_cell = NULL;
	last_start_end_valid = false;

	m_Resizing = false;
	m_iYOffset = 0;

	FixBorder(this);
	SetBorders(1);

	GetParser()->AddTagHandler(new SpanTagHandler());
	SetHtmlParserFonts(GetParser());

	Clear();

}

LogControl::~LogControl()
{
	delete m_cur_hand;
	delete m_cur_arrow;
}

void LogControl::OnSize(wxSizeEvent& event)
{

	if (GetSize().x > 0 && GetSize().y > 0)
	{

		#ifdef __WXMSW__
			m_Resizing = true;
		#endif

		wxHtmlWindow::OnSize(event);
		CalculateOffset();

		Refresh();

		#ifndef __WXMSW__
			Update();
			ScrollToBottom();
		#endif

	}

}

void LogControl::CalculateOffset()
{

	if (m_align_bottom)
	{

		wxSize virtual_size = GetVirtualSize();
		wxSize view_size = GetClientSize();

		m_iYOffset = view_size.y - m_Cell->GetHeight();

		if ((virtual_size.y - view_size.y) > 0)
		{
			m_iYOffset = view_size.y - m_iYSize;
		}

	}
	else
	{

		m_iYOffset = false;

	}

}

void LogControl::ClearRect(wxDC& dc, wxRect &rect)
{
	wxPen old_pen = dc.GetPen();
	wxBrush old_brush = dc.GetBrush();
	dc.SetPen(*wxWHITE_PEN);
	dc.DrawRectangle(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), rect.GetHeight());
	dc.SetBrush(old_brush);
	dc.SetPen(old_pen);
}

void LogControl::ClearBlankArea(wxDC &dc)
{

	dc.SetLogicalFunction(wxCOPY);

	wxRect rtClearArea;

	int x, y;
	wxScrolledWindow::GetViewStart(&x, &y);
	y *= wxHTML_SCROLL_STEP;
	y -= m_iYOffset;

	int bottom_gap_size =
		GetVirtualSize().y - m_Cell->GetHeight() - m_iYOffset;

	rtClearArea.x = 0;
	rtClearArea.width = GetClientSize().x;
	if (y <= 0)
	{

		rtClearArea.y = 0;
		rtClearArea.height = m_iYOffset;

		ClearRect(dc, rtClearArea);

	}

	if (bottom_gap_size > 0)
	{

		rtClearArea.height = GetClientSize().y;
		rtClearArea.y = GetClientSize().y - bottom_gap_size;

		ClearRect(dc, rtClearArea);

	}

}

void LogControl::OnErase(wxEraseEvent& event)
{

	wxDC *pDC = event.GetDC();
	bool bOwnDC = (pDC == NULL);

	if (bOwnDC)
	{
		pDC = new wxClientDC(this);
	}

	PrepareDC(*pDC);

	OnDraw(*pDC);

	if (bOwnDC)
	{
		delete pDC;
	}

}

#define USE_BACKBUFFER 1

void LogControl::OnDraw(wxDC& dcFront)
{

	if (m_Resizing)
	{

		m_Resizing = false;

		ScrollToBottom();

		Refresh();
	
	}
	else
	{

		if (/*m_tmpCanDrawLocks > 0 ||*/ m_Cell == NULL) return;

		GetInternalRepresentation()->SetBackgroundColour(*wxWHITE);

		CalculateOffset();

		wxRect client_area = GetClientRect();

#if USE_BACKBUFFER

		static wxSize BitmapSize = wxSize(
			wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
			wxSystemSettings::GetMetric(wxSYS_SCREEN_Y));
		static wxBitmap bmpBack(BitmapSize.x, BitmapSize.y);
		wxSize ClientSize = GetClientSize();
		if (ClientSize.x > BitmapSize.x || ClientSize.y > BitmapSize.y)
		{
			if (ClientSize.x > BitmapSize.x)
			{
				BitmapSize.x = ClientSize.x;
			}
			if (ClientSize.y > BitmapSize.y)
			{
				BitmapSize.y = ClientSize.y;
			}
			bmpBack = wxBitmap(BitmapSize.x, BitmapSize.y);
			wxBell();
		}
		wxMemoryDC dcBack;
		dcBack.SelectObject(bmpBack);
		PrepareDC(dcBack);

#else

		wxDC &dcBack = dcFront;

#endif
		
		ClearBlankArea(dcBack);

		int x, y;
		wxScrolledWindow::GetViewStart(&x, &y);
		x *= wxHTML_SCROLL_STEP;
		y *= wxHTML_SCROLL_STEP;

		wxRect rect = GetUpdateRegion().GetBox();

		dcBack.SetMapMode(wxMM_TEXT);

		m_Cell->Draw(
			dcBack,
			0, m_iYOffset,
			rect.GetTop() + y - 32,
			rect.GetBottom() + y + 32);

		if (m_red_line)
		{
			wxPen old_pen = dcBack.GetPen();
			dcBack.SetPen(*wxRED_PEN);
			wxRect red_rect = GetCellRect(m_red_line);
			dcBack.DrawLine(0, red_rect.y, GetClientSize().x, red_rect.y);
			dcBack.SetPen(old_pen);
		}

		if (last_start_end_valid)
		{
			InitDCForHighlighting(dcBack);
			HighlightCells(dcBack, last_start_cell, last_end_cell);
		}

#if USE_BACKBUFFER

		dcFront.Blit(x, y, client_area.width, client_area.height, &dcBack, x, y);
		dcBack.SelectObject(wxNullBitmap);

#endif

	}

}

wxPoint LogControl::GetVirtualMousePosition()
{

	int sx, sy;
	wxScrolledWindow::GetViewStart(&sx, &sy);
	sx *= wxHTML_SCROLL_STEP;
	sy *= wxHTML_SCROLL_STEP;
	sy -= m_iYOffset;

	int x, y;
	wxGetMousePosition(&x, &y);
	ScreenToClient(&x, &y);
	x += sx;
	y += sy;

	return wxPoint(x, y);

}

struct LogControlWordCell : public wxHtmlCell
{
	LogControlWordCell(const wxString& word, wxDC& dc);
	virtual void Draw(wxDC& dc, int x, int y, int view_y1, int view_y2);
	wxString m_Word;
};

wxString LogControl::GetCellText(wxHtmlCell *cell)
{

	if (cell->IsTerminalCell() && cell->GetWidth() > 0 && cell->GetHeight() > 0)
	{
		wxASSERT(sizeof LogControlWordCell == sizeof wxHtmlWordCell);
		LogControlWordCell *word_cell = (LogControlWordCell*)cell;
		return wxString(word_cell->m_Word.c_str());
	}

	return wxEmptyString;

}

wxRect LogControl::GetCellRect(wxHtmlCell *cell)
{
	
	wxRect rt(
		cell->GetPosX(), cell->GetPosY(),
		cell->GetWidth(), cell->GetHeight());
	
	wxHtmlCell *parent = cell->GetParent();

	while (parent != NULL)
	{
		rt.x += parent->GetPosX();
		rt.y += parent->GetPosY();
		parent = parent->GetParent();
	}

	rt.y += m_iYOffset;

	return rt;

}

wxHtmlCell *LogControl::GetCellAt(const wxPoint &pos)
{
	return m_Cell->FindCellByPos(pos.x, pos.y);
}

wxHtmlCell *LogControl::FindCell(const wxPoint &start_pos)
{
	
	wxPoint pos = start_pos;
	wxHtmlCell *cell = GetCellAt(pos);
	
	if (start_pos.y < 0)
	{

		pos.y = 4;
		cell = GetCellAt(pos);

	}

	while (cell == NULL && pos.y >= 0)
	{

		if (pos.x <= 0)
		{
			pos.x = 8;
		}

		while (cell == NULL && pos.x >= 0)
		{
			pos.x -= 4;
			cell = GetCellAt(pos);
		}

		pos.x = start_pos.x;
		pos.y -= 4;

	}

	return cell;

}

static wxHtmlCell *FindNext(wxHtmlCell *cell)
{
	if (!cell->IsTerminalCell())
	{
		wxHtmlContainerCell *container = (wxHtmlContainerCell*)cell;
		wxHtmlCell *child = container->GetFirstCell();
		if (child)
		{
			return child;
		}
	}

	if (cell->GetNext())
	{
		return cell->GetNext();
	}

	wxHtmlCell *parent = cell->GetParent();
	if (parent)
	{
		return parent->GetNext();
	}

	return NULL;

}

static void FirstCellFirst(wxHtmlCell **start_cell, wxHtmlCell **end_cell)
{
	wxHtmlCell *cell = *start_cell;
	while (cell != NULL)
	{
		if (cell == *end_cell) break;
		cell = FindNext(cell);
	}
	if (cell == NULL)
	{
		cell = *start_cell;
		*start_cell = *end_cell;
		*end_cell = cell;
	}
}

wxString LogControl::GetTextFromRange(wxHtmlCell *start_cell, wxHtmlCell *end_cell)
{
	wxString buffer;
	wxHtmlCell *cell = start_cell;
	bool last_was_new_line = false;
	while (cell != NULL)
	{
		if (cell->IsTerminalCell())
		{
			wxString text = GetCellText(cell);
			if (text.Length() > 0)
			{
				if (text == wxT("    "))
				{
					buffer += wxT('\t');
				}
				else
				{
					buffer += text;
				}
				last_was_new_line = false;
			}
		}
		else
		{
			if (!last_was_new_line)
			{
				buffer += wxT("\r\n");
				last_was_new_line = true;
			}
		}
		if (cell == end_cell) break;
		cell = FindNext(cell);
	}
	return buffer; 
}

void LogControl::HighlightCells(wxDC &dc, wxHtmlCell *start_cell, wxHtmlCell *end_cell)
{
	wxHtmlCell *cell = start_cell;
	while (cell != NULL)
	{
		if (cell->IsTerminalCell())
		{
			dc.DrawRectangle(GetCellRect(cell));
		}
		if (cell == end_cell) break;
		cell = FindNext(cell);
	}
}

void LogControl::HighlightCellsDiff(wxDC &dc, wxHtmlCell *start_cell1, wxHtmlCell *end_cell1, wxHtmlCell *start_cell2, wxHtmlCell *end_cell2)
{

	if (start_cell1 == start_cell2)
	{

		if (end_cell1 != end_cell2)
		{	

			wxHtmlCell *cell = start_cell1;

			while (cell != end_cell1 && cell != end_cell2)
			{
				cell = FindNext(cell);
			}

			wxASSERT((cell == end_cell1) ^ (cell == end_cell2));

			if (cell == end_cell1)
			{
				HighlightCells(dc, FindNext(end_cell1), end_cell2);
			}
			else
			{
				HighlightCells(dc, FindNext(end_cell2), end_cell1);
			}

		}

	}
	else if (end_cell1 == end_cell2)
	{

		wxHtmlCell *cell;

		int count1 = 0, count2 = 0;

		cell = start_cell1;
		while (cell != end_cell1)
		{
			count1++;
			cell = FindNext(cell);
		}

		cell = start_cell2;
		while (cell != end_cell2)
		{
			count2++;
			cell = FindNext(cell);
		}

		wxASSERT(count1 != count2);

		wxHtmlCell *cell1, *cell2;

		if (count1 < count2)
		{
			cell1 = start_cell2;
			cell2 = start_cell1;
		}
		else
		{
			cell1 = start_cell1;
			cell2 = start_cell2;
		}

		cell = cell1;
		while (FindNext(cell) != cell2)
		{
			cell = FindNext(cell);
		}

		HighlightCells(dc, cell1, cell);

	}
	else
	{

		HighlightCells(dc, start_cell1, end_cell1);
		HighlightCells(dc, start_cell2, end_cell2);
		
	}

}

void LogControl::InitDCForHighlighting(wxDC &dc)
{
	dc.SetLogicalFunction(wxINVERT);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);
}

void LogControl::OnMouseEvent(wxMouseEvent& event)
{

	m_tmpMouseMoved = TRUE;

	if ( !m_Cell )
	{
		return;
	}

	if (event.ButtonIsDown(1))
	{
		wxPoint pos = ScreenToClient(wxGetMousePosition());
		wxSize size = GetSize();
		int view_x, view_y;
		GetViewStart(&view_x, &view_y);
		if (pos.y < 0)
		{
			Scroll(-1, view_y - 1);
		}
		else if (pos.y > size.y)
		{
			Scroll(-1, view_y + 1);
		}
		if (pos.x < 0)
		{
			Scroll(view_x - 1, -1);
		}
		else if (pos.x > size.x)
		{
			Scroll(view_x + 1, -1);
		}
	}

	wxPoint pos = GetVirtualMousePosition();

	static wxPoint down_pos(0,0);
	static bool down_pos_valid = false; 

	static int captured = false;

	if (event.ButtonDown(1) && !captured)
	{
		CaptureMouse();
		captured = true;
	}
	else if (event.ButtonUp(1) && captured)
	{
		ReleaseMouse();
		captured = false;
	}

	if (last_start_end_valid && (!event.ButtonIsDown(1) || event.ButtonDown(1) || event.ButtonDown(2) || event.ButtonDown(3)))
	{

		if (!event.ButtonDown(2) && !event.ButtonDown(3))
		{
			if (wxTheClipboard->Open())
			{
				wxString text = GetTextFromRange(last_start_cell, last_end_cell);
				wxTheClipboard->Clear();
				wxTheClipboard->SetData(new wxTextDataObject(text));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
			}
		}

		Refresh();

		last_start_end_valid = false;
		down_pos_valid = false;

	}

	if (event.ButtonDown(1))
	{
		down_pos = pos;
		down_pos_valid = true;
	}
	else if (event.ButtonUp(1) && down_pos_valid && pos == down_pos)
	{

		down_pos_valid = false;

		SetFocus();

		wxHtmlCell *cell = m_Cell->FindCellByPos(pos.x, pos.y);

		if ( cell )
		{
			OnCellClicked(cell, pos.x, pos.y, event);
		}

	}
	else if (down_pos_valid && event.Dragging() && event.ButtonIsDown(1))
	{

		if (down_pos == pos) return;

		wxHtmlCell *start_cell = FindCell(down_pos);
		wxHtmlCell *end_cell = FindCell(pos);

		if (start_cell && end_cell)
		{

			wxClientDC dc(this);
			PrepareDC(dc);
			InitDCForHighlighting(dc);

			FirstCellFirst(&start_cell, &end_cell);

			if (last_start_end_valid)
			{
				HighlightCellsDiff(dc, start_cell, end_cell, last_start_cell, last_end_cell);
			}
			else
			{
				HighlightCells(dc, start_cell, end_cell);
			}

			last_start_cell = start_cell;
			last_end_cell = end_cell;
			last_start_end_valid = true;

		}

	}

}

void LogControl::OnIdle(wxIdleEvent& event)
{

	if (m_cur_hand == NULL)
	{
		#ifdef __WXMSW__
			m_cur_hand = new wxCursor(wxT("hand"));
		#else
			m_cur_hand = new wxCursor(wxCURSOR_HAND);
		#endif
		m_cur_arrow = new wxCursor(wxCURSOR_ARROW);
	}

	if (m_tmpMouseMoved && (m_Cell != NULL))
	{

		wxPoint pos = GetVirtualMousePosition();

		wxHtmlCell *cell = m_Cell->FindCellByPos(pos.x, pos.y);

		if (last_start_end_valid)
		{
			SetCursor(*m_cur_arrow);
		}

		if ( cell != m_tmpLastCell )
		{
			wxHtmlLinkInfo *lnk = cell ? cell->GetLink(pos.x, pos.y) : NULL;

			if (lnk != m_tmpLastLink)
			{
				if (lnk == NULL || last_start_end_valid)
				{
					SetCursor(*m_cur_arrow);
				}
				else
				{
					SetCursor(*m_cur_hand);
				}
				m_tmpLastLink = lnk;
			}

			m_tmpLastCell = cell;
		}
		else // mouse moved but stayed in the same cell
		{
			if ( cell )
				OnCellMouseHover(cell, pos.x, pos.y);
		}

		m_tmpMouseMoved = FALSE;
	}

}

void LogControl::ScrollToBottom()
{
	int x, y;
	GetVirtualSize(&x, &y);
	Scroll(0, y);
	CalcScrolledPosition(0, m_Cell->GetHeight(), NULL, &m_iYSize);
}

void LogControl::Clear()
{
	m_red_line = NULL;
	SetPage(wxT(""));
	ScrollToBottom();
}

void LogControl::AddHtmlLine(const wxString &line, bool split_long_words, bool red_line)
{

	wxString source = wxT("<br><code>") + line + wxT("</code>");

	wxClientDC *dc = new wxClientDC(this);
	dc->SetMapMode(wxMM_TEXT);
	SetBackgroundColour(wxColour(0xFF, 0xFF, 0xFF));

	wxHtmlWinParser *p2 = new wxHtmlWinParser(this);
	p2->SetFS(m_FS);
	p2->SetDC(dc);
	p2->AddTagHandler(new SpanTagHandler());
	SetHtmlParserFonts(p2);
	wxHtmlContainerCell *c2 = (wxHtmlContainerCell*)p2->Parse(source);

	if (split_long_words)
	{
		wxHtmlCell *cell = c2->GetFirstCell();
		cell = ((wxHtmlContainerCell*)cell->GetNext())->GetFirstCell();
		wxASSERT(cell);
		wxHtmlCell *last = NULL;
		while (cell)
		{
			cell->DrawInvisible(*dc, 0, 0);
			wxString text = GetCellText(cell);
			if (text.Length() > 32 && last)
			{
				wxASSERT(last->GetNext() == cell);
				wxHtmlLinkInfo *link = cell->GetLink();
				wxHtmlCell *first = new wxHtmlWordCell(text.Left(8), *dc);
				first->SetParent(cell->GetParent());
				if (link) first->SetLink(*link);
				wxHtmlCell *last_new = first;
				for (size_t i = 8; i < text.Length(); i += 8)
				{
					wxHtmlCell *next_new = new wxHtmlWordCell(text.Mid(i, 8), *dc);
					next_new->SetParent(cell->GetParent());
					if (link) next_new->SetLink(*link);
					last_new->SetNext(next_new);
					last_new = next_new;
				}

				wxHtmlCell *next = cell->GetNext();
				
				last->SetNext(first);
				last_new->SetNext(next);
				
				cell->SetNext(NULL);
				delete cell;
				cell = first;

			}
			last = cell;
			cell = cell->GetNext();
		}
	}

	if (!m_red_line && red_line)
	{
		m_red_line = c2;
	}

	m_Cell->InsertCell(c2);

	delete p2;

	delete dc;

	CreateLayout();

	ScrollToBottom();

	Refresh();

}

wxString LogControl::ConvertModifiersIntoHtml(const wxString &text, bool strip_mode)
{
	ModifierParser parser;
	parser.strip_mode = strip_mode;
	return parser.Parse(text);
}

void LogControl::AddTextLine(const wxString &line, const wxColour &line_colour, TextModifierMode mode, bool convert_urls, bool split_long_words, bool red_line)
{

	wxString html = FormatTextAsHtml(line);

	switch (mode)
	{

		case tmmParse:
			html = ConvertModifiersIntoHtml(html, false);
			break;

		case tmmStrip:
			html = ConvertModifiersIntoHtml(html, true);
			break;

		case tmmIgnore:
			html = html;
			break;

		default:
			wxFAIL_MSG(wxT("Invalid TextModifierMode for AddTextLine"));
			break;

	}

	if (convert_urls)
	{
		html = ConvertUrlsToLinks(html);
	}

	html = wxT("<font color='") + ColourToString(line_colour) + wxT("'>") + html + wxT("</font>");

	AddHtmlLine(html, split_long_words, red_line);

}

wxString LogControl::FormatTextAsHtml(const wxString &text)
{

	wxString html = text;

	html.Replace(wxT("&"),wxT("&amp;"));
	html.Replace(wxT("<"),wxT("&lt;"));
	html.Replace(wxT(">"),wxT("&gt;"));

	html.Replace(wxT("\r\n"),wxT("<br>"));
	html.Replace(wxT("\r"),wxT("<br>"));
	html.Replace(wxT("\n"),wxT("<br>"));

	if (html[0] == wxT(' '))
	{
		html = wxT("&nbsp;") + html.Mid(1);
	}
	html.Replace(wxT("\t"), wxT("&nbsp;&nbsp;&nbsp; "));
	html.Replace(wxT("  "), wxT(" &nbsp;"));

	return html;

}

#include <wx/tokenzr.h>

bool LogControl::IsEmail(const wxString &token)
{
	wxString buff = token;
	long a = buff.Find(wxT('@'));
	if (a > -1)
	{
		buff = token.Mid(a);
		if (buff.Find(wxT('.')) > -1)
		{
			if ((buff.Find(wxT(':')) == -1) && (buff.Find(wxT('/')) == -1))
			{
				return true;
			}
		}
	}
	return false;
}

wxString LogControl::ConvertUrlsToLinks(const wxString &text)
{
	static const wxString delims = wxT("\t\r\n '\"<>()");
	wxStringTokenizer st(text, delims, wxTOKEN_RET_DELIMS);

	wxString output;

	while (st.HasMoreTokens())
	{

		wxString token = st.GetNextToken();

		if (token.Length() > 0)
		{

			char last_char = token.Last();
			bool last_is_delim = (delims.Find(last_char) > -1);
			if (last_is_delim)
			{
				token = token.Left(token.Length() - 1);
			}

			wxString url;

			wxString token_lower = token.Lower();

			if (token_lower.Left(5) == wxT("news:"))
			{
				url = token;
			}
			else if (token.Left(2) == wxT("\\\\") && token.Length() > 2)
			{
				url = token;
			}
			else if ((token_lower.Left(7) == wxT("mailto:")) || (token.Find(wxT("://")) > -1))
			{
				url = token;
			}
			else if ((token_lower.Left(3) == wxT("www")) && (token.Find(wxT('.')) > -1))
			{
				url = wxT("http://") + token;
			}
			else if ((token_lower.Left(4) == wxT("ftp.")) && (token.Mid(4).Find(wxT('.')) > 0))
			{
				url = wxT("ftp://") + token;
			}
			else if (IsEmail(token))
			{
				url = wxT("mailto:") + token;
			}
			else if ((token_lower.Find(wxT(".com")) > -1) || (token_lower.Find(wxT(".net")) > -1) || (token_lower.Find(wxT(".org")) > -1))
			{
				url = wxT("http://") + token;
			}

			if (url.Length() > 0)
			{
				token = wxT("<a href=\"") + url + wxT("\">") + token + wxT("</a>");
			}

			output += token;
			if (last_is_delim)
			{
				output += last_char;
			}

		}

	}

	if (st.GetString().Length() > 0)
	{
		output += st.GetString();
	}

	return output;


}

void LogControl::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	event.SetString(link.GetHref());
	ProcessEvent(event);
}

void LogControl::ResetRedLine()
{
	if (m_red_line)
	{
		m_red_line = NULL;
		Refresh();
	}
}
