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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ConfigFile.cpp,v 1.18 2005-03-02 14:32:00 jason Exp $)

#include "ConfigFile.h"
#include "Dirt.h"
#include "util.h"
#include <wx/filename.h>
#include "Crypt.h"
#include <locale.h>
#include "Mutex.h"

DECLARE_APP(DirtApp)

const wxEventType wxEVT_CONFIG_FILE_CHANGED = wxNewEventType();

//////// ConfigFile ////////

class ConfigFileHelper : wxTimer
{
	
	friend class ConfigFile;

protected:
	ConfigFileHelper(ConfigFile *config)
		: m_config(config)
	{
		Start(2500);
	}

	virtual void Notify()
	{
		m_config->EnsureLatestLoaded();
	}

protected:
	ConfigFile *m_config;

};

#define BEGIN_READ() \
	bool lock_read_ok = MutexLock(); \
	EnsureLatestLoaded();

#define END_READ() \
	if (lock_read_ok) \
	{ \
		MutexUnlock(); \
		lock_read_ok = false; \
	}

#define BEGIN_WRITE() \
	bool lock_write_ok = MutexLock(); \
	EnsureLatestLoaded();

#define END_WRITE() \
	if (lock_write_ok) \
	{ \
		MaybeFlush(); \
		MutexUnlock(); \
		lock_write_ok = false; \
	}

wxString ConfigFile::GetConfigFilename()
{
	wxString filename = wxGetApp().GetConfigFilename();
	if (filename.Length() == 0)
	{
		wxFileName fn(GetSelf());
		fn.SetFullName(wxT("dirt.ini"));
		if (fn.FileExists())
		{
			return fn.GetFullPath();
		}
		return wxFileConfig::GetLocalFileName(wxT("dirt"));
	}
	return filename;
}

ConfigFile::ConfigFile()
	: m_base(NULL)
{
	m_handler = NULL;
	m_id = wxID_ANY;
	m_batch_depth = 0;
	m_filename = GetConfigFilename();
	m_mutex = new Mutex(m_filename+wxT(".lock"));
	m_helper = new ConfigFileHelper(this);
	ReInit();
}

ConfigFile::~ConfigFile()
{
	delete m_helper;
	wxASSERT_MSG(!m_batch_depth, wxT("EndBatch() not called before destroying ConfigFile"));
	wxASSERT_MSG(!m_mutex->IsLocked(), wxT("Config mutex locked in destructor"));
	bool lock_ok = MutexLock();
	delete m_base;
	if (lock_ok)
	{
		MutexUnlock();
	}
	delete m_mutex;
}

void ConfigFile::ReInit()
{
	bool lock_ok = MutexLock();
	char *old_locale = setlocale(LC_ALL, "C");
	if (m_base)
	{
		wxString temp_name;
		temp_name << m_filename << wxT(".tmp") << wxGetProcessId();
		bool rename_ok = wxRenameFile(m_filename, temp_name);
		if (rename_ok)
		{
			delete m_base;
			wxRemoveFile(m_filename);
			rename_ok = wxRenameFile(temp_name, m_filename);
			wxASSERT_MSG(rename_ok, wxT("Failed to rename back to original"));
		}
		else
		{
			wxFAIL_MSG(wxT("Failed to rename to temp filename during ReInit"));
			delete m_base;
		}
	}
	m_base = new wxFileConfig(wxT("dirt"), wxT(""), m_filename);
	m_base->SetUmask(0077);
	setlocale(LC_ALL, old_locale);
	m_last_file_mod = GetLastFileModified();
	if (lock_ok)
	{
		MutexUnlock();
	}
}

bool ConfigFile::MutexLock() const
{
	bool ok = m_mutex->Lock();
	wxASSERT_MSG(ok, wxT("Error locking config mutex"));
	return ok;
}

void ConfigFile::MutexUnlock() const
{
	m_mutex->Unlock();
}

bool ConfigFile::BeginBatch()
{
	bool ok = MutexLock();
	if (ok)
	{
		m_batch_depth++;
	}
	EnsureLatestLoaded();
	return ok;
}

void ConfigFile::EndBatch()
{
	wxASSERT_MSG(m_batch_depth, wxT("EndBatch() without BeginBatch()"));
	Flush();
	if (m_batch_depth)
	{
		MutexUnlock();
		m_batch_depth--;
	}
}

void ConfigFile::SetPath(const wxString& strPath)
{
	m_base->SetPath(strPath);
}

const wxString& ConfigFile::GetPath() const
{
	return m_base->GetPath();
}

bool ConfigFile::GetFirstGroup(wxString& str, long& lIndex) const
{
	wxASSERT_MSG(m_batch_depth, wxT("GetFirstGroup must be used between BeginBatch() and EndBatch()"));
	BEGIN_READ();
	bool retval = m_base->GetFirstGroup(str, lIndex);
	END_READ();
	return retval;
}

bool ConfigFile::GetNextGroup(wxString& str, long& lIndex) const
{
	return m_base->GetNextGroup(str, lIndex);
}

bool ConfigFile::GetFirstEntry(wxString& str, long& lIndex) const
{
	wxASSERT_MSG(m_batch_depth, wxT("GetFirstGroup must be used between BeginBatch() and EndBatch()"));
	BEGIN_READ();
	bool retval = m_base->GetFirstEntry(str, lIndex);
	END_READ();
	return retval;
}

bool ConfigFile::GetNextEntry(wxString& str, long& lIndex) const
{
	return m_base->GetNextEntry(str, lIndex);
}

size_t ConfigFile::GetNumberOfEntries(bool bRecursive) const
{
	BEGIN_READ();
	size_t retval = m_base->GetNumberOfEntries(bRecursive);
	END_READ();
	return retval;
}

size_t ConfigFile::GetNumberOfGroups(bool bRecursive) const
{
	BEGIN_READ();
	size_t retval = m_base->GetNumberOfGroups(bRecursive);
	END_READ();
	return retval;
}

bool ConfigFile::HasGroup(const wxString& strName) const
{
	BEGIN_READ();
	bool retval = m_base->HasGroup(strName);
	END_READ();
	return retval;
}

bool ConfigFile::HasEntry(const wxString& strName) const
{
	BEGIN_READ();
	bool retval = m_base->HasEntry(strName);
	END_READ();
	return retval;
}

bool ConfigFile::Flush(bool bCurrentOnly)
{
	bool lock_ok = MutexLock();
	wxASSERT_MSG(lock_ok, wxT("Error locking config mutex"));
	wxASSERT(IsLatestLoaded());
	char *old_locale = setlocale(LC_ALL, "C");
	bool ok = m_base->Flush(bCurrentOnly);
	wxASSERT_MSG(ok, wxT("Failed to flush to disk"));
	setlocale(LC_ALL, old_locale);
	if (ok)
	{
		m_last_file_mod = GetLastFileModified();
	}
	if (lock_ok)
	{
		MutexUnlock();
	}
	return ok;
}

bool ConfigFile::MaybeFlush()
{
	wxASSERT_MSG(m_mutex->IsLocked(), wxT("Mutex not locked in call to MaybeFlush()"));
	if (m_mutex->GetLockDepth() == 1)
	{
		return Flush();
	}
	return false;
}

wxDateTime ConfigFile::GetLastFileModified() const
{
	wxFileName fn(m_filename);
	if (fn.FileExists())
	{
		wxLogNull suppress_logging;
		wxDateTime last_mod;
		if (fn.GetTimes(NULL, &last_mod, NULL))
		{
			wxASSERT(last_mod.IsValid());
			return last_mod;
		}
		wxSleep(1);
		if (fn.GetTimes(NULL, &last_mod, NULL))
		{
			wxASSERT(last_mod.IsValid());
			return last_mod;
		}
	}
	return wxDateTime(1, wxDateTime::Jan, 1970).ToGMT();
}

bool ConfigFile::IsLatestLoaded() const
{
	bool lock_ok = MutexLock();
	bool latest = (m_last_file_mod == GetLastFileModified());
	if (lock_ok)
	{
		MutexUnlock();
	}
	return latest;
}

void ConfigFile::EnsureLatestLoaded() const
{
	bool lock_ok = MutexLock();
	if (!IsLatestLoaded())
	{
		const_cast<ConfigFile*>(this)->ReInit();
		SendEvent();
	}
	if (lock_ok)
	{
		MutexUnlock();
	}
}

void ConfigFile::SetEventHandler(wxEvtHandler *handler, int id)
{
	m_handler = handler;
	m_id = id;
}

void ConfigFile::SendEvent() const
{
	if (m_handler)
	{
		wxCommandEvent evt(wxEVT_CONFIG_FILE_CHANGED, m_id);
		m_handler->AddPendingEvent(evt);
	}
}

bool ConfigFile::RenameEntry(const wxString& oldName, const wxString& newName)
{
	BEGIN_WRITE();
	bool retval = m_base->RenameEntry(oldName, newName);
	END_WRITE();
	return retval;
}

bool ConfigFile::RenameGroup(const wxString& oldName, const wxString& newName)
{
	BEGIN_WRITE();
	bool retval = m_base->RenameGroup(oldName, newName);
	END_WRITE();
	return retval;
}

bool ConfigFile::DeleteEntry(const wxString& key, bool bGroupIfEmptyAlso)
{
	BEGIN_WRITE();
	bool retval = m_base->DeleteEntry(key, bGroupIfEmptyAlso);
	END_WRITE();
	return retval;
}

bool ConfigFile::DeleteGroup(const wxString& szKey)
{
	BEGIN_WRITE();
	bool retval = m_base->DeleteGroup(szKey);
	END_WRITE();
	return retval;
}

bool ConfigFile::DeleteAll()
{
	BEGIN_WRITE();
	bool retval = m_base->DeleteAll();
	END_WRITE();
	return retval;
}

bool ConfigFile::DoReadString(const wxString& key, wxString *pStr) const
{
	BEGIN_READ();
	bool retval = m_base->Read(key, pStr);
	END_READ();
	return retval;
}

bool ConfigFile::DoReadLong(const wxString& key, long *pl) const
{
	BEGIN_READ();
	bool retval = m_base->Read(key, pl);
	END_READ();
	return retval;
}

bool ConfigFile::DoWriteString(const wxString& key, const wxString& szValue)
{
	BEGIN_WRITE();
	bool retval = m_base->Write(key, szValue);
	END_WRITE();
	return retval;
}

bool ConfigFile::DoWriteLong(const wxString& key, long lValue)
{
	BEGIN_WRITE();
	bool retval = m_base->Write(key, lValue);
	END_WRITE();
	return retval;
}

//////// Config ////////

Config::Config(const wxString &path)
	: m_config(new ConfigFile), m_path(path)
{
	wxASSERT(m_path.Length());
	if (m_path[0u] != wxT('/'))
	{
		m_path.Prepend(wxT('/'));
	}
	wxASSERT(m_path.Length());
	if (m_path.Last() == wxT('/'))
	{
		m_path = m_path.Left(m_path.Length() - 1);
	}
	wxASSERT(m_path.Length());
}

Config::~Config()
{
	delete m_config;
}

bool Config::Flush()
{
	return m_config->Flush();
}

bool Config::ResetToDefaults()
{
	return m_config->DeleteGroup(m_path);
}

static const wxString EncodedPrefix = wxT("Encoded:");

wxString Config::DecodePassword(const wxString &value, bool decrypt) const
{

	if (!decrypt || value.Length() == 0 || !LeftEq(value, EncodedPrefix))
	{
		return value;
	}

	ByteBuffer data = Crypt::Base64Decode(value.Mid(EncodedPrefix.Length()));
	
	if (data.Length() < 40)
	{
		return wxEmptyString;
	}
	
	if (((data.Length() - 40) % 16) != 0)
	{
		return wxEmptyString;
	}

	const byte *ptr = data.LockRead();
	ByteBuffer crc32(ptr, 4);
	ByteBuffer len_buff(ptr+4, 4);
	size_t len = BytesToUint32(len_buff.LockRead(), len_buff.Length());
	len_buff.Unlock();
	ByteBuffer AESKey(ptr+8, 32);
	ByteBuffer enc(ptr+40, data.Length()-40);
	data.Unlock();	
	
	try
	{
		Crypt crypt;
		crypt.SetAESDecryptKey(AESKey);
		data = crypt.AESDecrypt(enc);
	}
	catch (...)
	{
		return wxEmptyString;
	}
	
	ByteBuffer dec(data.LockRead(), len);
	data.Unlock();
	
	if (crc32 == Crypt::CRC32(AESKey + dec))
	{
		return dec;
	}

	return wxEmptyString;

}

wxString Config::GetPassword(const wxString &key, bool decrypt) const
{
	return DecodePassword(m_config->Read(key), decrypt);
}

bool Config::SetPassword(const wxString &key, const wxString &password)
{
	if (password.Length())
	{
		wxString data = EncodePassword(password);
		return data.Length() && m_config->Write(key, data);
	}
	else
	{
		return m_config->Write(key, wxEmptyString);
	}
}

wxString Config::EncodePassword(const wxString &password) const
{
	ByteBuffer data;
	wxASSERT(password.Length());
	if (password.Length() > 0)
	{
		if (LeftEq(password, EncodedPrefix))
		{
			if (DecodePassword(password, true).Length() > 0)
			{
				data = password;
			}
		}
		else
		{
			ByteBuffer AESKey = Crypt::Random(32);
			Crypt crypt;
			crypt.SetAESEncryptKey(AESKey);
			data =
				Crypt::CRC32(AESKey + ByteBuffer(password)) + 
				Uint32ToBytes(password.Length()) +
				AESKey +
				crypt.AESEncrypt(password);
			data = EncodedPrefix + Crypt::Base64Encode(data, false);
		}
	}
	return data;
}

wxString Config::GetTristateString(const wxString &key, bool is_dir) const
{

	switch (GetTristateMode(key))
	{

		case tsmNone:
			return wxEmptyString;

		case tsmCustom:
			{
				wxFileName fn;
				if (is_dir)
				{
					fn = wxFileName(GetTristate(key), wxEmptyString);
				}
				else
				{
					fn = wxFileName(GetTristate(key));
				}
				wxFileName cfg(ConfigFile::GetConfigFilename());
				fn.Normalize(wxPATH_NORM_DOTS|wxPATH_NORM_ABSOLUTE|wxPATH_NORM_TILDE,
					cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
				return fn.GetFullPath();
			}

		case tsmDefault:
		default:
			wxFAIL_MSG(wxT("Unsupported TristateMode in GetTristateString"));
			return wxEmptyString;

	}

}

static Config::TristateMode MaybeToggleMode(const wxString &key, Config::TristateMode mode)
{
	/* Can't enable this until changes made in config dialog *
	if (key == wxT("/Client/Log Directory"))
	{
		if (mode == Config::tsmNone)
		{
			mode = Config::tsmDefault;
		}
		else if (mode == Config::tsmDefault)
		{
			mode = Config::tsmNone;
		}
	}
	*/
	return mode;
}

Config::TristateMode Config::GetTristateMode(const wxString &key) const
{
	Config::TristateMode mode;
	if (m_config->Exists(key))
	{
		if (GetTristate(key).Length())
		{
			mode = tsmCustom;
		}
		else
		{
			mode = tsmNone;
		}
	}
	else
	{
		mode = tsmDefault;
	}
	return MaybeToggleMode(key, mode);
}

wxString Config::GetTristate(const wxString &key) const
{
	return m_config->Read(key);
}

bool Config::SetTristate(const wxString &key, TristateMode type, const wxString &path, bool is_dir)
{

	type = MaybeToggleMode(key, type);

	switch (type)
	{

		case tsmNone:
			return m_config->Write(key, wxEmptyString);

		case tsmDefault:
			if (m_config->Exists(key))
			{
				return m_config->DeleteEntry(key);
			}
			else
			{
				return true;
			}

		case tsmCustom:
			if (path.Length())
			{
				wxFileName fn;
				if (is_dir)
				{
					fn = wxFileName(path, wxEmptyString);
				}
				else
				{
					fn = wxFileName(path);
				}
				wxFileName cfg(ConfigFile::GetConfigFilename());
				fn.MakeRelativeTo(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
				return m_config->Write(key, fn.GetFullPath());
			}
			return false;

		default:
			wxFAIL_MSG(wxT("Unknown TristateMode"));
			return false;

	}

}

wxString Config::GetLogDirKey() const
{
	return m_path+wxT("/Log Directory");
}

wxString Config::GetActualLogDir() const
{
	if (GetLogDirType() == tsmDefault)
	{
		wxFileName cfg(ConfigFile::GetConfigFilename());
		wxFileName fn(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR), wxT(""));
		fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
		return fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
	}
	else
	{
		return GetTristateString(GetLogDirKey(), true);
	}
}

Config::TristateMode Config::GetLogDirType() const
{
	return GetTristateMode(GetLogDirKey());
}

wxString Config::GetLogDirPath() const
{
	return GetTristate(GetLogDirKey());
}

bool Config::SetLogDir(TristateMode type, const wxString &dir)
{
	if (type == tsmCustom)
	{
		if (!wxFileName(dir).DirExists())
		{
			return false;
		}
	}
	return SetTristate(GetLogDirKey(), type, dir, true);
}
