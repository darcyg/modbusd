/*
 * ConfigFile.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ConfigFile.h"
#include <fstream>
#include <sstream>
#include "Log.h"
#include "Utils.h"

CConfigFile::CConfigFile(std::string filename)
	: m_fileName(filename)
{
	// TODO Auto-generated constructor stub

}

CConfigFile::~CConfigFile() {
	// TODO Auto-generated destructor stub
}

bool CConfigFile::parse(IOnConfigOptionCallback * const cb) {
	std::ifstream infile;
	std::string line;

	infile.open(m_fileName.c_str());

	if(!infile.is_open()) {
		Log("Cannot open config file %s", m_fileName.c_str());
		return false;
	}

	while (std::getline(infile, line))
	{
	    std::vector<std::string> parts;

	    line = choppa(line," \t\n");

	    if(line.empty() || line.at(0) == '#')
	    	continue;
#ifdef __DEBUG_CONF_PARSER__
	    Log("LINE: %s", line.c_str());
#endif
	    parts = split(line, '=');

		if (parts.size() != 2) {
			Log("Error parsing config file %s\n\t at line [%s]", m_fileName.c_str(), line.c_str());
			return false;
		}


	    parts[0] = choppa(parts[0], " \t\n");
	    parts[1] = choppa(parts[1], " \t\n");

	    if(!cb->OnConfigOption(parts[0], parts[1])) {
			Log("Error parsing config file %s: option %s was not accepted", m_fileName.c_str(), parts[0].c_str());
			return false;
	    }
	}

	infile.close();
	return true;
}

