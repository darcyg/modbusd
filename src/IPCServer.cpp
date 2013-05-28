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
#include <sys/ioctl.h>
#include <unistd.h>

//errno
#include <errno.h>

#include "Utils.h"

#include "Log.h"

CIPCServer::CIPCServer(std::string socket) :
		m_socket(-1), m_canAcceptConnections(true), m_maxSd(-1) {
	// TODO Auto-generated constructor stub

}

CIPCServer::~CIPCServer() {
	// TODO Auto-generated destructor stub
}

bool CIPCServer::Create() {
	int on = 1;
	struct sockaddr_un addr;
	//_do_backtrace();

	m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_socket == -1) {
		return false;
	}

	if (::ioctl(m_socket, FIONBIO, (char *) &on) < 0) {
		Log("CIPCServer: ioctl(FIONBIO) error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	strncpy(&addr.sun_path[1], m_socketName.c_str(), sizeof(addr.sun_path) - 2);

	Log("CIPCServer: Opening server socket.. [%s]", m_socketName.c_str());

	if (::bind(m_socket, (struct sockaddr *) &addr,
			_STRUCT_OFFSET (struct sockaddr_un, sun_path)
					+ m_socketName.length() + 1) == -1) {
		Log("CIPCServer: bind() error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	if (::listen(m_socket, 5) == -1) {
		Log("CIPCServer listen() error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	FD_ZERO(&m_master_set);
	m_maxSd = m_socket;
	FD_SET(m_socket, &m_master_set);

	return true;
}

void* CIPCServer::Run() {
	int rc, nb_of_descriptors_ready;
	// accept connection and call listener
	while (m_canAcceptConnections) {
		Log("CIPCServer: Waitign for client connection or data on socket...");

		memcpy(&m_working_set, &m_master_set, sizeof(m_master_set));

		rc = ::select(m_maxSd + 1, &m_working_set, NULL, NULL, NULL);

		if (rc < 0) {
			Log("CIPCServer: select() error");
			return NULL;
		}

		nb_of_descriptors_ready = rc;

		for (int i = 0; i <= m_maxSd && nb_of_descriptors_ready > 0; i++) {
			if (FD_ISSET(i, &m_working_set)) {

				nb_of_descriptors_ready--;

				// accept all connections on listen socket
				if (i == m_socket) {
					int clientSock = -1;

					do {
						clientSock = ::accept(m_socket, NULL, NULL);

						if (clientSock < 0) {
							if (errno != EWOULDBLOCK) {
								Log("CIPCServer: accept() error");
								m_canAcceptConnections = false;
							}
							break;
						}

						Log("CIPCServer: new incoming connection fd=%d", clientSock);
						FD_SET(clientSock, &m_master_set);
						if (clientSock > m_maxSd)
							m_maxSd = clientSock;

					} while (clientSock != -1);

				} else {
					// find a connection and notify
				}
			}
		}

	}
	return NULL;
}

