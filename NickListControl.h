#ifndef NickListControl_H_
#define NickListControl_H_

/**
 * EVT_MENU for right click
 * EVT_LISTBOX_DCLICK for double click
 *
 * Get the nick index via event.GetInt() and
 * use NickListControl::GetSelectedNick() to get the nick
 */
class NickListControl : public wxListBox
{

	static const wxString AwayPostfix;

public:

	NickListControl(wxWindow *parent, int id);
	virtual ~NickListControl();

	virtual void Add(const wxString &nick);
	virtual void Remove(const wxString &nick);
	virtual bool GetAway(const wxString &nick);
	virtual bool SetAway(const wxString &nick, bool away);
	virtual void Clear();
	virtual wxString GetNick(int index);
	virtual int GetCount();
	virtual int GetNickIndex(const wxString &nick);
	virtual int GetSelectedIndex();
	virtual wxString GetSelectedNick();

protected:
	void OnRightUp(wxMouseEvent &event);
	void OnMotion(wxMouseEvent &event);

protected:
	int HitTest(const wxPoint &pt);

private:
	DECLARE_EVENT_TABLE()

};

#endif
