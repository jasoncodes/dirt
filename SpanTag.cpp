#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: SpanTag.cpp,v 1.4 2003-02-13 13:16:50 jason Exp $)

#include "SpanTag.h"

#include <wx/tokenzr.h>

class SpanTagCell : public wxHtmlCell
{

public:

	SpanTagCell()
	{
		m_endcell = this;
	}

	virtual ~SpanTagCell()
	{
	}

	virtual void Draw(wxDC& dc, int x, int y, int WXUNUSED(view_y1), int WXUNUSED(view_y2))
	{
		Draw(dc, x, y);
	}

	virtual void DrawInvisible(wxDC& dc, int x, int y)
	{
		Draw(dc, x, y);
	}

	virtual void Draw(wxDC& dc, int x, int y)
	{

		wxBrush old_brush = dc.GetBrush();
		dc.SetBrush(wxBrush(m_colour, wxSOLID));

		wxPen old_pen = dc.GetPen();
		dc.SetPen(*wxTRANSPARENT_PEN);

		wxHtmlCell *cell = GetNext();

		while (cell != NULL && m_endcell != this)
		{

			if (cell->GetWidth() > 0 && cell->GetHeight() > 0)
			{

				wxASSERT_MSG(
					cell->GetParent() == GetParent(),
					"Children cells within <span> not supported");

				dc.DrawRectangle(
					x + cell->GetPosX(), y + cell->GetPosY(),
					cell->GetWidth(), cell->GetHeight());

			}

			if (cell == m_endcell)
			{
				break;
			}

			cell = cell->GetNext();

		}

		dc.SetPen(old_pen);
		dc.SetBrush(old_brush);

	}

	virtual void SetBackgroundColour(const wxColour &clr)
	{
		m_colour = clr;
	}

	virtual void SetEndCell(wxHtmlCell *end)
	{
		m_endcell = end;
	}

protected:
	wxColour m_colour;
	wxHtmlCell *m_endcell;

};



SpanTagHandler::SpanTagHandler()
{
}

SpanTagHandler::~SpanTagHandler()
{
}

wxString SpanTagHandler::GetSupportedTags()
{
	return "SPAN";
}

// the following is essentially wxTag::GetParamAsColour()
bool SpanTagHandler::ParseColour(const wxString& str, wxColour *clr)
{

	if (str.IsEmpty()) return FALSE;
	if (str.GetChar(0) == wxT('#'))
	{
		unsigned long tmp;
		if (wxSscanf(str, wxT("#%lX"), &tmp) != 1)
			return FALSE;
		*clr = wxColour((unsigned char)((tmp & 0xFF0000) >> 16),
						(unsigned char)((tmp & 0x00FF00) >> 8),
						(unsigned char)(tmp & 0x0000FF));
		return TRUE;
	}
	else
	{
		// Handle colours defined in HTML 4.0:
		#define HTML_COLOUR(name,r,g,b)                 \
			if (str.IsSameAs(wxT(name), FALSE))         \
				{ *clr = wxColour(r,g,b); return TRUE; }
		HTML_COLOUR("black",   0x00,0x00,0x00)
		HTML_COLOUR("silver",  0xC0,0xC0,0xC0)
		HTML_COLOUR("gray",    0x80,0x80,0x80)
		HTML_COLOUR("white",   0xFF,0xFF,0xFF)
		HTML_COLOUR("maroon",  0x80,0x00,0x00)
		HTML_COLOUR("red",     0xFF,0x00,0x00)
		HTML_COLOUR("purple",  0x80,0x00,0x80)
		HTML_COLOUR("fuchsia", 0xFF,0x00,0xFF)
		HTML_COLOUR("green",   0x00,0x80,0x00)
		HTML_COLOUR("lime",    0x00,0xFF,0x00)
		HTML_COLOUR("olive",   0x80,0x80,0x00)
		HTML_COLOUR("yellow",  0xFF,0xFF,0x00)
		HTML_COLOUR("navy",    0x00,0x00,0x80)
		HTML_COLOUR("blue",    0x00,0x00,0xFF)
		HTML_COLOUR("teal",    0x00,0x80,0x80)
		HTML_COLOUR("aqua",    0x00,0xFF,0xFF)
		#undef HTML_COLOUR
	}

	return FALSE;

}

wxString SpanTagHandler::ExtractStyle(const wxString &style_tag, const wxString &style)
{

	wxStringTokenizer pairs(style_tag, ";");

	while (pairs.HasMoreTokens())
	{

		wxString pair = pairs.GetNextToken();

		wxString name, value;

		int sep_index = pair.Find(':');
		if (sep_index < 0)
		{
			name = pair;
			value = "";
		}
		else
		{
			name = pair.Left(sep_index);
			value = pair.Mid(sep_index + 1);
		}
		name = name.Strip(wxString::both);
		value = value.Strip(wxString::both);

		if (name.CmpNoCase(style) == 0)
		{
			return value;
		}

	}

	return wxEmptyString;

}

bool SpanTagHandler::HandleTag(const wxHtmlTag& tag)
{

	wxString style_tag = tag.GetParam("STYLE");

	wxString bgcolor = ExtractStyle(style_tag, "BACKGROUND");

	wxHtmlContainerCell *c;
	c = m_WParser->GetContainer();

	SpanTagCell *cell = NULL;

	if (bgcolor.Length() > 0)
	{
		wxColour clr;
		if (ParseColour(bgcolor,&clr))
		{
			if (cell == NULL)
			{
				cell = new SpanTagCell();
				c->InsertCell(cell);
			}
			cell->SetBackgroundColour(clr);
		}
	}

	ParseInner(tag);

	if (cell != NULL)
	{
		wxHtmlCell *end = cell;
		while (end->GetNext())
		{
			end = end->GetNext();
		}
		cell->SetEndCell(end);
	}

	return true;

}
