/*
 * IPCCommand.cpp
 *
 *  Created on: May 26, 2013
 *      Author: ruinmmal
 */

#include "IPCCommand.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
	int length;
	int id;
	int status;
	char data[0];
} cmd_hdr_t;

CIPCCommand::CIPCCommand(CIPCConnection& connection)
	: m_connection(connection), m_pPayload(NULL), m_payloadLength(0)

{

}

CIPCCommand::~CIPCCommand() {
	if(m_pPayload) {
		delete [] m_pPayload;
	}
}

bool CIPCCommand::send() {
	if(m_payloadLength == 0 || m_pPayload == NULL)
		return false;
	return (m_connection.write(m_pPayload, m_payloadLength) == m_payloadLength);
}

bool CIPCCommand::setPayload(unsigned char* data, int length, int status) {
	if(data == NULL || length == 0) {
		return false;
	}

	m_pPayload = new unsigned char[length + sizeof(cmd_hdr_t)];

	if(m_pPayload == NULL)
		return false;

	cmd_hdr_t* pCmd = (cmd_hdr_t*)m_pPayload;

	pCmd->length = length;
	pCmd->id = rand();
	pCmd->status = status;

	m_payloadLength = length + sizeof(cmd_hdr_t);

	memcpy(&pCmd->data[0],  data, length);
	return true;
}

