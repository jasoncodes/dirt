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
RCS_ID($Id: TextTools.cpp,v 1.7 2004-12-15 19:32:20 jason Exp $)

#include "TextTools.h"
#include "Modifiers.h"
#include "util.h"

struct ModifierParserTag
{

	const wxString name;
	bool active;

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

wxString ColourRGBToString(int red, int green, int blue)
{
	return wxString::Format(
		wxT("#%02x%02x%02x"),
		red, green, blue);
}

wxString ColourIndexToString(int colour_number)
{
	long clr = colours[colour_number % colour_count];
	return ColourRGBToString(CLR_RED(clr), CLR_GREEN(clr), CLR_BLUE(clr));
}

const bool debug_tags = false;

class ModifierParser
{

public:
	ModifierParser()
		: m_tagFont(wxT("font")), m_tagSpan(wxT("span")), m_tagBold(wxT("b")), m_tagUnderline(wxT("u"))
	{
		strip_mode = false;
		ResetAllState();
	}

public:
	bool strip_mode;

protected:
	wxString m_result;
	int m_colour_pos;
	bool m_had_comma;
	bool m_last_was_comma;
	int m_colour_number[2];
	bool m_colour_number_valid[2];

	ModifierParserTagEntryStack m_tag_stack;

	ModifierParserTag m_tagFont;
	ModifierParserTag m_tagSpan;
	ModifierParserTag m_tagBold;
	ModifierParserTag m_tagUnderline;

	ModifierParserTagEntryArray m_reverse_tags;
	bool m_reverse_mode;

protected:
	void ResetAllState()
	{
		m_result.Empty();
		ResetColourState();
		m_tag_stack.Empty();
		m_tagFont.active = false;
		m_tagSpan.active = false;
		m_tagBold.active = false;
		m_tagUnderline.active = false;
		m_reverse_tags.Empty();
		m_reverse_mode = false;
	}

	void ResetColourState()
	{
		m_colour_pos = 0;
		m_had_comma = false;
		m_last_was_comma = false;
		m_colour_number[0] = 0;
		m_colour_number[1] = 0;
		m_colour_number_valid[0] = false;
		m_colour_number_valid[1] = false;
	}

	void end_tag_helper(ModifierParserTag &t)
	{
		if (!strip_mode)
		{
			if (debug_tags)
			{
				m_result << wxT("&lt;/") << t.name << wxT("&gt;");
			}
			else
			{
				m_result << wxT("</") << t.name << wxT(">");
			}
		}
	}

	void start_tag_helper(ModifierParserTag &t, const wxString &attribs)
	{
		if (!strip_mode)
		{
			if (debug_tags)
			{
				m_result << wxT("&lt;") << t.name << wxT(" ") << attribs << wxT("&gt;");
			}
			else
			{
				m_result << wxT("<") << t.name << wxT(" ") << attribs << wxT(">");
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
				ModifierParserTagEntry entry = m_tag_stack.Pop();
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
				m_tag_stack.Push(entry);
				start_tag_helper(entry.tag, entry.attribs);
			}
		}
		return last_attribs;
	}

	wxString start_tag(ModifierParserTag &t, const wxString attribs = wxEmptyString)
	{
		wxString last_attribs = end_tag(t);
		t.active = true;
		m_tag_stack.Push(ModifierParserTagEntry(t, attribs));
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
		while (!m_tag_stack.IsEmpty())
		{
			ModifierParserTagEntry entry = m_tag_stack.Pop();
			entry.tag.active = false;
			end_tag_helper(entry.tag);
		}
	}

	void start_colour_tag(ModifierParserTag &t, wxString attribs)
	{
		if (m_reverse_mode)
		{
			m_reverse_tags.Add(ModifierParserTagEntry(t, attribs));
		}
		else
		{
			start_tag(t, attribs);
		}
	}

	void end_colour_tag(ModifierParserTag &t)
	{
		if (m_reverse_mode)
		{
			m_reverse_tags.Add(ModifierParserTagEntry(t, wxT("")));
		}
		else
		{
			end_tag(t);
		}
	}

	void EndOfColourCode()
	{
		if (m_colour_pos == 1 && !m_colour_number_valid[0])
		{
			end_colour_tag(m_tagSpan);
			end_colour_tag(m_tagFont);
		}
		else if (m_colour_number_valid[0])
		{
			start_colour_tag(m_tagFont, wxT("color=\"") + ColourIndexToString(m_colour_number[0]) + wxT("\""));
			if (m_colour_number_valid[1])
			{
				start_colour_tag(m_tagSpan, wxT("style=\"background: ") + ColourIndexToString(m_colour_number[1]) + wxT("\""));
			}
		}
		m_colour_number_valid[0] = false;
		m_colour_number_valid[1] = false;
		m_colour_pos = 0;
	}

	void ReverseTag()
	{
		m_reverse_mode = !m_reverse_mode;
		if (m_reverse_mode)
		{

			wxString font_attribs = start_tag(m_tagFont, wxT("color=white"));
			wxString span_attribs = start_tag(m_tagSpan, wxT("style='background: black'"));

			m_reverse_tags.Empty();

			if (font_attribs.Length() > 0)
			{
				m_reverse_tags.Add(ModifierParserTagEntry(m_tagFont, font_attribs));
			}
			if (span_attribs.Length() > 0)
			{
				m_reverse_tags.Add(ModifierParserTagEntry(m_tagSpan, span_attribs));
			}

		}
		else
		{
			end_tag(m_tagSpan);
			end_tag(m_tagFont);

			for (size_t i = 0; i < m_reverse_tags.GetCount(); ++i)
			{
				ModifierParserTagEntry entry = m_reverse_tags.Item(i);
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
		m_result.Alloc(text.Length() * 3);
		m_reverse_tags.Alloc(10);

		for (size_t i = 0; i < text.Length(); ++i)
		{

			if (m_colour_pos == 0)
			{
				ResetColourState();
			}

			wxChar c = text[i];

			if ( m_colour_pos > 0 && // is inside a colour code and
				( (c == wxT(',') && m_had_comma) || // is either a 2nd comma
				  (c != wxT(',') && !wxIsdigit(c)) ) // or a non-valid character
				)
			{
				EndOfColourCode();
				m_had_comma = false;
				if (m_last_was_comma)
				{
					m_result << wxT(',');
				}
			}

			switch (c)
			{

				case BoldModifier:
					EndOfColourCode();
					toggle_tag(m_tagBold);
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
					toggle_tag(m_tagUnderline);
					break;

				case ColourModifier:
					ResetColourState();
					m_colour_pos = 1;
					break;

				case wxT('1'): case wxT('2'): case wxT('3'):
				case wxT('4'): case wxT('5'): case wxT('6'):
				case wxT('7'): case wxT('8'): case wxT('9'):
				case wxT('0'): // isdigit()
					if (m_colour_pos > 0)
					{
						m_colour_pos++;
						if (m_colour_pos > 3)
						{
							EndOfColourCode();
						}
						else
						{
							int x = m_had_comma ? 1 : 0;
							if (!m_colour_number_valid[x])
							{
								m_colour_number[x] = 0;
								m_colour_number_valid[x] = true;
							}
							m_colour_number[x] *= 10;
							m_colour_number[x] += (c - wxT('0'));
						}
					}
					if (m_colour_pos == 0)
					{
						m_result << c;
					}
					break;

				case wxT(','):
					if (m_colour_pos > 0)
					{
						if (m_had_comma)
						{
							EndOfColourCode();
							m_result << c << c;
						}
						else
						{
							m_colour_pos = 1;
							m_had_comma = true;
						}
					}
					else
					{
						m_result << c;
					}
					break;

				default:
					EndOfColourCode();
					m_result << c;
					break;

			}

			m_last_was_comma = (c == wxT(','));

		}

		EndOfColourCode();

		CleanupStack();

		return m_result;

	}

};

#include <wx/tokenzr.h>

bool IsEmail(const wxString &token)
{
	wxString buff = token;
	long a = buff.Find(wxT('@'));
	if (a > 0)
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

static bool RemoveURLPrefix(wxString &token, wxString &prefix_out, const wxString &prefix_in)
{
	if (LeftEq(token, prefix_in))
	{
		token = token.Mid(prefix_in.Length());
		prefix_out += prefix_in;
		return true;
	}
	return false;
}

static bool RemoveURLSuffix(wxString &token, wxString &suffix_out, const wxString &suffix_in)
{
	if (RightEq(token, suffix_in))
	{
		token = token.Left(token.Length() - suffix_in.Length());
		suffix_out = suffix_in + suffix_out;
		return true;
	}
	return false;
}

static bool RemoveURLPrefixAndSuffix(wxString &token, wxString &prefix_out, const wxString &prefix_in, wxString &suffix_out, const wxString &suffix_in)
{
	if (token.Length() > prefix_in.Length() + suffix_in.Length() && LeftEq(token, prefix_in) && RightEq(token, suffix_in))
	{
		RemoveURLPrefix(token, prefix_out, prefix_in);
		RemoveURLSuffix(token, suffix_out, suffix_in);
		return true;
	}
	return false;
}

wxString ConvertUrlsToLinks(const wxString &text)
{

	const wxString char1 = wxT('\x005');
	const wxString char2 = wxT('\x006');

	wxString delims = wxT("\t\r\n \"\x0a0");

	wxString tmp(text);

	tmp.Replace(wxT("&nbsp;"), wxT("\x0a0"));

	bool ltgt_fix = (tmp.Find(char1) == -1) && (tmp.Find(char2) == -1);

	if (ltgt_fix)
	{
		tmp.Replace(wxT("&lt;"), char1);
		tmp.Replace(wxT("&gt;"), char2);
		tmp.Replace(wxT("&gt;"), char2);
		delims += char1 + char2;
	}
	else
	{
		delims += wxT("<>");
	}

	wxStringTokenizer st(tmp, delims, wxTOKEN_RET_DELIMS);

	wxString output;
	bool append_nbsp = false;

	while (st.HasMoreTokens())
	{

		wxString token = st.GetNextToken();

		if (token.Right(1) == wxT('\x0a0'))
		{
			append_nbsp = true;
			token = token.Left(token.Length() - 1);
		}

		if (token.Length() > 0)
		{

			char last_char = token.Last();
			bool last_is_delim = (delims.Find(last_char) > -1);
			if (last_is_delim)
			{
				token = token.Left(token.Length() - 1);
			}

			wxString prefix, suffix;

			const wxString extra_chars_to_remove = wxT(".,?!");
			while (extra_chars_to_remove.Find(token.Right(1)) > -1)
			{
				suffix = token.Right(1) + suffix;
				token = token.Left(token.Length() - 1);
			}

			RemoveURLPrefixAndSuffix(token, prefix, wxT("("), suffix, wxT(")"));
			RemoveURLPrefix(token, prefix, wxT("'"));
			RemoveURLSuffix(token, suffix, wxT("'")) || RemoveURLSuffix(token, suffix, wxT("'s"));

			wxString url;

			wxString token_lower = token.Lower();

			if (token_lower.Left(5) == wxT("news:") && token_lower.Length() > 5)
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
			else if ((token_lower.Left(3) == wxT("www")) && (token.Length() > 4u) && (token.Find(wxT('.')) > -1) && (token.Find(wxT('.')) < (int)token.Length()-1))
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
			else if ((token_lower.Find(wxT(".com")) > 0) ||
			         (token_lower.Find(wxT(".net")) > 0) ||
			         (token_lower.Find(wxT(".org")) > 0) ||
			         (token_lower.Find(wxT(".gov")) > 0))
			{
				url = wxT("http://") + token;
			}

			if (url.Length() > 0)
			{
				token = wxT("<a href=\"") + 
					ConvertModifiersIntoHtml(url, true) +
					wxT("\" target=\"_blank\">") + token +
					wxT("</a>");
			}

			output += prefix + token + suffix;
			if (last_is_delim)
			{
				output += last_char;
			}

		}

		if (append_nbsp)
		{
			output += wxT("&nbsp;");
			append_nbsp = false;
		}

	}

	if (st.GetString().Length() > 0)
	{
		output += st.GetString();
	}

	if (ltgt_fix)
	{
		output.Replace(char1, wxT("&lt;"));
		output.Replace(char2, wxT("&gt;"));
	}
	tmp.Replace(wxT("\x0a0"), wxT("&nbsp;"));

	return output;


}

wxString FormatTextAsHtml(const wxString &text)
{

	wxString html = text;

	html.Replace(wxT("&"),wxT("&amp;"));
	html.Replace(wxT("<"),wxT("&lt;"));
	html.Replace(wxT(">"),wxT("&gt;"));

	html.Replace(wxT("\r\n"),wxT("<br>"));
	html.Replace(wxT("\r"),wxT("<br>"));
	html.Replace(wxT("\n"),wxT("<br>"));

	if (html[0u] == wxT(' '))
	{
		html = wxT("&nbsp;") + html.Mid(1);
	}
	html.Replace(wxT("\t"), wxT("&nbsp;&nbsp;&nbsp; "));

	wxString html2;
	html2.Alloc(html.Length()*2);

	bool last_was_space = false;
	bool last_was_modifier = false;
	for (size_t i = 0; i < html.Length(); ++i)
	{
		wxChar c = html[i];
		if (IsModifier(c))
		{
			last_was_modifier = true;
			last_was_space = true;
			html2 += c;
		}
		else if (wxIsspace(c))
		{
			last_was_modifier = false;
			if (last_was_space)
			{
				html2 += wxT("&nbsp;");
			}
			else
			{
				html2 += c;
			}
			last_was_space = !last_was_space;
		}
		else
		{
			last_was_modifier &= ((c == wxT(',')) || wxIsdigit(c));
			last_was_space = last_was_modifier;
			html2 += c;
		}
	}

	return html2;

}

wxString ConvertModifiersIntoHtml(const wxString &text, bool strip_mode)
{
	ModifierParser parser;
	parser.strip_mode = strip_mode;
	return parser.Parse(text);
}

