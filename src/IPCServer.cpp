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

//fcntl
#include <fcntl.h>

#include "Utils.h"

#include "Log.h"

#define MAXEVENTS 64

CIPCServer::CIPCServer(std::string socket, IIPCServerListener* pListener) :
		m_socketName(socket), m_socket(-1), m_pListener(pListener), m_canAcceptConnections(
				true), m_efd(-1), m_events(NULL) {
	// TODO Auto-generated constructor stub

}

CIPCServer::~CIPCServer() {
	// TODO Auto-generated destructor stub
}

int CIPCServer::make_socket_non_blocking(int sfd) {
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		Log("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		Log("fcntl");
		return -1;
	}

	return 0;
}

int CIPCServer::epoll_add_socket(int fd, CIPCConnection* pConnection) {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = reinterpret_cast<void*>(pConnection);
	return epoll_ctl(m_efd, EPOLL_CTL_ADD, fd, &event);
}

bool CIPCServer::Create() {
	struct sockaddr_un addr;

	m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_socket == -1) {
		Log("CIPCServer: socket() error");
		return false;
	}

	if (make_socket_non_blocking(m_socket) == -1) {
		Log("CIPCServer: make_socket_non_blocking error");
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
	return true;
}
bool CIPCServer::Start() {

	//epol_create1 is not declared for ARM targte
	m_efd = ::epoll_create(MAXEVENTS);

	if (m_efd == -1) {
		Log("CIPCServer epoll_create1() error");
		::close(m_socket);
		m_socket = -1;
		return false;
	}

	if (epoll_add_socket(m_socket, NULL) == -1) {
		Log("CIPCServer epoll_add_socket() error");
		::close(m_socket);
		::close(m_efd);
		m_efd = -1;
		m_socket = -1;
		return false;
	}

	m_events = new epoll_event[MAXEVENTS];

	if (m_events == NULL) {
		Log("CIPCServer: OOM allocating m_events");
		::close(m_socket);
		::close(m_efd);
		m_efd = -1;
		m_socket = -1;
		return false;
	}

	return CThread::Create();
}

void* CIPCServer::Run() {
	int n;

	Log("CIPCServer: Run() ->>");

	while (m_canAcceptConnections) {

		Log("CIPCServer: waiting for socket activity...");

		n = epoll_wait(m_efd, m_events, MAXEVENTS, -1);

		Log("CIPCServer: epoll n=%d", n);

		if (n == -1) {
			Log("CIPCServer: epoll error %d", errno);
			break;
		}

		for (int i = 0; i < n; i++) {
			Log("EVENT: ptr=%p event=0x%x", m_events[i].data.ptr,
					m_events[i].events);

			CIPCConnection* pConnection =
					reinterpret_cast<CIPCConnection*>(m_events[i].data.ptr);

			uint32_t events = m_events[i].events;

			// special case for 'accept' socket
			if (pConnection == NULL) {
				if (events & EPOLLIN) {
					events &= ~EPOLLIN;
					Log("CIPCServer: need accept connections ");
					do {
						int fd = ::accept(m_socket, NULL, NULL);

						if (fd == -1) {
							if (errno == EAGAIN || errno == EWOULDBLOCK) {
								//all connections accepted
								break;
							} else {
								//							m_canAcceptConnections = false;
								Log("CIPCServer: accept() error");
								break;
							}
						} else {
							Log("CIPCServer: incoming connection: fd=%d", fd);

							if (make_socket_non_blocking(fd) == -1) {
								::close(fd);
								break;
							} else {
								CIPCConnection* pConnection =
										new CIPCConnection(fd);

								if (pConnection == NULL) {
									Log(
											"CIPCServer: OOM creating new CIPCConnection");
									::close(fd);
									break;
								}
								m_pListener->IPCServerOnNewConnection(
										pConnection);
								epoll_add_socket(fd, pConnection);
							}
						}
					} while (true);
				}
				if (events != 0) {
					Log("[EPOLL][ACCEPT] ERROR: unhandled event: 0x%x", events);
				}
			} else {
				if (events & EPOLLIN) {
					events &= ~EPOLLIN;
					pConnection->NotifyEvent(CONN_EVENT_DATA_READY);
				}

				if (events & EPOLLHUP) {
					events &= ~EPOLLHUP;
					pConnection->NotifyEvent(CONN_EVENT_CONN_CLOSED);
				}

				if (events != 0) {
					Log("[EPOLL][DATA] ERROR: unhandled event: 0x%x", events);
				}
			}
		}
	}
	return NULL;
}
