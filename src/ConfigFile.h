/*
 * ConfigFile.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_

#include <string>

class IOnConfigOptionCallback {
public:
	virtual bool OnConfigOption(std::string& name, std::string& value) = 0;
protected:
	virtual ~IOnConfigOptionCallback() {}
};

class CConfigFile {
protected:
	std::string m_fileName;
public:
	CConfigFile(std::string filename);
	virtual ~CConfigFile();
	bool parse(IOnConfigOptionCallback * const cb);
};

#endif /* CONFIGFILE_H_ */
