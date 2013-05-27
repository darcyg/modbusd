/*
 * IPCConnection.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "IPCConnection.h"
#include "Utils.h"
//socket
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Log.h"

CIPCConnection::CIPCConnection(std::string socket)
	: m_socketName(socket), m_socket(-1)
{
	// TODO Auto-generated constructor stub

}

CIPCConnection::~CIPCConnection() {
	if(m_socket != -1) {
		::close(m_socket);
	}
}

bool CIPCConnection::connect() {
	struct sockaddr_un addr;

	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = 500 * 1000 * 1000;
	int connectResult;

	m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if(m_socket == -1)
		return false;

	// create abstract socket name
	::memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	::strncpy(&addr.sun_path[1], m_socketName.c_str(), sizeof(addr.sun_path) - 2);

	// wait for 50 sec at most
	for(int i = 0; i < 100; i++) {
		connectResult = ::connect(m_socket, (struct sockaddr *) &addr, _STRUCT_OFFSET (struct sockaddr_un, sun_path) + m_socketName.length() + 1);
		if (connectResult == -1) {
			::nanosleep(&tim, NULL);
		} else {
			break;
		}
	}

	if (connectResult == -1) {
		::close(m_socket);
		return false;
	}
	return true;
}


int CIPCConnection::read(unsigned char* buffer, int size) {
	return 0;
}

int CIPCConnection::write(unsigned char* buffer, int size) {
	return 0;
}


