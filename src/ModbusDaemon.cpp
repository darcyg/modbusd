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
#include "IPCCommand.h"

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


static setting_t settings[] = {
	{"Частота вращения",0.0, 0x257,1},
	{"Защита по перегрузу",0.0, 0x232,1},
	{"Время блокировки защиты по перегрузке",0.0, 0x233,1},
	{"Время блокировки АПВ по перегрузке",0.0, 0x235,1},
	{"Количество перезапусков",0.0, 0x251,1},
	{"Защита по недогрузу",0.0, 0x237,1},
	{"Время блокировки защиты по недогрузу",0.0, 0x238,1},
	{"Время блокировки АПВ по недогрузу",0.0, 0x23A,1},
	{"Скважина",0.0, 0x273,1},
	{"Время работы",0.0, 0x254,1},
	{"Время паузы",0.0, 0x255,1},
	{"Время (часы, минуты)",0.0,0x27C,1},
	{"Дата (число, месяц)",0.0,0x27A,1},
	{"Напряжение вторичной обмотки трансформатора",0.0, 0x230,1},
	{"Время блокировки запуска после включения питания",0.0, 0x22D,1},
	{"Защита по Rиз",0.0, 0x240,1},
	{"Дисбаланс по U вх.   лин.",0.0, 0x229,1},
	{"Время срабатывания защиты по дисбалансу U вх.  лин.",0.0, 0x22B,1},
	{"Дисбаланс по  I  вых. фаз.",0.0, 0x23B,1},
	{"Время срабатывания защиты по дисбалансу I  вых. фаз",0.0, 0x23D,1},
	{"Предельная температура ВД ",0.0, 0x24B,1},
	{"Давление жидкости на приеме насоса ",0.0, 0x246,1},
};

#define NUMBER_OF_SETTINGS (sizeof(settings) / sizeof(setting_t))

CModbusDaemon::CModbusDaemon(std::string processName, int argc, char* argv[])
	: CDaemonProcess(processName, argc, argv), m_tcpPort(DEFAULT_TCP_PORT)

{
	m_configOptions["ParametersDB"] = "";
	m_configOptions["EventsDB"] = "";
	m_configOptions["MODBUS_LocalAddress"] = "127.0.0.1:502";
	m_configOptions["MODBUS_Rtu_Baudrate"] = "9600";
	m_configOptions["MODBUS_Rtu_Port"] = "";

	m_pLoop = new CModbusLoop();
	m_pPump = new CDataPump(parameters, NUMBER_OF_PARAMETERS, settings, NUMBER_OF_SETTINGS);
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

	Log("---- ENV ----");
	Log("ParamsDB name: %s", m_sParamDbName.c_str());
	Log("EventsDB name: %s", m_sEventDbName.c_str());
	Log("IP address: %s", m_tcpAddr.c_str());
	Log("TCP port: %d", m_tcpPort);
	Log("-------------");
	return true;
}


int CModbusDaemon::daemonLoop()
{
	Log("daemonLoop() -->>");
	if(m_pPump->Create(m_sParamDbName, m_sEventDbName)) {
		m_pPump->RegisterDataUpdateListener(m_pLoop);
		if(m_pLoop->Create(m_tcpAddr, m_tcpPort)) {

			//command loop goes here

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
//	m_pIpcConnection->write((const unsigned char*)"Hello!",6);
//	m_pIpcConnection->write((const unsigned char*)"Hello1!",7);
//	m_pIpcConnection->close();
	CIPCCommand* pCommand = new CIPCCommand(*m_pIpcConnection);

	pCommand->setPayload((unsigned char*)"hello", 5);
	pCommand->send();

	pCommand->waitForReply();

	Log("parentLoop() --<<");
	return 0;
}
