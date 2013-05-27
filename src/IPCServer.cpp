/*
 * IPCServer.cpp
 *
 *  Created on: May 26, 2013
 *      Author: ruinmmal
 */

#include "IPCServer.h"

//socket
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Utils.h"

#include "Log.h"

CIPCServer::CIPCServer(std::string socket) {
	// TODO Auto-generated constructor stub

}

CIPCServer::~CIPCServer() {
	// TODO Auto-generated destructor stub
}

bool CIPCServer::Create() {
	struct sockaddr_un addr;
	_do_backtrace();

	m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_socket == -1) {
		return false;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	strncpy(&addr.sun_path[1], m_socketName.c_str(), sizeof(addr.sun_path) - 2);

	Log( "CIPCConnection: Opening server socket.. [%s]", m_socketName.c_str());

	if (::bind(m_socket, (struct sockaddr *) &addr, _STRUCT_OFFSET (struct sockaddr_un, sun_path) + m_socketName.length() + 1) == -1) {
		Log( "CIPCConnection: bind() error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	if (::listen(m_socket, 5) == -1) {
		Log( "CIPCConnection listen() error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	return true;
}

void* CIPCServer::Run() {
	// accept connection and call listener
	return NULL;
}


