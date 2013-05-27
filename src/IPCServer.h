/*
 * IPCServer.h
 *
 *  Created on: May 26, 2013
 *      Author: ruinmmal
 */

#ifndef IPCSERVER_H_
#define IPCSERVER_H_

#include "Thread.h"
#include "IPCConnection.h"
#include <list>

class IIPCServerListener {
public:
	virtual void IPCServerOnNewConnection(CIPCConnection* pConnection) = 0;
protected:
	virtual ~IIPCServerListener();
};

class CIPCServer: public CThread {
private:
	std::string m_socketName;
	// socket for accepting connections
	int m_socket;

	std::list<IIPCServerListener*> m_pListeners;

protected:
	// list of active connections
	std::list<CIPCConnection*> m_connections;
	virtual void* Run();
public:
	CIPCServer(std::string socket);
	virtual ~CIPCServer();
	bool Create();
};

#endif /* IPCSERVER_H_ */
