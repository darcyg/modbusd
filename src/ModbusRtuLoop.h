/*
 * ModbusRtuLoop.h
 *
 *  Created on: Jun 10, 2013
 *      Author: ruinmmal
 */

#ifndef MODBUSRTULOOP_H_
#define MODBUSRTULOOP_H_

#include "ModbusLoop.h"

class CModbusRtuLoop: public CModbusLoop {
protected:
	std::string m_comPort;
	int m_speed;
	int m_bpb;
	char m_parity;
	int m_stopBits;
	int m_slaveId;
protected:
    virtual int AcceptModbusConnection();
public:
	CModbusRtuLoop(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings, std::string ritexPath,
			std::string comPort, int speed, int bpb, char parity, int stopBits, int slaveId);
	virtual ~CModbusRtuLoop();
	virtual bool Create();
};

#endif /* MODBUSRTULOOP_H_ */
