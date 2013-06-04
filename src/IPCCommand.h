/*
 * IPCCommand.h
 *
 *  Created on: May 26, 2013
 *      Author: ruinmmal
 */

#ifndef IPCCOMMAND_H_
#define IPCCOMMAND_H_

#include "IPCConnection.h"

class CIPCCommand {
private:
	CIPCConnection& m_connection;
	unsigned char* m_pPayload;
	int m_payloadLength;
protected:
	bool SendReply(unsigned char* data, int length);
public:
	CIPCCommand(CIPCConnection& connection);
	virtual ~CIPCCommand();
	bool send();
	bool setPayload(unsigned char* data, int length, int status = 0);
	bool waitForReply() { return true; }
};

#endif /* IPCCOMMAND_H_ */
