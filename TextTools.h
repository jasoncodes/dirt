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


#ifndef TextTools_H_
#define TextTools_H_

#include "Modifiers.h"

wxString FormatTextAsHtml(const wxString &text);
wxString ConvertUrlsToLinks(const wxString &text);
wxString ConvertModifiersIntoHtml(const wxString &text, bool strip_mode);
bool IsEmail(const wxString &token);
wxString ColourRGBToString(int red, int green, int blue);
wxString ColourIndexToString(int colour_number);

inline wxString ColourRGBToString(long clr)
{
	return ColourRGBToString(CLR_RED(clr), CLR_GREEN(clr), CLR_BLUE(clr));
}

inline wxString ColourRGBToString(const wxColour &clr)
{
	return ColourRGBToString(clr.Red(), clr.Green(), clr.Blue());
}

#endif

