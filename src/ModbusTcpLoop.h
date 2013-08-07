/*
 * ModbusTcpLoop.h
 *
 *  Created on: Jun 10, 2013
 *      Author: ruinmmal
 */

#ifndef MODBUSTCPLOOP_H_
#define MODBUSTCPLOOP_H_

#include "ModbusLoop.h"
#include <string>

class CModbusTcpLoop: public CModbusLoop {
protected:
	int m_port;
	std::string m_addr;
    int m_socket;
protected:
    virtual int AcceptModbusConnection();
public:
	CModbusTcpLoop(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings, std::string ritexPath,
			std::string addr, int port);
	virtual ~CModbusTcpLoop();
	virtual bool Create();
};

#endif /* MODBUSTCPLOOP_H_ */
