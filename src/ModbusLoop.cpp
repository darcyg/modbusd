/*
 * CModbusLoop.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ModbusLoop.h"
#include "Log.h"

#include <unistd.h>

CModbusLoop::CModbusLoop()
	: m_ctx(NULL), m_mapping1(NULL), m_mapping2(NULL), m_mapping(NULL), m_headerLength(0), m_socket(-1)
{
    m_query = new uint8_t[MODBUS_TCP_MAX_ADU_LENGTH];
}

CModbusLoop::~CModbusLoop() {
	if(m_socket != -1) {
		::close(m_socket);
		m_socket = -1;
	}

	if(m_ctx) {
		modbus_close(m_ctx);
		modbus_free(m_ctx);
		m_ctx = NULL;
	}

	if(m_mapping1) {
		modbus_mapping_free(m_mapping1);
		m_mapping1 = NULL;
	}

	if(m_mapping2) {
		modbus_mapping_free(m_mapping2);
		m_mapping2 = NULL;
	}

	if(m_query) {
		delete [] m_query;
		m_query = NULL;
	}

	pthread_mutex_destroy(&m_mutex);
}


bool CModbusLoop::Create(std::string addr, int port)
{
	m_ctx = modbus_new_tcp(addr.c_str(), port);

	if(m_ctx == NULL) {
		Log("Unable to allocate libmodbus context");
		return false;
	}

    modbus_set_debug(m_ctx, TRUE);

    m_headerLength = modbus_get_header_length(m_ctx);
    modbus_set_debug(m_ctx, TRUE);

	Log("[MODBUS] listen()");
    m_socket = modbus_tcp_listen(m_ctx, 1);

    if(m_socket < 0) {
    	Log("[MODBUS] listen() failed");
    	return false;
    }

    m_mapping1 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB, 0);
    m_mapping2 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB, 0);

    m_mapping = m_mapping1;

#if 0
    for(int i = 0; i < HOLDING_REGS_NB; i++) {
    	m_mapping->tab_registers[HOLDING_REGS_ADDR+i] = 0xAAAA + i;
    }
#endif

    if(m_mapping1 == NULL || m_mapping2 == NULL) {
    	Log("[MODBUS] modbus_mapping_new() failed");
    	return false;
    }

	pthread_mutex_init(&m_mutex, NULL);


	return CThread::Create();
}



void* CModbusLoop::Run()
{
	Log("[MODBUS] Run() -->>");

	while (true) {
		Log("[MODBUS] accept(1)");
		modbus_tcp_accept(m_ctx, &m_socket);
		Log("[MODBUS] accept(2)");

		for(;;) {
			int rc;
			do {
				rc = modbus_receive(m_ctx, m_query);
				Log("Got query: length=%d", rc);
				/* Filtered queries return 0 */
			} while (rc == 0);

			if (rc == -1) {
				/* Connection closed by the client or error */
				break;
			}

			Log("Got query: length=%d", rc);

			pthread_mutex_lock(&m_mutex);
			rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
			pthread_mutex_unlock(&m_mutex);

			if (rc == -1) {
				break;
			}
		}
	}
	Log("[MODBUS] Run() --<<");

	return NULL;
}

void CModbusLoop::OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_t* settings, int nbSettings)
{
	Log("OnDataUpdated(): nbParams=%d nbSettings=%d", nbParams, nbSettings);

	modbus_mapping_t * pm = m_mapping == m_mapping1 ? m_mapping2 : m_mapping1;

	for(int i = 0; i < nbParams; i++) {
		pm->tab_registers[params[i].m_startReg] = params[i].m_value;
	}

	for(int i = 0; i < nbSettings; i++) {
		pm->tab_input_registers[settings[i].m_startReg] = settings[i].m_value;
	}


	pthread_mutex_lock(&m_mutex);
	m_mapping = pm;
	pthread_mutex_unlock(&m_mutex);
}



