/**
 * SwitchBar Event Information:
 * 
 * wxEVT_COMMAND_BUTTON_CLICKED  Selection Change
 * wxEVT_COMMAND_MENU_SELECTED   Right Click On Button
 * wxEVT_COMMAND_MIDDLE_CLICK    Middle Click On Button
 * 
 * GetInt()         Currently selected button number
 * GetExtraLong()   Triggering button number
 * GetString()      Triggering button's caption
 * GetClientData()  Triggering button's user data
 */

#ifndef SwitchBar_H_
#define SwitchBar_H_

#include <wx/dynarray.h>

extern const wxEventType wxEVT_COMMAND_MIDDLE_CLICK;

#define EVT_MIDDLE_CLICK(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_MIDDLE_CLICK, winid, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) & fn, (wxObject *) NULL ),

struct SwitchBarButton;
WX_DECLARE_OBJARRAY(SwitchBarButton, SwitchBarButtonArray);

class FileDropEvent;

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

	void RaiseEvent(int triggering_button, wxEventType type);
	int HitTest(const wxPoint& pt);

protected:
	void OnPaint(wxPaintEvent &event);
	void OnMouse(wxMouseEvent &event);
	void OnErase(wxEraseEvent &event);
	void OnFileDrop(FileDropEvent &event);
	void OnSysColourChanged(wxSysColourChangedEvent &event);

protected:
	static wxIcon PrepareIcon(const wxIcon &icon);
	void LoadSystemColours();

protected:
	SwitchBarButtonArray m_buttons;
	int m_selected;
	wxColour m_colour_highlight, m_colour_shadow;
	wxPen m_pen_highlight, m_pen_shadow;

private:
	DECLARE_EVENT_TABLE()

};

#endif
