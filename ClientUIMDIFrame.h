#ifndef ClientUIMDIFrame_H_
#define ClientUIMDIFrame_H_

#include "SwitchBarParent.h"
#include "Client.h"
#include <wx/datetime.h>

class ClientUIMDICanvas;
class NickListControl;
class ClientUIMDITransferPanel;
class TrayIcon;

class ClientUIMDIFrame : public SwitchBarParent, public ClientEventHandler
{

public:

	ClientUIMDIFrame();
	virtual ~ClientUIMDIFrame();

	Client* GetClient() { return m_client; }

	bool IsFocused();
	bool ResetWindowPos();
	void ResetRedLines();
	wxArrayString* GetNicklist() { return &m_nicklist; }
	wxDateTime GetLogDate();

protected:
	void OnFileExit(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnFocusTimer(wxTimerEvent &event);
	void OnActivate(wxActivateEvent &event);
	void OnTrayDblClick(wxMouseEvent &event);
	void OnTrayRightClick(wxMouseEvent &event);
	void OnRestore(wxCommandEvent &event);
	void OnIconize(wxIconizeEvent &event);
	void OnTrayTimer(wxTimerEvent &event);
	void OnBinding(wxCommandEvent &event);
	void OnCtrlF(wxCommandEvent &event);

	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual wxArrayString OnClientSupportedCommands();
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientError(const wxString &context, const wxString &type, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientStateChange();
	virtual void OnClientAuthNeeded(const wxString &text);
	virtual void OnClientAuthDone(const wxString &text);
	virtual void OnClientAuthBad(const wxString &text);
	virtual void OnClientMessageOut(const wxString &context, const wxString &nick, const wxString &text, bool is_action);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_action, bool is_private);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick);
	virtual void OnClientUserAway(const wxString &nick, const wxString &msg);
	virtual void OnClientUserBack(const wxString &nick, const wxString &msg);
	virtual void OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details);
	virtual void OnClientTransferNew(const FileTransfer &transfer);
	virtual void OnClientTransferDelete(const FileTransfer &transfer);
	virtual void OnClientTransferState(const FileTransfer &transfer);
	virtual void OnClientTransferTimer(const FileTransfer &transfer);

protected:
	wxTimer *m_tmrFocus;
	Client *m_client;
	bool m_focused;
	NickListControl *m_lstNickList;
	wxArrayString m_nicklist;
	bool m_alert;
	int m_flash;
	TrayIcon *m_tray;
	wxString m_title;
	wxTimer *m_tmrTray;
	bool m_tray_flash;
	bool m_tray_auto_restore;
	wxDateTime m_log_date;
	bool m_log_date_okay;

	ClientUIMDICanvas* GetContext(const wxString &context, bool create_if_not_exist = true, bool on_not_exist_return_null = false);
	ClientUIMDITransferPanel* GetContext(const long transferid);
	void AddLine(const wxString &context, const wxString &line, const wxColour &line_colour = *wxBLACK, bool create_if_not_exist = true, bool suppress_alert = false, bool convert_urls = true);
	void UpdateCaption();
	void NickPrompt(const wxString &nick);

private:
	DECLARE_EVENT_TABLE()

};

#endif
