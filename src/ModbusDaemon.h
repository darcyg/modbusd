/*
 * ModbusDaemon.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef MODBUSDAEMON_H_
#define MODBUSDAEMON_H_

#include "DaemonProcess.h"
#include <map>
#include "ModbusLoop.h"
#include "DataPump.h"

#define DEFAULT_TCP_PORT 502

#define MODBUS_MODE_NONE -1
#define MODBUS_MODE_RTU   0
#define MODBUS_MODE_TCP   1

class CModbusDaemon: public CDaemonProcess {

protected:
	CModbusLoop* m_pLoop;
	CDataPump* m_pPump;
	std::string m_sParamDbName;
	std::string m_sEventDbName;
	std::string m_sRitexPath;
	int m_tcpPort;
	std::string m_tcpAddr;
	int m_modbusMode;
protected:
	virtual int daemonLoop();
	virtual int parentLoop();
	virtual bool setupEnvironment();
public:
	CModbusDaemon(std::string processName, int argc, char* argv[]);
	virtual ~CModbusDaemon();

};

#endif /* MODBUSDAEMON_H_ */
