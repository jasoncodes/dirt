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

};

#define EVT_TRAYICON_LEFT_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_DOWN, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_LEFT_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_UP, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_DOWN, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_UP, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_DOWN(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_DOWN, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_UP(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_UP, wxID_ANY, id, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_LEFT_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_LEFT_DCLICK, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_MIDDLE_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MIDDLE_DCLICK, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),
#define EVT_TRAYICON_RIGHT_DCLICK(id, func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_RIGHT_DCLICK, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxMouseEventFunction) & func, (wxObject *) NULL ),

#endif
