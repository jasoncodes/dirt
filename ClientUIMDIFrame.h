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


#ifndef ClientUIMDIFrame_H_
#define ClientUIMDIFrame_H_

#include "SwitchBarMDI.h"
#include "Client.h"
#include <wx/datetime.h>
#include <wx/sound.h>

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
	bool MinToTray();
	void RestoreFromTray();
	void InitLogs();
	static bool IsHotKeySupported();

protected:
	void OnFileExit(wxCommandEvent& event);
	void OnToolsPasswords(wxCommandEvent& event);
	void OnToolsOptions(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnFocusTimer(wxTimerEvent &event);
	void OnTrayDblClick(wxMouseEvent &event);
	void OnTrayRightClick(wxMouseEvent &event);
	void OnRestore(wxCommandEvent &event);
	void OnIconize(wxIconizeEvent &event);
	void OnTrayTimer(wxTimerEvent &event);
	void OnBinding(wxCommandEvent &event);
	void OnCtrlF(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnConfigFileChanged(wxCommandEvent &event);

protected:
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
	virtual void OnClientCTCPIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPReplyOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick);
	virtual void OnClientUserAway(const wxString &nick, const wxString &msg, long away_time, long away_time_diff, bool already_away, long last_away_time, const wxString &last_msg);
	virtual void OnClientUserBack(const wxString &nick, const wxString &msg, long away_time, long away_time_diff);
	virtual void OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details);
	virtual void OnClientTransferNew(const FileTransfer &transfer);
	virtual void OnClientTransferDelete(const FileTransfer &transfer, bool user_initiated);
	virtual void OnClientTransferState(const FileTransfer &transfer);
	virtual void OnClientTransferTimer(const FileTransfer &transfer);
	virtual ResumeState OnClientTransferResumePrompt(const FileTransfer &transfer, const wxString &new_filename, bool can_resume);

protected:
	ClientUIMDICanvas* GetContext(const wxString &context, bool create_if_not_exist = true, bool on_not_exist_return_null = false, bool case_sensitive = false);
	ClientUIMDITransferPanel* GetContext(const long transferid);
	void AddLine(const wxString &context, const wxString &line, const wxColour &line_colour = *wxBLACK, bool create_if_not_exist = true, bool suppress_alert = false, bool convert_urls = true);
	void UpdateCaption();
	void NickPrompt(const wxString &nick);
	void DoAlert();
	void DoFlashWindow();
	void SetFonts();
	void SetHotKey();
	bool SetHotKey(int keycode, wxUint8 mods);
	void OnHotKey();
	int GetHotKeyKeyCode() const;
	wxUint8 GetHotKeyModifiers() const;
	void OnFocusGained();
	void OnFocusLost();

	wxString ConvertTitleToToolTip(const wxString &title) const;

	#ifdef __WXMSW__
		virtual long MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
	#endif

protected:
	wxTimer *m_tmrFocus;
	Client *m_client;
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
	#if wxUSE_SOUND
		wxSound m_sound;
	#endif
	#ifdef __WXMSW__
		wxLongLong_t m_last_flash_window;
	#endif
	#ifdef __WXMSW__
		int m_hotkey_keycode;
		wxUint8 m_hotkey_mods;
	#endif
	bool m_focused;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientUIMDIFrame)

};

#endif
