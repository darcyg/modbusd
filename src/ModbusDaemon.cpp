/*
 * ModbusDaemon.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ModbusDaemon.h"
#include "Log.h"

#include <stdlib.h>
#include <vector>
#include "Utils.h"

#include "settings_lukoil.h"
#include "settings_tnkbp.h"

static data_parameter_t parameters[] = {
	{1050100090,0.0,0x100,1},
	{1050100100,0.0,0x101,1},
	{1050110050,0.0,0x108,1},
	{1050110060,0.0,0x109,1},
	{1050110070,0.0,0x10A,1},
	{1050111000,0.0,0x10C,1},
	{1050111010,0.0,0x10D,1},
	{1050111020,0.0,0x10E,1},
	{1050100060,0.0,0x10f,1},
	{1050100050,0.0,0x111,1},
	{1050112010,0.0,0x112,1},
	{1050112020,0.0,0x113,1},
	{1050117000,0.0,0x114,1},
	{1080105030,0.0,0x121,1},
	{1080104010,0.0,0x125,1},
	{1080104020,0.0,0x126,1},
	{1080100009,0.0,0x128,1},
	{1080100010,0.0,0x129,1},
};

#define NUMBER_OF_PARAMETERS (sizeof(parameters) / sizeof(data_parameter_t))


CModbusDaemon::CModbusDaemon(std::string processName, int argc, char* argv[])
	: CDaemonProcess(processName, argc, argv), m_pLoop(NULL), m_pPump(NULL), m_tcpPort(DEFAULT_TCP_PORT)

{
	m_configOptions["ParametersDB"] = "";
	m_configOptions["EventsDB"] = "";
	m_configOptions["MODBUS_LocalAddress"] = "127.0.0.1:502";
	m_configOptions["MODBUS_Rtu_Baudrate"] = "9600";
	m_configOptions["MODBUS_Rtu_Port"] = "";
	m_configOptions["MODBUS_Map"] = "LUKOIL";
	m_configOptions["Ritex_Path"] = "";
}

CModbusDaemon::~CModbusDaemon() {
	if(m_pLoop) {
		delete m_pLoop;
		m_pLoop = NULL;
	}
	if(m_pPump) {
		delete m_pPump;
		m_pPump = NULL;
	}
}


bool CModbusDaemon::setupEnvironment()
{
	config_options_iterator itr;

	m_sParamDbName = m_configOptions.find(std::string("ParametersDB"))->second;

	if(m_sParamDbName.empty()) {
		Log("[config]: ParametersDB option is missing");
		return false;
	}

	m_sEventDbName = m_configOptions.find(std::string("EventsDB"))->second;

	if(m_sEventDbName.empty()) {
		Log("[config]: EventsDB option is missing");
		return false;
	}

	m_sRitexPath = m_configOptions.find(std::string("Ritex_Path"))->second;

	if(m_sRitexPath.empty()) {
		Log("[config]: Ritex_Path option is missing");
		return false;
	}


	// default value always exist
	std::string s = m_configOptions.find(std::string("MODBUS_LocalAddress"))->second;

	std::vector<std::string> v = split(s,':');

	if(v.size() > 0) {
		m_tcpAddr = v[0];
		if(v.size() > 1) {
			m_tcpPort = atoi(v[1].c_str());
		}
	} else {
		Log("[config]: Wrong IP address format for MODBUS_LocalAddress");
		return false;
	}

	s = m_configOptions.find(std::string("MODBUS_Map"))->second;

	if(s == std::string("LUKOIL")) {
		m_pLoop = new CModbusLoop(parameters, NUMBER_OF_PARAMETERS, lukoil_Settings, NUMBER_OF_SETTINGS_LUKOIL, m_sRitexPath);
		m_pPump = new CDataPump(parameters, NUMBER_OF_PARAMETERS, lukoil_Settings, NUMBER_OF_SETTINGS_LUKOIL);
	} else if (s == std::string("TNKBP")) {
		m_pLoop = new CModbusLoop(parameters, NUMBER_OF_PARAMETERS, tnkbp_Settings, NUMBER_OF_SETTINGS_TNKBP, m_sRitexPath);
		m_pPump = new CDataPump(parameters, NUMBER_OF_PARAMETERS, tnkbp_Settings, NUMBER_OF_SETTINGS_TNKBP);
	} else {
		Log("[config]: unknown MODBUS_Map value");
		return false;
	}

	Log("---- ENV ----");
	Log("ParamsDB name: %s", m_sParamDbName.c_str());
	Log("EventsDB name: %s", m_sEventDbName.c_str());
	Log("IP address: %s", m_tcpAddr.c_str());
	Log("TCP port: %d", m_tcpPort);
	Log("MODBUS mapping: %s", s.c_str());
	Log("-------------");



	return true;
}


int CModbusDaemon::daemonLoop()
{
	Log("daemonLoop() -->>");
	if(m_pPump->Create(m_sParamDbName, m_sEventDbName)) {
		m_pPump->RegisterDataUpdateListener(m_pLoop);
		if(m_pLoop->Create(m_tcpAddr, m_tcpPort)) {
			m_pLoop->Join();
			m_pPump->Join();
		} else {
			Log("Error creating main loop");
		}
	} else {
		Log("Error creating data pump");
	}
	Log("daemonLoop() --<<");
	return 0;
}

int CModbusDaemon::parentLoop()
{
	Log("parentLoop() -->>");
	Log("parentLoop() --<<");
	return 0;
}
