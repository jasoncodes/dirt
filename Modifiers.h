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


#ifndef Modifiers_H_
#define Modifiers_H_

struct ModifierMapping
{
	bool ctrl, alt, shift;
	int key_code;
	wxChar modifier_char;
	wxChar *alternate_modifier;
	bool is_colour_popup;
};

enum Modifiers
{
	// name                    hex       oct dec binary
	BoldModifier      = wxT('\x002'), // 002   2 00010
	ColourModifier    = wxT('\x003'), // 003   3 00011
	OriginalModifier  = wxT('\x00f'), // 017  15 01111
	ReverseModifier   = wxT('\x016'), // 026  22 10110
	UnderlineModifier = wxT('\x01f')  // 037  31 11111
};

const ModifierMapping modifier_mappings[] =
{
	// ctrl  alt    shift  key  modifier_char      alt      ctrl-k
	{ true,  false, false, 'B', BoldModifier,      wxT("\244b"), false },
	{ true,  false, false, 'O', OriginalModifier,  wxT("\244c"), false },
	{ true,  false, false, 'R', ReverseModifier,   wxT("\244r"), false },
	{ true,  false, false, 'U', UnderlineModifier, wxT("\244u"), false },
//	{ true,  false, false, 'I', ItalicModifier,    wxT("\244i"), false },
	{ true,  false, false, 'K', ColourModifier,    wxT("\244"),  true  }
};

const size_t modifier_count = ((sizeof modifier_mappings) / (sizeof modifier_mappings[0]));

const wxColour colours[] =
{

	wxColour(255, 255, 255),
	wxColour(0,   0,   0),
	wxColour(0,   0,   128),
	wxColour(0,   128, 0),

	wxColour(255, 0,   0),
	wxColour(128, 0,   0),
	wxColour(128, 0,   128),
	wxColour(255, 128, 0),

	wxColour(255, 255, 0),
	wxColour(0,   255, 0),
	wxColour(0,   128, 128),
	wxColour(0,   255, 255),

	wxColour(0,   0,   255),
	wxColour(255, 0,   255),
	wxColour(128, 128, 128),
	wxColour(192, 192, 192)

};

const wxColour colour_backgrounds[] =
{

	wxColour(0,   0,   0),
	wxColour(255, 255, 255),
	wxColour(255, 255, 255),
	wxColour(255, 255, 255),

	wxColour(255, 255, 255),
	wxColour(255, 255, 255),
	wxColour(255, 255, 255),
	wxColour(255, 255, 255),

	wxColour(0,   0,   0),
	wxColour(0,   0,   0),
	wxColour(0,   0,   0),
	wxColour(0,   0,   0),

	wxColour(0,   0,   0),
	wxColour(0,   0,   0),
	wxColour(0,   0,   0),
	wxColour(0,   0,   0)

};

const size_t colour_count = ((sizeof colours) / (sizeof colours[0]));

static inline int GetModifierIndex(wxChar c)
{
	for (size_t i = 0; i < modifier_count; ++i)
	{
		if (modifier_mappings[i].modifier_char == c)
		{
			return i;
		}
	}
	return -1;
}

static inline bool IsModifier(wxChar c)
{
	return (GetModifierIndex(c) > -1);
}

static inline wxString ReplaceAlternateModifiers(const wxString &text)
{

	wxString result = text;

	for (size_t i = 0; i < modifier_count; ++i)
	{
		result.Replace(
			wxString(modifier_mappings[i].alternate_modifier).Lower(),
			wxString(modifier_mappings[i].modifier_char));
		result.Replace(
			wxString(modifier_mappings[i].alternate_modifier).Upper(),
			wxString(modifier_mappings[i].modifier_char));
	}

	return result;

}

#endif
