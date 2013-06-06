/*
 * CModbusLoop.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "ModbusLoop.h"
#include "Log.h"

#include <unistd.h>
#include <stdio.h>

#define MB_FUNCTION_READ_HOLDING 0x3
#define MB_FUNCTION_READ_INPUT   0x4
#define MB_FUNCTION_WRITE_HOLDING 0x6
#define MB_FUNCTION_WRITE_HOLDING_MULTIPLE 0x10

CModbusLoop::CModbusLoop(data_parameter_t* params, int nbParams, setting_t* settings, int nbSettings)
	: m_ctx(NULL), m_mapping1(NULL), m_mapping2(NULL), m_mapping(NULL), m_headerLength(0), m_socket(-1),
	  m_params(params), m_settings(settings), m_nbParams(nbParams), m_nbSettings(nbSettings)
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

	Log("[MODBUS] listen()");
    m_socket = modbus_tcp_listen(m_ctx, 1);

    if(m_socket < 0) {
    	Log("[MODBUS] listen() failed");
    	return false;
    }

    m_mapping1 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB, INPUT_REGS_ADDR + INPUT_REGS_NB);
    m_mapping2 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB, INPUT_REGS_ADDR + INPUT_REGS_NB);

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

inline uint8_t CModbusLoop::getFunction() {
	return m_query[m_headerLength];
}

inline uint16_t CModbusLoop::getStartAddress() {
	return MODBUS_GET_INT16_FROM_INT8(m_query, m_headerLength + 1);
}

inline uint16_t CModbusLoop::getValueF6() {
	return MODBUS_GET_INT16_FROM_INT8(m_query, m_headerLength + 3);
}


inline uint16_t CModbusLoop::getRegisterCountF10() {
	return MODBUS_GET_INT16_FROM_INT8(m_query, m_headerLength + 3);
}

#define getRegisterCountF3() getRegisterCountF10()
#define getRegisterCountF4() getRegisterCountF10()

inline uint16_t CModbusLoop::getByteCountF10() {
	return m_query[m_headerLength + 5];
}

bool CModbusLoop::isValidHoldingReg(uint16_t addr, int count) {
	int regsFound = 0;

	if(count > m_nbSettings)
		return false;

	for(int i = 0; i < m_nbSettings || count > regsFound; i++) {
		if((m_settings[i].m_startReg >= addr) && (m_settings[i].m_startReg < (addr + count))) {
			regsFound++;
		}
	}
	return regsFound == count;
}
bool CModbusLoop::isValidInputReg(uint16_t addr, int count) {

	int regsFound = 0;

	if(count > m_nbParams)
		return false;

	for(int i = 0; i < m_nbParams || count > regsFound; i++) {
		if((m_params[i].m_startReg >= addr) && (m_params[i].m_startReg < (addr + count))) {
			regsFound++;
		}
	}
	return regsFound == count;
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

			uint8_t function;
			uint16_t address;

			do {
				rc = modbus_receive(m_ctx, m_query);
				Log("Got query: length=%d", rc);
				/* Filtered queries return 0 */
			} while (rc == 0);

			if (rc == -1) {
				/* Connection closed by the client or error */
				break;
			}

			function = getFunction();
			address = getStartAddress();

			Log("Got query: length=%d FUNCTION: 0x%X", rc, function);

			for(int i = 0; i < rc; i++) {
				printf("0x%x ", m_query[i]);
			}
			printf("\n");

			switch (function) {
			case MB_FUNCTION_WRITE_HOLDING:
				Log("MB_FUNCTION_WRITE_HOLDING: addr=0x%X value=0x%X", address, getValueF6());
				if(!isValidHoldingReg(address,1)) {
					rc = modbus_reply_exception(m_ctx, m_query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				}
				//TODO: do actual write and replay
				break;

			case MB_FUNCTION_WRITE_HOLDING_MULTIPLE:
				Log("MB_FUNCTION_WRITE_HOLDING_MULTIPLE: addr=0x%X n_reg=%d n_byte=%d", address, getRegisterCountF10(), getByteCountF10());
				if(!isValidHoldingReg(address,1)) {
					rc = modbus_reply_exception(m_ctx, m_query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				}
				//TODO: do actual write and replay
				break;

			// F3 read multiple regs. settings
			case MB_FUNCTION_READ_HOLDING:
				Log("MB_FUNCTION_READ_HOLDING: addr=0x%X count=%d", address, getRegisterCountF3());
				if(!isValidHoldingReg(address, getRegisterCountF3())) {
					rc = modbus_reply_exception(m_ctx, m_query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				} else {
					pthread_mutex_lock(&m_mutex);
					rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
					pthread_mutex_unlock(&m_mutex);
				}
				break;

			// F4 read multiple regs. params
			case MB_FUNCTION_READ_INPUT:
				Log("MB_FUNCTION_READ_INPUT: addr=0x%X count=%d", address, getRegisterCountF4());
				if(!isValidInputReg(address, getRegisterCountF4())) {
					rc = modbus_reply_exception(m_ctx, m_query, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				} else {
					pthread_mutex_lock(&m_mutex);
					rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
					pthread_mutex_unlock(&m_mutex);
				}
				break;

			default:
				{
					Log("UNSUPPORTED FUNCTION: 0x%X", function);
					rc = modbus_reply_exception(m_ctx, m_query, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
					break;
				}
			}

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
		pm->tab_input_registers[params[i].m_startReg] = params[i].m_value;
	}

	for(int i = 0; i < nbSettings; i++) {
		pm->tab_registers[settings[i].m_startReg] = settings[i].m_value;
	}


	pthread_mutex_lock(&m_mutex);
	m_mapping = pm;
	pthread_mutex_unlock(&m_mutex);
}



