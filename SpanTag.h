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


#ifndef SpanTag_H_
#define SpanTag_H_

#include <wx/html/winpars.h>

class SpanTagHandler : public wxHtmlWinTagHandler
{

public:

	SpanTagHandler();
	virtual ~SpanTagHandler();
	virtual wxString GetSupportedTags();
	static bool ParseColour(const wxString& str, wxColour *clr);
	static wxString ExtractStyle(const wxString &style_tag, const wxString &style);
	virtual bool HandleTag(const wxHtmlTag& tag);

	DECLARE_NO_COPY_CLASS(SpanTagHandler)

};

#endif
