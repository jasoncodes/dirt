#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileDropTarget.cpp,v 1.2 2003-08-01 07:48:01 jason Exp $)

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
