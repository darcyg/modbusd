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

#include <sys/socket.h>
#include <sys/epoll.h>

class IIPCServerListener {
public:
	virtual void IPCServerOnNewConnection(CIPCConnection* pConnection) = 0;
protected:
	virtual ~IIPCServerListener() {};
};

class CIPCServer: public CThread {
private:
	std::string m_socketName;
	// socket for accepting connections
	int m_socket;

	IIPCServerListener* m_pListener;

	bool m_canAcceptConnections;

	int m_efd;
	struct epoll_event *m_events;
private:
	int make_socket_non_blocking(int sfd);
	int epoll_add_socket(int fd, CIPCConnection* pConnection);

protected:
	// list of active connections
	std::list<CIPCConnection*> m_connections;
	virtual void* Run();
public:
	CIPCServer(std::string socket, IIPCServerListener* pListener);
	virtual ~CIPCServer();
	bool Create();
	bool Start();
	int getSocketId() { return m_socket; }
};

#endif /* IPCSERVER_H_ */
