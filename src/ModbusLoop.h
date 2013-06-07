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
#define HOLDING_REGS_NB 0x300

#define INPUT_REGS_ADDR 0x100
#define INPUT_REGS_NB 0x300


class CModbusLoop: public CThread, public IOnDataUpdateListener {
protected:
    modbus_t* m_ctx;
    modbus_mapping_t *m_mapping1;
    modbus_mapping_t *m_mapping2;
    modbus_mapping_t *m_mapping;

    int m_headerLength;
    int m_socket;
    uint8_t *m_query;

	data_parameter_t* m_params;
	setting_t* m_settings;
	int m_nbParams;
	int m_nbSettings;

	std::string m_sRitexPath;

	pthread_mutex_t m_mutex;

	uint8_t getFunction();
	uint16_t getStartAddress();
	uint16_t getValueF6();
	uint16_t getRegisterCountF10();
	uint16_t getByteCountF10();
	uint16_t* getValuesPtrF10();

	bool isValidHoldingReg(uint16_t addr, int count);
	bool isValidInputReg(uint16_t addr, int count);
	bool WriteSettingByAddress(uint16_t addr, uint16_t value);
public:
	CModbusLoop(data_parameter_t* params, int nbParams, setting_t* settings, int nbSettings, std::string ritexPath);
	virtual ~CModbusLoop();
	virtual void* Run();
	virtual bool Create(std::string addr, int port);

public:
	virtual void OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_t* settings, int nbSettings);
};

#endif /* CMODBUSLOOP_H_ */
