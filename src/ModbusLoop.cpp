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
#include <vector>
#include "Utils.h"
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#undef CHECK_REGISTER_ADDRESS

#define MB_FUNCTION_READ_HOLDING 0x3
#define MB_FUNCTION_READ_INPUT   0x4
#define MB_FUNCTION_WRITE_HOLDING 0x6
#define MB_FUNCTION_WRITE_HOLDING_MULTIPLE 0x10

CModbusLoop::CModbusLoop(data_parameter_t* params, int nbParams,
		setting_t* settings, int nbSettings, std::string ritexPath) :
		m_ctx(NULL), m_mapping1(NULL), m_mapping2(NULL), m_mapping(NULL), m_headerLength(
				0), m_query(NULL), m_params(params), m_settings(settings), m_nbParams(
				nbParams), m_nbSettings(nbSettings), m_sRitexPath(ritexPath) {

}

CModbusLoop::~CModbusLoop() {


	if (m_ctx) {
		modbus_close(m_ctx);
		modbus_free(m_ctx);
		m_ctx = NULL;
	}

	if (m_mapping1) {
		modbus_mapping_free(m_mapping1);
		m_mapping1 = NULL;
	}

	if (m_mapping2) {
		modbus_mapping_free(m_mapping2);
		m_mapping2 = NULL;
	}

	if (m_query) {
		delete[] m_query;
		m_query = NULL;
	}

	pthread_mutex_destroy(&m_mutex);
}

bool CModbusLoop::Create() {


	m_mapping1 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB,
			INPUT_REGS_ADDR + INPUT_REGS_NB);
	m_mapping2 = modbus_mapping_new(0, 0, HOLDING_REGS_ADDR + HOLDING_REGS_NB,
			INPUT_REGS_ADDR + INPUT_REGS_NB);

	m_mapping = m_mapping1;

#if 0
	for(int i = 0; i < HOLDING_REGS_NB; i++) {
		m_mapping->tab_registers[HOLDING_REGS_ADDR+i] = 0xAAAA + i;
	}
#endif

	if (m_mapping1 == NULL || m_mapping2 == NULL) {
		Log("[MODBUS] modbus_mapping_new() failed");
		return false;
	}

	pthread_mutex_init(&m_mutex, NULL);

	modbus_set_debug(m_ctx, FALSE);

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

inline uint16_t* CModbusLoop::getValuesPtrF10() {
	return (uint16_t*)(m_query + m_headerLength + 7);
}

bool CModbusLoop::isValidHoldingReg(uint16_t addr, int count) {
#ifdef CHECK_REGISTER_ADDRESS
	int regsFound = 0;

	if (count > m_nbSettings)
		return false;

	for (int i = 0; i < m_nbSettings || count > regsFound; i++) {
		if ((m_settings[i].m_startReg >= addr)
				&& (m_settings[i].m_startReg < (addr + count))) {
			regsFound++;
		}
	}
	return regsFound == count;
#else
	return true;
#endif
}
bool CModbusLoop::isValidInputReg(uint16_t addr, int count) {
#ifdef CHECK_REGISTER_ADDRESS
	int regsFound = 0;

	if (count > m_nbParams)
		return false;

	for (int i = 0; i < m_nbParams || count > regsFound; i++) {
		if ((m_params[i].m_startReg >= addr)
				&& (m_params[i].m_startReg < (addr + count))) {
			regsFound++;
		}
	}
	return regsFound == count;
#else
	return true;
#endif
}

bool CModbusLoop::WriteSettingByAddress(uint16_t addr, uint16_t value) {
	int id = -1;
	pid_t childPid;
	for (int i = 0; i < m_nbSettings; i++) {
		if (m_settings[i].m_startReg == addr) {
			id = m_settings[i].m_id;
			break;
		}
	}

	if(id == -1)
		return false;

	//prepare command line
	int maxLen = m_sRitexPath.length() + 2 * 12;
	char* tmpBuf = new char[maxLen];

	if (tmpBuf == NULL)
		return false;

	snprintf(tmpBuf, maxLen, m_sRitexPath.c_str(), id, value);

	printf("Command line for exec(): [%s]\n", tmpBuf);

	std::vector<std::string> parts = split(std::string(tmpBuf), ' ');

	delete[] tmpBuf;

	int argc = parts.size();

	if (argc == 0) {
		return false;
	}
	//now create array of parameters
	char** argv = new char*[argc + 1];
	if (argv == NULL) {
		return false;
	}

	//put NULL in the last element
	argv[argc] = NULL;

	for (int i = 0; i < argc; i++) {
		int argLen = parts[i].length() + 1;
		argv[i] = new char[argLen];
		strncpy(argv[i], parts[i].c_str(), argLen);
	}

	int pipefd[2];
	pipe(pipefd);

	childPid = ::fork();
	if (childPid == 0) {
	    close(pipefd[0]);    // close reading end in the child

	    dup2(pipefd[1], 1);  // send stdout to the pipe
	    dup2(pipefd[1], 2);  // send stderr to the pipe

	    //close(pipefd[1]);    // this descriptor is no longer needed

		execvp(parts[0].c_str(), argv);
		exit(0);
	} else {
		int b = 0, rc = 0;
	    char buffer[1024];
	    close(pipefd[1]);  // close the write end of the pipe in the parent

	    do {
	    	rc = read(pipefd[0], buffer + b, sizeof(buffer) - b);
	    	b += rc;
	    } while (rc != 0 || b == 1023);
	    buffer[b] = '\0';
	    close(pipefd[0]);  // close the write end of the pipe in the parent

	    printf("[CHILD RETURNED] %s\n", buffer);

		int child_status;
		waitpid(childPid, &child_status, 0);

		//parse response
		if(buffer[0] == '8')
			return true;
		else
			return false;
	}
	return true;
}

void* CModbusLoop::Run() {
	Log("[MODBUS] Run() -->>");
	int rc;

	while (true) {
		Log("[MODBUS] accept(1)");
		rc = AcceptModbusConnection();
		Log("[MODBUS] accept(2) %d", rc);

		if(rc < 0) {
			Log("[MODBUS] error accepting connection", rc);
			break;
		}

		for (;;) {

			uint8_t function;
			uint16_t address;

			do {
				rc = modbus_receive(m_ctx, m_query);
				Log("Got query: length=%d", rc);
				/* Filtered queries return 0 */
			} while (rc == 0);

			if (rc == -1) {
				/* Connection closed by the client or error */
				modbus_close(m_ctx);
				break;
			}

			function = getFunction();
			address = getStartAddress();

			Log("Got query: length=%d FUNCTION: 0x%X", rc, function);

			for (int i = 0; i < rc; i++) {
				printf("0x%x ", m_query[i]);
			}
			printf("\n");

			switch (function) {
			case MB_FUNCTION_WRITE_HOLDING:
				Log("MB_FUNCTION_WRITE_HOLDING: addr=0x%X value=0x%X", address,
						getValueF6());
				if (!isValidHoldingReg(address, 1)) {
					rc = modbus_reply_exception(m_ctx, m_query,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				}
				if(!WriteSettingByAddress(address, getValueF6())) {
					rc = modbus_reply_exception(m_ctx, m_query,
							MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
				} else {
					pthread_mutex_lock(&m_mutex);
					rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
					pthread_mutex_unlock(&m_mutex);
				}
				break;

			case MB_FUNCTION_WRITE_HOLDING_MULTIPLE:
				{
					bool isException = false;
					Log(
							"MB_FUNCTION_WRITE_HOLDING_MULTIPLE: addr=0x%X n_reg=%d n_byte=%d",
							address, getRegisterCountF10(), getByteCountF10());

					if (!isValidHoldingReg(address, getRegisterCountF10())) {
						rc = modbus_reply_exception(m_ctx, m_query,
								MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
						Log("------------0");
						break;
					}

					Log("------------1");
					for(int i = 0; i < getRegisterCountF10(); i++) {
						Log("------------2");

						uint16_t value = getValuesPtrF10()[i];
						Log("------------3");

						if(!WriteSettingByAddress(address + i, value)) {
							Log("------------4");
							rc = modbus_reply_exception(m_ctx, m_query,
									MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
							isException = true;
							break;
						}
					}
					Log("------------5");

					if(!isException) {
						Log("------------6");

						pthread_mutex_lock(&m_mutex);
						rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
						pthread_mutex_unlock(&m_mutex);
					}
				}
				break;

				// F3 read multiple regs. settings
			case MB_FUNCTION_READ_HOLDING:
				Log("MB_FUNCTION_READ_HOLDING: addr=0x%X count=%d", address,
						getRegisterCountF3());
				if (!isValidHoldingReg(address, getRegisterCountF3())) {
					rc = modbus_reply_exception(m_ctx, m_query,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				} else {
					pthread_mutex_lock(&m_mutex);
					rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
					pthread_mutex_unlock(&m_mutex);
				}
				break;

				// F4 read multiple regs. params
			case MB_FUNCTION_READ_INPUT:
				Log("MB_FUNCTION_READ_INPUT: addr=0x%X count=%d", address,
						getRegisterCountF4());
				if (!isValidInputReg(address, getRegisterCountF4())) {
					rc = modbus_reply_exception(m_ctx, m_query,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
					break;
				} else {
					pthread_mutex_lock(&m_mutex);
					rc = modbus_reply(m_ctx, m_query, rc, m_mapping);
					pthread_mutex_unlock(&m_mutex);
				}
				break;

			default: {
				Log("UNSUPPORTED FUNCTION: 0x%X", function);
				rc = modbus_reply_exception(m_ctx, m_query,
						MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
				break;
			}
			}

			if (rc == -1) {
				modbus_close(m_ctx);
				break;
			}
		}
	}
	Log("[MODBUS] Run() --<<");

	return NULL;
}

void CModbusLoop::OnDataUpdated(const data_parameter_t* params, int nbParams,
		const setting_t* settings, int nbSettings) {
	Log("OnDataUpdated(): nbParams=%d nbSettings=%d", nbParams, nbSettings);

	modbus_mapping_t * pm = m_mapping == m_mapping1 ? m_mapping2 : m_mapping1;

	for (int i = 0; i < nbParams; i++) {
		pm->tab_input_registers[params[i].m_startReg] = params[i].m_value;
	}

	for (int i = 0; i < nbSettings; i++) {
		pm->tab_registers[settings[i].m_startReg] = settings[i].m_value;
	}

	pthread_mutex_lock(&m_mutex);
	m_mapping = pm;
	pthread_mutex_unlock(&m_mutex);
}

