#ifndef ConfigFile_H_
#define ConfigFile_H_

#include <wx/confbase.h>
#include <wx/fileconf.h>

class CryptSocketProxySettings;

class ConfigFile : public wxFileConfig
{

public:
	ConfigFile();
	virtual ~ConfigFile();

};

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

	virtual wxConfigBase* GetConfig() const { return m_config; }

	virtual bool Flush();
	virtual bool ResetToDefaults();

	virtual wxString GetActualLogDir() const;
	virtual TristateMode GetLogDirType() const;
	virtual wxString GetLogDirPath() const;
	virtual bool SetLogDir(TristateMode type, const wxString &dir);

protected:
	virtual wxString GetPassword(const wxString &key, bool decrypt) const;
	virtual bool SetPassword(const wxString &key, const wxString &password);
	virtual wxString DecodePassword(const wxString &value, bool decrypt) const;
	virtual wxString EncodePassword(const wxString &password) const;

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
