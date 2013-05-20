/*
 * CModbusLoop.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef CMODBUSLOOP_H_
#define CMODBUSLOOP_H_

#include "Thread.h"
#include "DataPump.h"
#include <modbus.h>

#define HOLDING_REGS_ADDR 0x100
#define HOLDING_REGS_NB 0x60

class CModbusLoop: public CThread, public IOnDataUpdateListener {
protected:
    modbus_t* m_ctx;
    modbus_mapping_t *m_mapping1;
    modbus_mapping_t *m_mapping2;
    modbus_mapping_t *m_mapping;

    int m_headerLength;
    int m_socket;
    uint8_t *m_query;

	pthread_mutex_t m_mutex;
public:
	CModbusLoop();
	virtual ~CModbusLoop();
	virtual void* Run();
	virtual bool Create(std::string addr, int port);

public:
	virtual void OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_t* settings, int nbSettings);
};

#endif /* CMODBUSLOOP_H_ */
