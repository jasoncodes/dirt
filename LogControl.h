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


// To get link click events from LogControl listen for the
// wxEVT_COMMAND_BUTTON_CLICKED event and use event.GetString()
// to get the URI of the link that was clicked.

#ifndef LogControl_H_
#define LogControl_H_

#include "wx/html/htmlcell.h"
#include <wx/fdrepdlg.h>
#include <wx/html/winpars.h>

#include <wx/dynarray.h>
#ifndef WX_DEFINE_ARRAY_PTR
	#define WX_DEFINE_ARRAY_PTR WX_DEFINE_ARRAY
#endif
WX_DEFINE_ARRAY_PTR(wxHtmlCell*, HtmlCellPointerArray);

class wxHtmlWindow;

enum TextModifierMode
{
	tmmParse,
	tmmStrip,
	tmmIgnore
};

class LogControl : public wxScrolledWindow
{

public:
	LogControl(wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		bool align_bottom = true);
	virtual ~LogControl();

	void ScrollToBottom();
	void Clear();
	void ResetRedLine();
	void ShowFindDialog(bool show);
	void AddSeparator();
	bool SetFont(const wxFont &font);

	void AddHtmlLine(const wxString &line, bool split_long_words = false, bool red_line = false);
	void AddTextLine(const wxString &line, const wxColour &line_colour = *wxBLACK, TextModifierMode mode = tmmParse, bool convert_urls = true, bool split_long_words = true, bool red_line = false);

	static wxString GetCellText(wxHtmlCell *cell);
	static void ClearRect(wxDC& dc, const wxRect &rect);
	static void SetHtmlParserFonts(wxHtmlWinParser *parser);
	static wxFont GetDefaultFixedWidthFont();

protected:
	void OnSize(wxSizeEvent& event);
	void OnErase(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnFindDialog(wxFindDialogEvent &event);
	void OnScroll(wxScrollWinEvent &event);
	
	virtual void OnLinkClicked(const wxHtmlLinkInfo& link);

	void CalculateOffset();
	void ClearBlankArea(wxDC& dc);
	void EnsureVisible(wxHtmlCell *cell);
	void CreateLayout();

	wxPoint GetVirtualMousePosition();
	wxRect GetCellRect(wxHtmlCell *cell);
	wxHtmlCell *GetCellAt(const wxPoint &pos);
	wxHtmlCell *FindCell(const wxPoint &pos);

	void HighlightCells(wxDC &dc, wxHtmlCell *start_cell, wxHtmlCell *end_cell);
	void HighlightCellsDiff(wxDC &dc, wxHtmlCell *start_cell1, wxHtmlCell *end_cell1, wxHtmlCell *start_cell2, wxHtmlCell *end_cell2);
	wxString GetTextFromRange(wxHtmlCell *start_cell, wxHtmlCell *end_cell, bool strip_prefix);

	static void InitDCForHighlighting(wxDC &dc);

	virtual void OnDraw(wxDC& dc);

protected:
	wxHtmlContainerCell *m_Cell;
	wxString m_buff;

	int m_iYOffset;
	bool m_Resizing;
	int m_iYSize;
	bool m_align_bottom;

	static bool s_bInitDone;

	wxHtmlLinkInfo *m_tmpLastLink;
	wxHtmlCell *m_tmpLastCell;
	bool m_tmpMouseMoved;

	wxCursor *m_cur_hand;
	wxCursor *m_cur_arrow;

	wxHtmlCell *m_last_start_cell;
	wxHtmlCell *m_last_end_cell;
	bool m_last_start_end_valid;

	wxHtmlCell *m_red_line;

	HtmlCellPointerArray m_separators;

	wxFindReplaceDialog *m_find_dlg;
	wxFindReplaceData m_find_data;
	wxHtmlCell *m_find_pos1;
	wxHtmlCell *m_find_pos2;
	bool m_find_show_sel;
	bool m_first_line;

	wxHtmlWindow *m_dummy_html_wnd;

	int m_wheel_rotation;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(LogControl)

};

#endif
