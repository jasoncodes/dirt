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
//  name                hex         oct dec binary
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
	wxColour(0,   0,   127),
	wxColour(0,   147, 0),

	wxColour(255, 0,   0),
	wxColour(127, 0,   0),
	wxColour(156, 0,   156),
	wxColour(252, 127, 0),

	wxColour(255, 255, 0),
	wxColour(0,   252, 0),
	wxColour(0,   147, 147),
	wxColour(0,   255, 255),

	wxColour(0,   0,   252),
	wxColour(255, 0,   255),
	wxColour(127, 127, 127),
	wxColour(210, 210, 210)

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
	for (int i = 0; i < modifier_count; ++i)
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
