/*
 * ModbusDaemon.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ModbusDaemon.h"
#include "Log.h"

CModbusDaemon::CModbusDaemon(std::string processName, int argc, char* argv[])
	: CDaemonProcess(processName, argc, argv)
{
	m_pLoop = new CModbusLoop();
}

CModbusDaemon::~CModbusDaemon() {
	// TODO Auto-generated destructor stub
}

bool CModbusDaemon::OnConfigOption(std::string& name, std::string& value)
{
	Log("[Config]: [%s]-[%s]", name.c_str(), value.c_str());
	m_configOptions[name] = value;
	return true;
}

bool CModbusDaemon::setupEnvironment()
{
	//TODO: compile config
	return true;
}


int CModbusDaemon::daemonLoop()
{
	Log("daemonLoop() -->>");
	m_pLoop->Join();
	Log("daemonLoop() --<<");
	return 0;
}

int CModbusDaemon::parentLoop()
{
	Log("IN PARENT");
	return 0;
}
