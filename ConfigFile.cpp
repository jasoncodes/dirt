#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ConfigFile.cpp,v 1.3 2003-05-14 07:53:14 jason Exp $)

#include "ConfigFile.h"
#include "Dirt.h"
#include "util.h"
#include <wx/filename.h>
#include "Crypt.h"

DECLARE_APP(DirtApp)

static wxString GetConfigFilename()
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
	: wxFileConfig(wxT("dirt"), wxT(""), GetConfigFilename())
{
	SetUmask(0077);
}

ConfigFile::~ConfigFile()
{
}

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

static wxString DecodePassword(const wxString &value, bool decrypt)
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
	ByteBuffer data;
	if (password.Length() > 0)
	{
		if (LeftEq(password, EncodedPrefix))
		{
			return (DecodePassword(password, true).Length() > 0);
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
	return m_config->Write(key, data);
}

wxString Config::GetActualLogDir() const
{

	switch (GetLogDirType())
	{

		case ldtNone:
			return wxEmptyString;

		case ldtDefault:
			{
				wxFileName cfg(GetConfigFilename());
				wxFileName fn(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR), wxT(""));
				fn.SetPath(fn.GetPathWithSep() + wxT("dirtlogs"));
				return fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
			}

		case ldtCustom:
			{
				wxFileName fn(GetLogDirPath(), wxEmptyString);
				wxFileName cfg(GetConfigFilename());
				fn.MakeAbsolute(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
				return fn.GetFullPath();
			}

		default:
			wxFAIL_MSG(wxT("Unknown LogDirType"));
			return wxEmptyString;

	}

}

wxString Config::GetLogDirKey() const
{
	return m_path+wxT("/Log Directory");
}

Config::LogDirType Config::GetLogDirType() const
{
	if (m_config->Exists(GetLogDirKey()))
	{
		if (GetLogDirPath().Length())
		{
			return ldtCustom;
		}
		else
		{
			return ldtNone;
		}
	}
	else
	{
		return ldtDefault;
	}
}

wxString Config::GetLogDirPath() const
{
	return m_config->Read(GetLogDirKey());
}

bool Config::SetLogDir(LogDirType type, const wxString &dir)
{

	switch (type)
	{

		case ldtNone:
			return m_config->Write(GetLogDirKey(), wxEmptyString);

		case ldtDefault:
			return m_config->DeleteEntry(GetLogDirKey());

		case ldtCustom:
			if (dir.Length())
			{
				wxFileName fn(dir, wxEmptyString);
				wxFileName cfg(GetConfigFilename());
				fn.MakeRelativeTo(cfg.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
				return m_config->Write(GetLogDirKey(), fn.GetFullPath());
			}
			return false;

		default:
			wxFAIL_MSG(wxT("Unknown LogDirType"));
			return false;

	}

}
