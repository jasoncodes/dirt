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

#endif
