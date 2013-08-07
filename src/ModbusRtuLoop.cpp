/*
 * ModbusRtuLoop.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: ruinmmal
 */

#include "ModbusRtuLoop.h"
#include "Log.h"

CModbusRtuLoop::CModbusRtuLoop(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings, std::string ritexPath,
		std::string comPort, int speed, int bpb, char parity, int stopBits, int slaveId)
	: CModbusLoop(params, nbParams, settings, nbSettings, ritexPath), m_comPort(comPort), m_speed(speed), m_bpb(bpb),
	  m_parity(parity), m_stopBits(stopBits), m_slaveId(slaveId)
{
	m_query = new uint8_t[MODBUS_RTU_MAX_ADU_LENGTH];
}

CModbusRtuLoop::~CModbusRtuLoop() {

}

int CModbusRtuLoop::AcceptModbusConnection() {
	return modbus_connect(m_ctx);
}

bool CModbusRtuLoop::Create() {
	m_ctx = modbus_new_rtu(m_comPort.c_str(), m_speed, m_parity, m_bpb, m_stopBits);

	if (m_ctx == NULL) {
		Log("Unable to allocate libmodbus context");
		return false;
	}
	modbus_set_slave(m_ctx, m_slaveId);

	modbus_rtu_set_serial_mode(m_ctx, MODBUS_RTU_RS232);

	m_headerLength = modbus_get_header_length(m_ctx);


	return CModbusLoop::Create();
}

