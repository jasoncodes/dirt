#ifndef ConfigFile_H_
#define ConfigFile_H_

#include <wx/confbase.h>
#include <wx/fileconf.h>

class ConfigFile : public wxFileConfig
{

public:
	ConfigFile();
	virtual ~ConfigFile();

};

class Config
{

public:

	enum LogDirType
	{
		ldtNone,
		ldtDefault,
		ldtCustom
	};

	Config(const wxString &path);
	virtual ~Config();

	virtual wxConfigBase* GetConfig() const { return m_config; }

	virtual bool Flush();
	virtual bool ResetToDefaults();

	virtual wxString GetActualLogDir() const;
	virtual LogDirType GetLogDirType() const;
	virtual wxString GetLogDirPath() const;
	virtual bool SetLogDir(LogDirType type, const wxString &dir);

protected:
	virtual wxString GetPassword(const wxString &key, bool decrypt) const;
	virtual bool SetPassword(const wxString &key, const wxString &password);
	virtual wxString GetLogDirKey() const;

protected:
	ConfigFile *m_config;
	wxString m_path;

};

#endif
