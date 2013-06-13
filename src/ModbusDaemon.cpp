/*
 * ModbusDaemon.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ModbusDaemon.h"
#include "ModbusTcpLoop.h"
#include "ModbusRtuLoop.h"

#include "Log.h"

#include <stdlib.h>
#include <vector>
#include "Utils.h"

#include "settings_lukoil.h"
#include "settings_tnkbp.h"
#include "params_lukoil.h"
#include "params_tnkbp.h"



CModbusDaemon::CModbusDaemon(std::string processName, int argc, char* argv[])
	: CDaemonProcess(processName, argc, argv), m_pLoop(NULL), m_pPump(NULL), m_tcpPort(DEFAULT_TCP_PORT),
	  m_modbusMode(MODBUS_MODE_NONE)

{
	m_configOptions["ParametersDB"] = "";
	m_configOptions["EventsDB"] = "";
	m_configOptions["MODBUS_LocalAddress"] = "";
	m_configOptions["MODBUS_Rtu_Baudrate"] = "9600";
	m_configOptions["MODBUS_Rtu_Port"] = "";
	m_configOptions["MODBUS_Rtu_Settings"] = "8N1";
	m_configOptions["MODBUS_Rtu_SlaveID"] = "";
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
	// default values for RTU
	int comSpeed;
	char comParity;
	int comBpb;
	int comStopBits;
	int slaveId;

	config_options_iterator itr;

	std::string ipAddr = m_configOptions.find(std::string("MODBUS_LocalAddress"))->second;

	std::string comPort = m_configOptions.find(std::string("MODBUS_Rtu_Port"))->second;

	if(ipAddr.empty() && comPort.empty()) {
		Log("[config]: either MODBUS_LocalAddress or MODBUS_Rtu_Port must be specified");
		return false;
	}


	if(!ipAddr.empty() && !comPort.empty()) {
		Log("[config]: Both MODBUS_LocalAddress and MODBUS_Rtu_Port are specified");
		return false;
	}

	if(!ipAddr.empty()) {
		m_modbusMode = MODBUS_MODE_TCP;
	} else {
		m_modbusMode = MODBUS_MODE_RTU;
	}

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

	data_parameter_t* parameters = NULL;
	int nbParams = 0;

	int nbSettings = 0;
	setting_t* settings = NULL;

	std::string mapping = m_configOptions.find(std::string("MODBUS_Map"))->second;

	if(mapping == std::string("LUKOIL")) {
		settings = lukoil_Settings;
		nbSettings = NUMBER_OF_SETTINGS_LUKOIL;
		parameters = lukoil_params;
		nbParams = NUMBER_OF_PARAMS_LUKOIL;

	} else if (mapping == std::string("TNKBP")) {
		settings = tnkbp_Settings;
		nbSettings = NUMBER_OF_SETTINGS_TNKBP;
		parameters = tnkbp_params;
		nbParams = NUMBER_OF_PARAMS_TNKBP;
	} else {
		Log("[config]: unknown MODBUS_Map value: possible [LUKOIL|TNKBP]");
		return false;
	}



	if (m_modbusMode == MODBUS_MODE_TCP) {
		std::vector<std::string> v = split(ipAddr,':');

		if(v.size() > 0) {
			m_tcpAddr = v[0];
			if(v.size() > 1) {
				m_tcpPort = atoi(v[1].c_str());
			}
		} else {
			Log("[config]: Wrong IP address format for MODBUS_LocalAddress");
			return false;
		}

		m_pLoop = new CModbusTcpLoop(parameters, nbParams, settings, nbSettings, m_sRitexPath,
				m_tcpAddr, m_tcpPort);

	} else {
		std::string s = m_configOptions.find(std::string("MODBUS_Rtu_SlaveID"))->second;

		if(s.empty()) {
			Log("[config]: MODBUS_Rtu_SlaveID must be defined for RTU mode");
			return false;
		}

		slaveId = atoi(s.c_str());

		if(slaveId == 0 && s != "0") {
			Log("[config]: MODBUS_Rtu_SlaveID: must be a number, found %s", s.c_str());
			return false;
		}

		s = m_configOptions.find(std::string("MODBUS_Rtu_Baudrate"))->second;

		if(!s.empty()) {
			comSpeed = atoi(s.c_str());
		}

		s = m_configOptions.find(std::string("MODBUS_Rtu_Settings"))->second;

		if(s.length() == 3) {
			comParity = s.substr(1,1)[0];
			comBpb = atoi(s.substr(0,1).c_str());
			comStopBits = atoi(s.substr(2,1).c_str());
		} else {
			Log("[config]: incorrect format of MODBUS_Rtu_Settings. Ex: 8N1");
			return false;
		}

		m_pLoop = new CModbusRtuLoop(parameters, nbParams, settings, nbSettings, m_sRitexPath,
				comPort, comSpeed, comBpb, comParity, comStopBits, slaveId);
	}

	m_pPump = new CDataPump(parameters, nbParams, settings, nbSettings);


	Log("---- ENV ----");
	Log("Modbus backend: %s", m_modbusMode == MODBUS_MODE_TCP ? "TCP" : "RTU");
	Log("ParamsDB name: %s", m_sParamDbName.c_str());
	Log("EventsDB name: %s", m_sEventDbName.c_str());
	Log("MODBUS mapping: %s", mapping.c_str());

	if(m_modbusMode == MODBUS_MODE_TCP) {
		Log("IP address: %s", m_tcpAddr.c_str());
		Log("TCP port: %d", m_tcpPort);
	} else {
		Log("RTU slave ID: %d", slaveId);
		Log("RTU port: %s", comPort.c_str());
		Log("RTU speed: %d",comSpeed);
		Log("RTU bpb: %d", comBpb);
		Log("TRU parity %c", comParity);
		Log("RTU stop bits %d", comStopBits);
	}

	Log("-------------");



	return true;
}


int CModbusDaemon::daemonLoop()
{
	Log("daemonLoop() -->>");
	if(m_pPump->Create(m_sParamDbName, m_sEventDbName)) {
		m_pPump->RegisterDataUpdateListener(m_pLoop);
		if(m_pLoop->Create()) {
			m_pLoop->Join();
			//FIXME: implement thread state listeners and cancel from it
			m_pPump->Cancel();
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
