#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ConfigFile.cpp,v 1.7 2003-06-26 06:13:19 jason Exp $)

#include "ConfigFile.h"
#include "Dirt.h"
#include "util.h"
#include <wx/filename.h>
#include "Crypt.h"
#include <locale.h>

DECLARE_APP(DirtApp)

//////// ConfigFile ////////

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
	}
	return wxFileConfig::GetLocalFileName(wxT("dirt"));
}

ConfigFile::ConfigFile()
	: m_base(NULL)
{
	InitBase();
}

ConfigFile::~ConfigFile()
{
	delete m_base;
}

void ConfigFile::InitBase()
{
	char *old_locale = setlocale(LC_ALL, "C");
	delete m_base;
	m_base = new wxFileConfig(wxT("dirt"), wxT(""), GetConfigFilename());
	m_base->SetUmask(0077);
	setlocale(LC_ALL, old_locale);
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
	return m_base->GetFirstGroup(str, lIndex);
}

bool ConfigFile::GetNextGroup(wxString& str, long& lIndex) const
{
	return m_base->GetNextGroup(str, lIndex);
}

bool ConfigFile::GetFirstEntry(wxString& str, long& lIndex) const
{
	return m_base->GetFirstEntry(str, lIndex);
}

bool ConfigFile::GetNextEntry(wxString& str, long& lIndex) const
{
	return m_base->GetNextEntry(str, lIndex);
}

size_t ConfigFile::GetNumberOfEntries(bool bRecursive) const
{
	return m_base->GetNumberOfEntries(bRecursive);
}

size_t ConfigFile::GetNumberOfGroups(bool bRecursive) const
{
	return m_base->GetNumberOfGroups(bRecursive);
}

bool ConfigFile::HasGroup(const wxString& strName) const
{
	return m_base->HasGroup(strName);
}

bool ConfigFile::HasEntry(const wxString& strName) const
{
	return m_base->HasEntry(strName);
}

bool ConfigFile::Flush(bool bCurrentOnly)
{
	char *old_locale = setlocale(LC_ALL, "C");
	bool success = m_base->Flush(bCurrentOnly);
	setlocale(LC_ALL, old_locale);
	return success;
}

bool ConfigFile::RenameEntry(const wxString& oldName, const wxString& newName)
{
	return m_base->RenameEntry(oldName, newName);
}

bool ConfigFile::RenameGroup(const wxString& oldName, const wxString& newName)
{
	return m_base->RenameGroup(oldName, newName);
}

bool ConfigFile::DeleteEntry(const wxString& key, bool bGroupIfEmptyAlso)
{
	return m_base->DeleteEntry(key, bGroupIfEmptyAlso);
}

bool ConfigFile::DeleteGroup(const wxString& szKey)
{
	return m_base->DeleteGroup(szKey);
}

bool ConfigFile::DeleteAll()
{
	return m_base->DeleteAll();
}

bool ConfigFile::DoReadString(const wxString& key, wxString *pStr) const
{
	return m_base->Read(key, pStr);
}

bool ConfigFile::DoReadLong(const wxString& key, long *pl) const
{
	return m_base->Read(key, pl);
}

bool ConfigFile::DoWriteString(const wxString& key, const wxString& szValue)
{
	return m_base->Write(key, szValue);
}

bool ConfigFile::DoWriteLong(const wxString& key, long lValue)
{
	return m_base->Write(key, lValue);
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
			wxFAIL_MSG(wxT("Unknown LogDirType"));
			return wxEmptyString;

	}

}

Config::TristateMode Config::GetTristateMode(const wxString &key) const
{
	if (m_config->Exists(key))
	{
		if (GetTristate(key).Length())
		{
			return tsmCustom;
		}
		else
		{
			return tsmNone;
		}
	}
	else
	{
		return tsmDefault;
	}
}

wxString Config::GetTristate(const wxString &key) const
{
	return m_config->Read(key);
}

bool Config::SetTristate(const wxString &key, TristateMode type, const wxString &path, bool is_dir)
{

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
