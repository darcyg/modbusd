/*
 * IPCConnection.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef IPCCONNECTION_H_
#define IPCCONNECTION_H_

#include <string>

typedef enum {
	CONN_EVENT_DATA_READY,
	CONN_EVENT_CONN_CLOSED,
} EConnectionEvent;

class CIPCConnection;

class IIPCConnectionEventListener {
public:
	virtual void OnIPCConnectionClosed(CIPCConnection* pConnection) = 0;
	virtual void OnIPCConnectionDataReady(CIPCConnection* pConnection) = 0;
protected:
	virtual ~IIPCConnectionEventListener() {}
};

class CIPCConnection {
	friend class CIPCServer;
private:
	std::string m_socketName;
	int m_socket;
	IIPCConnectionEventListener* m_pListener;
	bool m_isClosed;
private:
	void NotifyEvent(EConnectionEvent event);
	CIPCConnection(int socket);
public:
	CIPCConnection(std::string socket);
	virtual ~CIPCConnection();
	bool connect();
	int getSocketId() const { return m_socket; }
	int read(unsigned char* buffer, int size);
	int write(const unsigned char* buffer, int size);
	int close();
	void setListener(IIPCConnectionEventListener* pListener) {
		m_pListener = pListener;
	}
};

#endif /* IPCCONNECTION_H_ */
