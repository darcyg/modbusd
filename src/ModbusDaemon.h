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

class CModbusDaemon: public CDaemonProcess {

protected:
	CModbusLoop* m_pLoop;
	CDataPump* m_pPump;
	std::string m_sParamDbName;
	std::string m_sEventDbName;
	int m_tcpPort;
	std::string m_tcpAddr;
protected:
	virtual int daemonLoop();
	virtual int parentLoop();
	virtual bool setupEnvironment();
public:
	CModbusDaemon(std::string processName, int argc, char* argv[]);
	virtual ~CModbusDaemon();

};

#endif /* MODBUSDAEMON_H_ */
