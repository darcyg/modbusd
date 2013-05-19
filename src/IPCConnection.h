/*
 * IPCConnection.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef IPCCONNECTION_H_
#define IPCCONNECTION_H_

#include <string>

class CIPCConnection {
	std::string m_socketName;
	int m_socket;
public:
	CIPCConnection(std::string socket);
	virtual ~CIPCConnection();
	bool create();
	bool connect();
	int read(unsigned char* buffer, int size);
	int write(unsigned char* buffer, int size);
};

#endif /* IPCCONNECTION_H_ */
