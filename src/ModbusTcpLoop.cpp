/*
 * ModbusTcpLoop.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: ruinmmal
 */

#include "ModbusTcpLoop.h"
#include <modbus.h>
#include "Log.h"
#include <unistd.h>

CModbusTcpLoop::CModbusTcpLoop(data_parameter_t* params, int nbParams, setting_t* settings, int nbSettings, std::string ritexPath,
		std::string addr, int port) :
		CModbusLoop(params, nbParams, settings, nbSettings, ritexPath), m_port(port), m_addr(addr), m_socket(-1)
{
	m_query = new uint8_t[MODBUS_TCP_MAX_ADU_LENGTH];
}

CModbusTcpLoop::~CModbusTcpLoop() {
	if (m_socket != -1) {
		::close(m_socket);
		m_socket = -1;
	}
}

bool CModbusTcpLoop::Create() {
	m_ctx = modbus_new_tcp(m_addr.c_str(), m_port);

	if (m_ctx == NULL) {
		Log("Unable to allocate libmodbus context");
		return false;
	}

	m_headerLength = modbus_get_header_length(m_ctx);

	Log("[MODBUS] listen()");
	m_socket = modbus_tcp_listen(m_ctx, 1);

	if (m_socket < 0) {
		Log("[MODBUS] listen() failed");
		return false;
	}
	return CModbusLoop::Create();
}

int CModbusTcpLoop::AcceptModbusConnection() {
	return modbus_tcp_accept(m_ctx, &m_socket);
}


