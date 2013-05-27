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
	bool connect();
	int getSocketId() const { return m_socket; }
	int read(unsigned char* buffer, int size);
	int write(unsigned char* buffer, int size);
};

#endif /* IPCCONNECTION_H_ */
