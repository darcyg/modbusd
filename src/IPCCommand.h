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
	CIPCConnection* m_connection;
protected:
	bool SendReply(unsigned char* data, int length);
public:
	CIPCCommand();
	virtual ~CIPCCommand();
};

#endif /* IPCCOMMAND_H_ */
