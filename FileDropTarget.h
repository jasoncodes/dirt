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


#ifndef FileDropTarget_H_
#define FileDropTarget_H_

#include <wx/dnd.h>

class FileDropTarget : public wxEvtHandler, public wxFileDropTarget
{

public:
	FileDropTarget(wxEvtHandler *handler, wxWindowID id);
	virtual ~FileDropTarget();

protected:
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);

protected:
	wxEvtHandler *m_handler;
	wxWindowID m_id;

private:
	DECLARE_NO_COPY_CLASS(FileDropTarget)

};


extern const wxEventType wxEVT_FILE_DROP;

class FileDropEvent : public wxEvent
{

public:
	FileDropEvent(int id, FileDropTarget *src, const wxArrayString &filenames)
		: wxEvent(id, wxEVT_FILE_DROP), m_filenames(filenames), m_is_drop(true), m_is_accepted(false)
	{
		SetEventObject(src);
	}

	FileDropEvent(int id, FileDropTarget *src)
		: wxEvent(id, wxEVT_FILE_DROP), m_is_drop(false), m_is_accepted(false)
	{
		SetEventObject(src);
	}

	virtual FileDropTarget *GetFileDropTarget() const
	{
		return (FileDropTarget*)GetEventObject();
	}

	virtual bool IsDrop()
	{
		return m_is_drop;
	}

	virtual bool IsAccepted()
	{
		return m_is_accepted;
	}

	virtual void Accept(bool is_accepted)
	{
		m_is_accepted = is_accepted;
	}

	virtual const wxArrayString& GetFilenames() const
	{
		return m_filenames;
	}

	virtual wxEvent *Clone() const
	{
		return new FileDropEvent(*this);
	}

protected:
	wxArrayString m_filenames;
	bool m_is_drop;
	bool m_is_accepted;

};

typedef void (wxEvtHandler::*FileDropEventFunction)(FileDropEvent&);

#define EVT_FILE_DROP(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_FILE_DROP, id, -1, \
		(wxObjectEventFunction) \
		wxStaticCastEvent( FileDropEventFunction, &func ), \
		(wxObject *) NULL ),

#endif
