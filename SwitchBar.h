/**
 * SwitchBar Event Information:
 * 
 * wxEVT_COMMAND_BUTTON_CLICKED  Selection Change
 * wxEVT_COMMAND_MENU_SELECTED   Right Click On Button
 * 
 * GetInt()         Currently selected button number
 * GetExtraLong()   Triggering button number
 * GetString()      Triggering button's caption
 * GetClientData()  Triggering button's user data
 */

#ifndef _SwitchBar_H
#define _SwitchBar_H

#include <wx/dynarray.h>

struct SwitchBarButton;
WX_DECLARE_OBJARRAY(SwitchBarButton, SwitchBarButtonArray);

class SwitchBar : public wxPanel
{

public:

	SwitchBar(
		wxWindow *parent,
		wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER,
		const wxString& name = wxToolBarNameStr);

	virtual ~SwitchBar();

	int AddButton(const wxString &caption, const wxIcon &icon = wxNullIcon, void *user_data = NULL);
	void RemoveButton(int button_index);
	int GetButtonCount();
	void SelectButton(int button_index);
	int GetIndexFromUserData(void *user_data);
	void *GetUserDataFromIndex(int button_index);
	int GetSelectedIndex();
	int GetButtonWidth();
	wxRect GetButtonRect(int button_index);
	void SetButtonCaption(int button_index, const wxString &caption);
	wxString GetButtonCaption(int button_index);
	void SetButtonIcon(int button_index, const wxIcon &icon);
	wxIcon GetButtonIcon(int button_index);
	void SimulateClick(int button_index);
	void SetButtonHighlight(int button_index, bool new_value);
	bool GetButtonHighlight(int button_index);
	void SetButtonProgress(int button_index, int new_value);
	int GetButtonProgress(int button_index);

	void RaiseEvent(int triggering_button, bool is_right_click);
	int HitTest(const wxPoint& pt);

protected:
	void OnPaint(wxPaintEvent &event);
	void OnMouse(wxMouseEvent &event);
	void OnErase(wxEraseEvent &event);

	static wxIcon PrepareIcon(const wxIcon &icon);

protected:
	SwitchBarButtonArray m_buttons;
	int m_selected;

};

#endif
