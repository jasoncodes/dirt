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


#ifndef TrayIcon_H_
#define TrayIcon_H_

class TrayIconPrivate;

class TrayIcon : public wxEvtHandler
{

	friend class TrayIconPrivate;

public:
	TrayIcon();
	virtual ~TrayIcon();

	virtual void SetEventHandler(wxEvtHandler *handler, wxEventType id = wxID_ANY);

	virtual bool Ok();

	virtual void SetIcon(const char **xpm);
	virtual void SetToolTip(const wxString &tooltip);

	virtual bool PopupMenu(wxMenu *menu, const wxPoint &pos);

protected:
	TrayIconPrivate *m_priv;
	wxEvtHandler *m_handler;
	wxEventType m_id;

private:
	DECLARE_NO_COPY_CLASS(TrayIcon)

};

#define EVT_TRAYICON_LEFT_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_DOWN, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_LEFT_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_UP, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_DOWN, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_UP, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_DOWN, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_UP, wxID_ANY, id, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_LEFT_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_DCLICK, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_DCLICK, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_DCLICK, id, wxID_ANY, (wxObjectEventFunction) wxStaticCastEvent( wxMouseEventFunction, &func ), (wxObject *) NULL ),

#endif
