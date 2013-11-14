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

#define HOLDING_REGS_ADDR 0x0
#define HOLDING_REGS_NB 0x910

#define INPUT_REGS_ADDR 0x0
#define INPUT_REGS_NB 0x910


class CModbusLoop: public CThread, public IOnDataUpdateListener {
protected:
    modbus_t* m_ctx;
//    modbus_mapping_t *m_mapping1;
//    modbus_mapping_t *m_mapping2;
    modbus_mapping_t *m_mapping;

    int m_headerLength;
    uint8_t *m_query;

	data_parameter_t* m_params;
	setting_m_t* m_settings;
	int m_nbParams;
	int m_nbSettings;

	std::string m_sRitexPath;
	std::string m_sRitexPathEngine;

	pthread_mutex_t m_mutex;
	bool m_isDataAvailable;

	uint8_t getFunction();
	uint16_t getStartAddress();
	uint16_t getValueF6();
	uint16_t getRegisterCountF10();
	uint16_t getByteCountF10();
	uint16_t* getValuesPtrF10();

	bool isValidHoldingReg(uint16_t addr, int count);
	bool isValidInputReg(uint16_t addr, int count);
	bool WriteSettingByAddress(uint16_t addr, uint16_t value);
	bool ChangeEngineStatus(bool isOn);

	virtual int AcceptModbusConnection() = 0;
public:
	CModbusLoop(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings, std::string ritexPath);
	virtual ~CModbusLoop();
	virtual void* Run();
	virtual bool Create();

public:
	virtual void OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_m_t* settings, int nbSettings);
	virtual void OnDataAvailable(bool isAvailable);
};

#endif /* CMODBUSLOOP_H_ */
