#ifndef _NickListControl_H
#define _NickListControl_H

class NickListControl : public wxListBox
{

public:

	NickListControl(wxWindow *parent, int id);
	virtual ~NickListControl();

	virtual void Add(const wxString &nick);
	virtual void Remove(const wxString &nick);
	virtual void Clear();
	virtual wxString GetNick(int index);
	virtual int GetCount();

};

#endif
