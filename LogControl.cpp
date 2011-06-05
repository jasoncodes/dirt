/*
    Copyright 2002, 2003 General Software Laboratories


    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LogControl.cpp,v 1.77 2004-11-25 09:46:17 jason Exp $)

#include <wx/image.h>
#include <wx/sysopt.h>
#include <wx/html/winpars.h>
#include <wx/tokenzr.h>
#include <ctype.h>
#include <wx/clipbrd.h>
#include "utilgui.h"
#include "Dirt.h"
#include "LogControl.h"
#include "SpanTag.h"
#include "Modifiers.h"
#include "TextTools.h"
#include <wx/fdrepdlg.h>
#include <wx/textbuf.h>
#include <wx/html/htmlwin.h>

DECLARE_APP(DirtApp)

bool LogControl::s_bInitDone = false;

BEGIN_EVENT_TABLE(LogControl, wxScrolledWindow)
	EVT_SIZE(LogControl::OnSize)
	EVT_ERASE_BACKGROUND(LogControl::OnErase)
	EVT_PAINT(LogControl::OnPaint)
	EVT_IDLE(LogControl::OnIdle)
	EVT_MOUSE_EVENTS(LogControl::OnMouseEvent)
	EVT_FIND(wxID_ANY, LogControl::OnFindDialog)
	EVT_FIND_NEXT(wxID_ANY, LogControl::OnFindDialog)
	EVT_FIND_REPLACE(wxID_ANY, LogControl::OnFindDialog)
	EVT_FIND_REPLACE_ALL(wxID_ANY, LogControl::OnFindDialog)
	EVT_FIND_CLOSE(wxID_ANY, LogControl::OnFindDialog)
	EVT_SCROLLWIN(LogControl::OnScroll)
END_EVENT_TABLE()

void LogControl::SetHtmlParserFonts(wxHtmlWinParser *parser)
{

	#if wxCHECK_VERSION(2,5,0)
		static int *default_sizes = NULL;
	#else
		// The following is copied from wxHtmlWinParser::wxHtmlWinParser()
		// as it is not exposed anywhere and NULL isn't allowed in 2.4.0
		#ifdef __WXMSW__
				static int default_sizes[7] = {7, 8, 10, 12, 16, 22, 30};
		#elif defined(__WXMAC__)
				static int default_sizes[7] = {9, 12, 14, 18, 24, 30, 36};
		#else
				static int default_sizes[7] = {10, 12, 14, 16, 19, 24, 32};
		#endif
	#endif

	parser->SetFonts(wxEmptyString, wxEmptyString, default_sizes);

}

wxFont LogControl::GetDefaultFixedWidthFont()
{
	wxFrame *frm = new wxFrame(NULL, wxID_ANY, wxEmptyString);
	wxHtmlWindow *html = new wxHtmlWindow(frm);
	wxHtmlWinParser *parser = html->GetParser();
	SetHtmlParserFonts(parser);
	wxClientDC *pDC = new wxClientDC(html);
	parser->SetFontFixed(TRUE);
	parser->SetDC(pDC);
	wxFont font = *(parser->CreateCurrentFont());
	delete pDC;
	frm->Destroy();
	return font;
}

LogControl::LogControl(wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	bool align_bottom)
	: wxScrolledWindow(parent, id, pos, size, wxScrolledWindowStyle|wxSUNKEN_BORDER),
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

	m_find_dlg = NULL;
	m_find_show_sel = false;

	m_tmpMouseMoved = FALSE;
	m_cur_hand = NULL;
	m_cur_arrow = NULL;
	m_tmpLastLink = NULL;
	m_tmpLastCell = NULL;

	m_last_start_cell = NULL;
	m_last_end_cell = NULL;
	m_last_start_end_valid = false;

	m_Resizing = false;
	m_iYOffset = 0;

	m_wheel_rotation = 0;

	FixBorder(this);

	m_Cell = NULL;

	m_dummy_html_wnd = new wxHtmlWindow; // just so wxHtmlWinParser stays happy

	SetFont(GetDefaultFixedWidthFont());

	Clear();

}

LogControl::~LogControl()
{
	delete m_find_dlg;
	delete m_cur_hand;
	delete m_cur_arrow;
	delete m_Cell;
	delete m_dummy_html_wnd;
}

void LogControl::OnSize(wxSizeEvent& event)
{

	if (GetSize().x > 0 && GetSize().y > 0)
	{

		#ifdef __WXMSW__
			m_Resizing = true;
		#endif

		wxScrolledWindow::OnSize(event);
		CreateLayout();
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

void LogControl::ClearRect(wxDC& dc, const wxRect &rect)
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

		rtClearArea.height = GetClientSize().y + wxHTML_SCROLL_STEP;
		rtClearArea.y = GetClientSize().y - bottom_gap_size;

		ClearRect(dc, rtClearArea);

	}

}

void LogControl::OnPaint(wxPaintEvent &event)
{
	wxScrolledWindow::OnPaint(event);
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

		m_Cell->SetBackgroundColour(*wxWHITE);

		CalculateOffset();

		wxRect client_area = GetClientRect();

		/*
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
		}
		*/
		wxBitmap bmpBack(client_area.width, client_area.height);
		wxMemoryDC dcBack;
		dcBack.SelectObject(bmpBack);
		ClearRect(dcBack, wxRect(wxPoint(0,0), GetClientSize()));
		PrepareDC(dcBack);

		ClearBlankArea(dcBack);

		int x, y;
		wxScrolledWindow::GetViewStart(&x, &y);
		x *= wxHTML_SCROLL_STEP;
		y *= wxHTML_SCROLL_STEP;

		wxRect rect = GetUpdateRegion().GetBox();

		dcBack.SetMapMode(wxMM_TEXT);
		dcBack.SetBackgroundMode(wxTRANSPARENT);

#if wxCHECK_VERSION(2,5,0)
		wxHtmlRenderingInfo render_info;
#endif

		m_Cell->Draw(
			dcBack,
			0, m_iYOffset,
			rect.GetTop() + y - 32,
			rect.GetBottom() + y + 32
#if wxCHECK_VERSION(2,5,0)
			, render_info
#endif
			);

		if (m_separators.GetCount())
		{
			wxPen old_pen = dcBack.GetPen();
			dcBack.SetPen(*wxLIGHT_GREY_PEN);
			for (size_t i = 0; i < m_separators.GetCount(); ++i)
			{
				wxHtmlCell *cell = m_separators.Item(i);
				if (cell)
				{
					cell = cell->GetNext();
					if (cell)
					{
						wxRect red_rect = GetCellRect(cell);
						dcBack.DrawLine(0, red_rect.y, GetClientSize().x, red_rect.y);
					}
				}
			}
			dcBack.SetPen(old_pen);
		}

		if (m_red_line)
		{
			wxPen old_pen = dcBack.GetPen();
			dcBack.SetPen(*wxRED_PEN);
			wxRect red_rect = GetCellRect(m_red_line);
			dcBack.DrawLine(0, red_rect.y, GetClientSize().x, red_rect.y);
			dcBack.SetPen(old_pen);
		}

		if (m_find_show_sel)
		{
			InitDCForHighlighting(dcBack);
			HighlightCells(dcBack, m_find_pos1, m_find_pos2);
		}

		if (m_last_start_end_valid)
		{
			InitDCForHighlighting(dcBack);
			HighlightCells(dcBack, m_last_start_cell, m_last_end_cell);
		}

		dcFront.Blit(x, y, client_area.width, client_area.height, &dcBack, x, y);
		dcBack.SelectObject(wxNullBitmap);

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

class LogControlWordCell : public wxHtmlCell
{
public:
	LogControlWordCell(const wxString& word, wxDC& dc);
	virtual void Draw(wxDC& dc, int x, int y, int view_y1, int view_y2);
	wxString m_Word;
	#if wxCHECK_VERSION(2,5,0)
		bool m_allowLinebreak;
	#endif
	DECLARE_NO_COPY_CLASS(LogControlWordCell)
};

wxString LogControl::GetCellText(wxHtmlCell *cell)
{

	if (cell->IsTerminalCell() && cell->GetWidth() > 0 && cell->GetHeight() > 0)
	{
		wxASSERT(sizeof (class LogControlWordCell) == sizeof (class wxHtmlWordCell));
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

	if (!cell) return NULL;

	if (!cell->IsTerminalCell())
	{
		wxHtmlContainerCell *container = (wxHtmlContainerCell*)cell;
#if wxCHECK_VERSION(2,5,0)
		wxHtmlCell *child = container->GetFirstChild();
#else
		wxHtmlCell *child = container->GetFirstCell();
#endif
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

wxString LogControl::GetTextFromRange(wxHtmlCell *start_cell, wxHtmlCell *end_cell, bool strip_prefix)
{

	wxString buffer;
	wxHtmlCell *cell = start_cell;
	bool last_was_new_line = false;

	wxString line_sep;
	switch (wxTextBuffer::typeDefault)
	{

		case wxTextFileType_Unix:
		case wxTextFileType_None:
		default:
			line_sep = wxT("\n");
			break;

		case wxTextFileType_Dos:
		case wxTextFileType_Os2:
			line_sep = wxT("\r\n");
			break;

		case wxTextFileType_Mac:
			line_sep = wxT("\r");
			break;

	}

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
				buffer += line_sep;
				last_was_new_line = true;
			}
		}
		if (cell == end_cell) break;
		cell = FindNext(cell);
	}
	if (strip_prefix)
	{
		wxArrayString lines = SplitString(buffer, line_sep);
		for (size_t i = 0; i < lines.GetCount(); ++i)
		{
			wxString line = lines[i];
			line.Trim(true);
			line.Trim(false);
			if (line.Length() > 7 &&
				line[0u] == wxT('[') && wxIsdigit(line[1u]) && wxIsdigit(line[2u]) &&
				line[3u] == wxT(':') && wxIsdigit(line[4u]) && wxIsdigit(line[5u]) &&
				line[6u] == wxT(']') && line[7u] == wxT(' '))
			{
				line = line.Mid(8);
			}
			if (line.Length() > 2 && line[0u] == wxT('<'))
			{
				int j = line.Find(wxT('>'));
				if (j > 0)
				{
					if (line.Length() > (size_t)j || line[(size_t)j+1] == wxT(' '))
					{
						line = line.Mid(j+2);
					}
					else
					{
						line = line.Mid(j+1);
					}
				}
			}
			lines[i] = line;
		}
		buffer = JoinArray(lines, line_sep);
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

	if (event.ButtonIsDown(1) &&
		event.GetEventType() != wxEVT_ENTER_WINDOW &&
		event.GetEventType() != wxEVT_LEAVE_WINDOW)
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

	m_wheel_rotation += event.GetWheelRotation();

	int wheel_delta = event.GetWheelDelta();
	if (wheel_delta == 0)
	{
		wheel_delta = 120;
	}

	int lines_per_action = event.GetLinesPerAction();
	if (lines_per_action == 0)
	{
		lines_per_action = 3;
	}

	while (m_wheel_rotation >= wheel_delta)
	{
		int view_x, view_y;
		GetViewStart(&view_x, &view_y);
		Scroll(-1, wxMax(0, view_y - lines_per_action));
		m_wheel_rotation -= wheel_delta;
	}
	while (m_wheel_rotation <= -wheel_delta)
	{
		int view_x, view_y;
		GetViewStart(&view_x, &view_y);
		Scroll(-1, view_y + lines_per_action);
		m_wheel_rotation += wheel_delta;
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

	if (m_last_start_end_valid &&
		(!event.ButtonIsDown(1) || event.ButtonDown(1) ||
		event.ButtonDown(2) || event.ButtonDown(3)))
	{

		if (!event.ButtonDown(2) && !event.ButtonDown(3))
		{
			if (wxTheClipboard->Open())
			{
				wxString text = GetTextFromRange(
					m_last_start_cell, m_last_end_cell, wxGetApp().IsControlDown());
				wxTheClipboard->Clear();
				wxTheClipboard->SetData(new wxTextDataObject(text));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
			}
		}

		m_tmpLastCell = NULL;
		m_tmpLastLink = NULL;

		Refresh();

		m_last_start_end_valid = false;
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
			wxHtmlLinkInfo *lnk = cell->GetLink(pos.x, pos.y);
			if (lnk)
			{
				wxHtmlLinkInfo lnk2(*lnk);
				lnk2.SetEvent(&event);
				lnk2.SetHtmlCell(cell);
				OnLinkClicked(lnk2);
			}
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

			if (m_last_start_end_valid)
			{
				HighlightCellsDiff(dc, start_cell, end_cell, m_last_start_cell, m_last_end_cell);
			}
			else
			{
				HighlightCells(dc, start_cell, end_cell);
			}

			m_last_start_cell = start_cell;
			m_last_end_cell = end_cell;
			m_last_start_end_valid = true;

		}

	}

}

void LogControl::ScrollLog(int lines)
{
	int view_x, view_y;
	GetViewStart(&view_x, &view_y);
	Scroll(-1, view_y + lines);
}

void LogControl::OnIdle(wxIdleEvent &WXUNUSED(event))
{

	if (m_cur_hand == NULL)
	{
		#ifdef __WXMSW__
			m_cur_hand = new wxCursor(wxT("hand"));
			if (!m_cur_hand->Ok())
			{
				delete m_cur_hand;
				m_cur_hand = new wxCursor(wxCURSOR_HAND);
			}
		#else
			m_cur_hand = new wxCursor(wxCURSOR_HAND);
		#endif
		m_cur_arrow = new wxCursor(wxCURSOR_ARROW);
	}

	if (m_tmpMouseMoved && (m_Cell != NULL))
	{

		wxPoint pos = GetVirtualMousePosition();

		wxHtmlCell *cell = m_Cell->FindCellByPos(pos.x, pos.y);

		if (m_last_start_end_valid)
		{
			SetCursor(*m_cur_arrow);
		}

		if ( cell != m_tmpLastCell )
		{
			wxHtmlLinkInfo *lnk = cell ? cell->GetLink(pos.x, pos.y) : NULL;

			if (lnk != m_tmpLastLink)
			{
				if (lnk == NULL || m_last_start_end_valid)
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

		m_tmpMouseMoved = FALSE;
	}

}

void LogControl::ScrollToBottom()
{
	int x, y;
	GetVirtualSize(&x, &y);
	Scroll(0, y);
	CalcScrolledPosition(0, m_Cell->GetHeight(), NULL, &m_iYSize);
	CalculateOffset();
}

void LogControl::Clear()
{
	m_buff.Clear();
	m_red_line = NULL;
	m_separators.Empty();
	m_last_start_end_valid = false;
	m_find_pos1 = m_Cell;
	m_find_pos2 = m_Cell;
	m_find_show_sel = false;
	m_first_line = true;
	if (m_find_dlg)
	{
		m_find_dlg->Close(true);
		delete m_find_dlg;
		m_find_dlg = NULL;
	}
	delete m_Cell;
	wxHtmlWinParser parser(m_dummy_html_wnd);
	wxClientDC *dc = new wxClientDC(this);
	dc->SetMapMode(wxMM_TEXT);
	SetBackgroundColour(wxColour(0xFF, 0xFF, 0xFF));
	parser.SetDC(dc);
	m_Cell = (wxHtmlContainerCell*)parser.Parse(wxT(""));
	delete dc;
	CreateLayout();
	ScrollToBottom();
	Refresh();
}

class LogControlParser : public wxHtmlWinParser
{

public:
	LogControlParser(wxHtmlWindow *wnd, const wxFont &font)
		: wxHtmlWinParser(wnd), m_font(font)
	{
	}

	virtual wxFont* CreateCurrentFont()
	{
		m_font.SetWeight(GetFontBold()?wxBOLD:wxNORMAL);
		m_font.SetUnderlined(GetFontUnderlined()?true:false);
		GetDC()->SetFont(m_font);
		return &m_font;
	}

protected:
	wxFont m_font;

};

void LogControl::AddHtmlLine(const wxString &line, bool split_long_words, bool red_line)
{

	wxString source;

	if (!m_first_line)
	{
		source = wxT("<br>") + line;
	}
	else
	{
		source = line;
		m_first_line = false;
	}

	#if wxCHECK_VERSION(2,5,0)
		if ((double)m_buff.capacity() < m_buff.size() * 1.25)
		{
			m_buff.reserve(m_buff.size() * 2);
		}
	#endif

	m_buff << source;

	wxClientDC *dc = new wxClientDC(this);
	dc->SetMapMode(wxMM_TEXT);
	dc->SetFont(GetFont());
	SetBackgroundColour(wxColour(0xFF, 0xFF, 0xFF));

	wxHtmlWinParser *p2 = new LogControlParser(m_dummy_html_wnd, GetFont());
	p2->SetDC(dc);
	p2->AddTagHandler(new SpanTagHandler());
	wxHtmlContainerCell *c2 = (wxHtmlContainerCell*)p2->Parse(source);

	if (split_long_words)
	{
#if wxCHECK_VERSION(2,5,0)
		wxHtmlCell *cell = c2->GetFirstChild();
		cell = ((wxHtmlContainerCell*)cell->GetNext())->GetFirstChild();
#else
		wxHtmlCell *cell = c2->GetFirstCell();
		cell = ((wxHtmlContainerCell*)cell->GetNext())->GetFirstCell();
#endif
		if (!cell)
		{
#if wxCHECK_VERSION(2,5,0)
			cell = ((wxHtmlContainerCell*)c2->GetFirstChild())->GetFirstChild();
#else
			cell = ((wxHtmlContainerCell*)c2->GetFirstCell())->GetFirstCell();
#endif
		}
		wxHtmlCell *last = NULL;
		while (cell)
		{
#if wxCHECK_VERSION(2,5,0)
			wxHtmlRenderingInfo render_info;
			cell->DrawInvisible(*dc, 0, 0, render_info);
#else
			cell->DrawInvisible(*dc, 0, 0);
#endif
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

void LogControl::CreateLayout()
{
    int ClientWidth, ClientHeight;

    if (!m_Cell) return;

    GetClientSize(&ClientWidth, &ClientHeight);
    m_Cell->Layout(ClientWidth);
    if (ClientHeight < m_Cell->GetHeight() + GetCharHeight())
    {
        SetScrollbars(
              wxHTML_SCROLL_STEP, wxHTML_SCROLL_STEP,
              m_Cell->GetWidth() / wxHTML_SCROLL_STEP,
              (m_Cell->GetHeight() + GetCharHeight()) / wxHTML_SCROLL_STEP
              /*cheat: top-level frag is always container*/);
    }
    else /* we fit into window, no need for scrollbars */
    {
        SetScrollbars(wxHTML_SCROLL_STEP, 1, m_Cell->GetWidth() / wxHTML_SCROLL_STEP, 0); // disable...
        GetClientSize(&ClientWidth, &ClientHeight);
        m_Cell->Layout(ClientWidth); // ...and relayout
    }

}

class LogControlContainerCell : public wxHtmlContainerCell
{

public:
	LogControlContainerCell(wxHtmlContainerCell *parent)
		: wxHtmlContainerCell(parent)
	{
	}

	inline wxHtmlCell* GetLastCell()
	{
		return m_LastCell;
	}

	DECLARE_NO_COPY_CLASS(LogControlContainerCell)

};

void LogControl::AddSeparator()
{
	wxHtmlCell *last = ((LogControlContainerCell*)m_Cell)->GetLastCell();
	m_separators.Add(last);
	Refresh();
}

bool LogControl::SetFont(const wxFont &font)
{
	if (font != wxScrolledWindow::GetFont())
	{
		bool ok = wxScrolledWindow::SetFont(font);
		wxString buff = m_buff;
		Clear();
		AddHtmlLine(buff, true, false);
		return ok;
	}
	return true;
}

void LogControl::AddTextLine(const wxString &line, const wxColour &line_colour, TextModifierMode mode, bool convert_urls, bool split_long_words, bool red_line)
{

	wxString html = FormatTextAsHtml(line);

	if (convert_urls)
	{
		html = ConvertUrlsToLinks(html);
	}

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

	html = wxT("<font color='") + ColourRGBToString(line_colour) + wxT("'>") + html + wxT("</font>");

	AddHtmlLine(html, split_long_words, red_line);

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

void LogControl::ShowFindDialog(bool show)
{

	if (show)
	{

		if (!m_find_dlg)
		{
			m_find_data = wxFindReplaceData(wxFR_DOWN);
			m_find_dlg = new wxFindReplaceDialog(this, &m_find_data, wxT("Find"), wxFR_NOUPDOWN|wxFR_NOWHOLEWORD);
			m_find_pos1 = m_Cell;
			m_find_pos2 = m_Cell;
			m_find_show_sel = false;
			Refresh();
		}

		m_find_dlg->Show(false);
		m_find_dlg->Show(true);

	}
	else
	{

		if (m_find_dlg)
		{
			delete m_find_dlg;
			m_find_dlg = NULL;
			m_find_pos1 = m_Cell;
			m_find_pos2 = m_Cell;
			m_find_show_sel = false;
			Refresh();
		}

	}

}

void LogControl::OnFindDialog(wxFindDialogEvent &event)
{

	wxEventType type = event.GetEventType();

	if (type == wxEVT_COMMAND_FIND || type == wxEVT_COMMAND_FIND_NEXT)
	{

		int flags = event.GetFlags();

		bool direction_down = ((flags & wxFR_DOWN) != 0);
		bool whole_word = ((flags & wxFR_WHOLEWORD) != 0);
		bool case_sensitive = ((flags & wxFR_MATCHCASE) != 0);
		if (!direction_down || whole_word)
		{
			wxFAIL;
		}

		if (type == wxEVT_COMMAND_FIND)
		{
			m_find_pos1 = m_Cell;
			m_find_pos2 = m_Cell;
		}
		else
		{
			m_find_pos2 = FindNext(m_find_pos2);
			m_find_pos1 = m_find_pos2;
		}

		wxString to_find = event.GetFindString();
		if (to_find.Length() < 1) return;
		if (!case_sensitive)
		{
			to_find.MakeLower();
		}
		while (m_find_pos2)
		{
			wxString text = GetTextFromRange(m_find_pos1, m_find_pos2, false);
			if (!case_sensitive)
			{
				text.MakeLower();
			}
			int index = text.Find(to_find);
			if (index > -1)
			{
				while (m_find_pos1 != m_find_pos2)
				{
					wxHtmlCell *next = FindNext(m_find_pos1);
					text = GetTextFromRange(next, m_find_pos2, false);
					if (!case_sensitive)
					{
						text.MakeLower();
					}
					if (text.Find(to_find) == -1)
					{
						break;
					}
					m_find_pos1 = next;
				}
				break;
			}
			while (text.Length() > to_find.Length() && m_find_pos1 != m_find_pos2)
			{
				wxHtmlCell *next = FindNext(m_find_pos1);
				text = GetTextFromRange(next, m_find_pos2, false);
				if (text.Length() < to_find.Length())
				{
					break;
				}
				m_find_pos1 = next;
			}
			m_find_pos2 = FindNext(m_find_pos2);
		}
		if (m_find_pos2)
		{
			m_find_show_sel = true;
			EnsureVisible(m_find_pos1);
			Refresh();
		}
		else
		{
			m_find_pos1 = m_Cell;
			m_find_pos2 = m_Cell;
			m_find_show_sel = false;
			Refresh();
			wxMessageBox(
				wxString::Format(wxT("No more matches for \"%s\" found."), event.GetFindString().c_str()),
				wxT("Find"), wxOK | wxICON_INFORMATION, event.GetDialog());
		}

	}
	else if (type == wxEVT_COMMAND_FIND_CLOSE)
	{
		m_find_dlg = NULL;
		m_find_show_sel = false;
		event.GetDialog()->Destroy();
		Refresh();
	}
	else
	{
		wxFAIL_MSG(wxT("Unexpected event type in LogControl::OnFindDialog"));
	}

}

void LogControl::EnsureVisible(wxHtmlCell *cell)
{
	int y = 0;
	while (cell)
	{
		y += cell->GetPosY();
		cell = cell->GetParent();
	}
	y += m_iYOffset;
	y /= wxHTML_SCROLL_STEP;
	int current_min_y;
	GetViewStart(NULL, &current_min_y);
	int current_max_y = current_min_y + (GetClientSize().y / wxHTML_SCROLL_STEP);
	wxASSERT(current_min_y<current_max_y);
	if (y < current_min_y)
	{
		Scroll(-1, y);
	}
	else if (y+1 > current_max_y)
	{
		Scroll(-1, y - (current_max_y - current_min_y) + 1);
	}
}

void LogControl::OnScroll(wxScrollWinEvent &event)
{
	m_tmpMouseMoved = true;
	event.Skip();
}
