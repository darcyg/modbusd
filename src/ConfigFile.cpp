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
	    std::stringstream iss(line);
	    std::string a, b;

	    line = choppa(line," \t\n");

	    if(line.empty() || line.at(0) == '#')
	    	continue;

	    Log("LINE: %s", line.c_str());

	    if (!(iss >> a >> b)) {
			Log("Error parsing config file %s", m_fileName.c_str());
	    	return false;
	    }
	    if(!cb->OnConfigOption(a,b)) {
			Log("Error parsing config file %s: option %s was not accepted", m_fileName.c_str(), a.c_str());
			return false;
	    }
	}

	infile.close();
	return true;
}

