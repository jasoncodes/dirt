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
RCS_ID($Id: FileDropTarget.cpp,v 1.3 2004-05-16 04:42:45 jason Exp $)

#include "FileDropTarget.h"

const wxEventType wxEVT_FILE_DROP = wxNewEventType();

FileDropTarget::FileDropTarget(wxEvtHandler *handler, wxWindowID id)
	: m_handler(handler), m_id(id)
{
}

FileDropTarget::~FileDropTarget()
{
}

bool FileDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
{
	if (m_handler)
	{
		FileDropEvent evt(m_id, this, filenames);
		m_handler->ProcessEvent(evt);
		return evt.IsAccepted();	
	}
	return false;
}

wxDragResult FileDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	wxDragResult result = wxFileDropTarget::OnDragOver(x, y, def);
	if (result == wxDragMove || result == wxDragLink)
	{
		result = wxDragCopy;
	}
	if (result == wxDragCopy && m_handler)
	{
		FileDropEvent evt(m_id, this);
		m_handler->ProcessEvent(evt);
		result = evt.IsAccepted() ? wxDragCopy : wxDragNone;	
	}
	return result;
}
