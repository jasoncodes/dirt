// To get link click events from LogControl listen for the
// wxEVT_COMMAND_BUTTON_CLICKED event and use event.GetString()
// to get the URI of the link that was clicked.

#ifndef LogControl_H_
#define LogControl_H_

#include "wx/html/htmlwin.h"
#include <wx/fdrepdlg.h>

enum TextModifierMode
{
	tmmParse,
	tmmStrip,
	tmmIgnore
};

class LogControl : public wxHtmlWindow
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

	void AddHtmlLine(const wxString &line, bool split_long_words = false, bool red_line = false);
	void AddTextLine(const wxString &line, const wxColour &line_colour = *wxBLACK, TextModifierMode mode = tmmParse, bool convert_urls = true, bool split_long_words = true, bool red_line = false);
	static wxString FormatTextAsHtml(const wxString &text);
	static wxString ConvertUrlsToLinks(const wxString &text);
	static wxString ConvertModifiersIntoHtml(const wxString &text, bool strip_mode);

	static wxString GetCellText(wxHtmlCell *cell);
	static bool IsEmail(const wxString &token);
	static void ClearRect(wxDC& dc, wxRect &rect);

protected:
	void OnSize(wxSizeEvent& event);
	void OnErase(wxEraseEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnFindDialog(wxFindDialogEvent &event);
	
	virtual void OnLinkClicked(const wxHtmlLinkInfo& link);

	void CalculateOffset();
	void ClearBlankArea(wxDC& dc);
	void EnsureVisible(wxHtmlCell *cell);

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
	int m_iYOffset;
	bool m_Resizing;
	int m_iYSize;
	bool m_align_bottom;

	static bool s_bInitDone;

	// stuff that is for some reason private not protected in wxHtmlWindow
	wxHtmlLinkInfo *m_tmpLastLink;
	wxHtmlCell *m_tmpLastCell;
	bool m_tmpMouseMoved;
	// these have been changed from static for easier maintenance
	wxCursor *m_cur_hand;
	wxCursor *m_cur_arrow;

	wxHtmlCell *last_start_cell;
	wxHtmlCell *last_end_cell;
	bool last_start_end_valid;

	wxHtmlCell *m_red_line;

	wxFindReplaceDialog *m_find_dlg;
	wxFindReplaceData m_find_data;
	wxHtmlCell *m_find_pos1;
	wxHtmlCell *m_find_pos2;
	bool m_find_show_sel;
	bool m_first_line;

private:
	DECLARE_EVENT_TABLE()

};

#endif
