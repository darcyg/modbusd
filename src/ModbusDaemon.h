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
class CModbusDaemon: public CDaemonProcess {

protected:
	std::map<std::string, std::string> m_configOptions;
	CModbusLoop* m_pLoop;
protected:
	virtual int daemonLoop();
	virtual int parentLoop();
	virtual bool setupEnvironment();
public:
	CModbusDaemon(std::string processName, int argc, char* argv[]);
	virtual ~CModbusDaemon();

public:
	// from
	virtual bool OnConfigOption(std::string& name, std::string& value);
};

#endif /* MODBUSDAEMON_H_ */
