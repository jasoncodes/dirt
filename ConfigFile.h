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


#ifndef ConfigFile_H_
#define ConfigFile_H_

#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/datetime.h>

class CryptSocketProxySettings;
class Mutex;
class ConfigFileHelper;

class ConfigFile : public wxConfigBase
{

	friend class ConfigFileHelper;

public:
	static wxString GetConfigFilename();

public:
	ConfigFile();
	virtual ~ConfigFile();

	virtual void SetPath(const wxString& strPath);
	virtual const wxString& GetPath() const;

	virtual bool GetFirstGroup(wxString& str, long& lIndex) const;
	virtual bool GetNextGroup (wxString& str, long& lIndex) const;
	virtual bool GetFirstEntry(wxString& str, long& lIndex) const;
	virtual bool GetNextEntry (wxString& str, long& lIndex) const;

	virtual size_t GetNumberOfEntries(bool bRecursive = FALSE) const;
	virtual size_t GetNumberOfGroups(bool bRecursive = FALSE) const;

	virtual bool HasGroup(const wxString& strName) const;
	virtual bool HasEntry(const wxString& strName) const;

	virtual bool Flush(bool bCurrentOnly = FALSE);

	virtual bool RenameEntry(const wxString& oldName, const wxString& newName);
	virtual bool RenameGroup(const wxString& oldName, const wxString& newName);

	virtual bool DeleteEntry(const wxString& key, bool bGroupIfEmptyAlso = TRUE);
	virtual bool DeleteGroup(const wxString& szKey);
	virtual bool DeleteAll();

	virtual bool BeginBatch();
	virtual void EndBatch();

	virtual void SetEventHandler(wxEvtHandler *handler, int id);

protected:
	virtual bool DoReadString(const wxString& key, wxString *pStr) const;
	virtual bool DoReadLong(const wxString& key, long *pl) const;

	virtual bool DoWriteString(const wxString& key, const wxString& szValue);
	virtual bool DoWriteLong(const wxString& key, long lValue);

protected:
	virtual void ReInit();
	virtual bool MutexLock() const;
	virtual void MutexUnlock() const;
	virtual bool MaybeFlush();
	virtual wxDateTime GetLastFileModified() const;
	virtual bool IsLatestLoaded() const;
	virtual void EnsureLatestLoaded() const;
	virtual void SendEvent() const;

protected:
	wxFileConfig *m_base;
	ConfigFileHelper *m_helper;
	wxString m_filename;
	Mutex *m_mutex;
	size_t m_batch_depth;
	wxDateTime m_last_file_mod;
	wxEvtHandler *m_handler;
	int m_id;

};

extern const wxEventType wxEVT_CONFIG_FILE_CHANGED;

#define EVT_CONFIG_FILE_CHANGED(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_CONFIG_FILE_CHANGED, id, -1, \
		(wxObjectEventFunction) \
		wxStaticCastEvent( wxCommandEventFunction, &func ), \
		(wxObject *) NULL ),

class Config
{

	friend class CryptSocketProxySettings;

public:

	enum TristateMode
	{
		tsmNone,
		tsmDefault,
		tsmCustom
	};

	Config(const wxString &path);
	virtual ~Config();

	virtual ConfigFile* GetConfig() const { return m_config; }

	virtual bool Flush();
	virtual bool ResetToDefaults();

	virtual wxString GetActualLogDir() const;
	virtual TristateMode GetLogDirType() const;
	virtual wxString GetLogDirPath() const;
	virtual bool SetLogDir(TristateMode type, const wxString &dir);

	virtual wxString GetPassword(const wxString &key, bool decrypt) const;
	virtual bool SetPassword(const wxString &key, const wxString &password);
	virtual wxString DecodePassword(const wxString &value, bool decrypt) const;
	virtual wxString EncodePassword(const wxString &password) const;

	inline bool BeginBatch() const { return m_config->BeginBatch(); }
	inline void EndBatch() const { m_config->EndBatch(); }
	
	inline void SetEventHandler(wxEvtHandler *handler, int id)
	{
		m_config->SetEventHandler(handler, id);
	}

protected:
	virtual wxString GetLogDirKey() const;

	virtual wxString GetTristateString(const wxString &key, bool is_dir) const;
	virtual TristateMode GetTristateMode(const wxString &key) const;
	virtual wxString GetTristate(const wxString &key) const;
	virtual bool SetTristate(const wxString &key, TristateMode type, const wxString &path, bool is_dir);

protected:
	ConfigFile *m_config;
	wxString m_path;

};

#endif
