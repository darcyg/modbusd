/*
 * DaemonProcess.cpp
 *
 *  Created on: May 18, 2013
 *      Author: ruinmmal
 */

#include "DaemonProcess.h"
#include "ConfigFile.h"
#include "Log.h"

// fork
#include <unistd.h>
//socket
#include <sys/un.h>
#include <sys/socket.h>

//errno
#include <errno.h>

//unmask
#include <sys/types.h>
#include <sys/stat.h>

//exit
#include <stdlib.h>

//open
#include <fcntl.h>

//stdio, stderr
#include <stdio.h>

#define BD_MAX_CLOSE 8192

CDaemonProcess::CDaemonProcess(std::string processName, int argc, char* argv[], bool canAcceptConnections) :
		m_daemonName(processName), m_lockName(processName), m_argc(argc), m_argv(
				argv), m_canAcceptConnections(canAcceptConnections), m_pIpcConnection(NULL), m_pIpcServer(NULL) {
	m_configOptions["debuglevel"] = "0";
}

CDaemonProcess::~CDaemonProcess() {
	if (m_pIpcConnection) {
		delete m_pIpcConnection;
		m_pIpcConnection = NULL;
	}
}

const std::string& CDaemonProcess::getLockName() const {
	return m_lockName;
}

CDaemonProcess::EError CDaemonProcess::createLockName() {
	m_lockName = m_daemonName;
	return ERROR_NO_ERROR;
}

EExecutionContext CDaemonProcess::becomeDaemon() {
	int maxfd, fd;

	switch (::fork()) {
	case -1:
		return CONTEXT_ERROR;
	case 0:
		break;
	default:
		::SetLogContext(CONTEXT_PARENT);
		return CONTEXT_PARENT;
		break;
	}

	/* Become leader of new session */
	if (::setsid() == -1)
		return CONTEXT_ERROR;

	/* Ensure we are not session leader */
	switch (::fork()) {
	case -1:
		return CONTEXT_ERROR;
	case 0:
		::SetLogContext(CONTEXT_DAEMON);

		break;
	default:
		::_exit(EXIT_SUCCESS); //exit from the second child
		break;
	}

	::umask(0);
	::chdir("/");

	//we are in daemon code now
	maxfd = ::sysconf(_SC_OPEN_MAX);
	if (maxfd == -1) {
		/* Limit is indeterminate... */
		maxfd = BD_MAX_CLOSE;
	}
	/* so take a guess. start from STDERR which is the biggest */
	for (fd = STDERR_FILENO + 1; fd < maxfd; fd++) {
		// keep log file and out lock opened
		if ((fd != ::GetLogFd())
				&& (fd != m_pIpcServer->getSocketId()) /*&& (fd != STDERR_FILENO)*/)
			::close(fd);
	}

	::close(STDIN_FILENO);
	/* Reopen standard fd's to /dev/null */
	fd = ::open("/dev/null", O_RDWR);
	if (fd != STDIN_FILENO) {
		/* 'fd' should be 0 */
		return CONTEXT_ERROR;
	}
#ifdef __LOG_DONOT_REDIRECT_STDIO__
	if (::dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
		return CONTEXT_ERROR;
	}
	if (::dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
		return CONTEXT_ERROR;
	}
#else
	if (::dup2(::GetLogFd(), STDOUT_FILENO) != STDOUT_FILENO) {
		return CONTEXT_ERROR;
	}
	if (::dup2(::GetLogFd(), STDERR_FILENO) != STDERR_FILENO) {
		return CONTEXT_ERROR;
	}
#endif
	//*((int*)0) = 1;
	Log("DAEMON PID: %d", ::getpid());

	return CONTEXT_DAEMON;
}

bool CDaemonProcess::setupEnvironment() {
	return true;
}

CDaemonProcess::EError CDaemonProcess::start() {

	Log("%s version %s", m_daemonName.c_str(), _DAEMON_VERSION_);

	m_pIpcServer = new CIPCServer(m_lockName, this);
	if (m_pIpcServer == NULL) {
		return ERROR_OOM;
	}

	if (m_pIpcServer->Create()) {
		// DAEMON WAS NOT RUNNING
		// we have created server connection, now can run as daemon if required
		// parse config file first

		std::string configName = std::string("./") + m_daemonName
				+ std::string(".conf");
		CConfigFile* pConfig = new CConfigFile(configName);

		if (!pConfig->parse(this)) {
			Log("Error parsing config file");
			delete pConfig;
			return ERROR_FATAL;
		}

		delete pConfig;
		pConfig = NULL;

		// now setup the required env.
		if (setupEnvironment()) {
			// and fork
			switch (becomeDaemon()) {
			case CONTEXT_DAEMON:
				if(m_canAcceptConnections) {
					m_pIpcServer->Start();
				}
				daemonLoop();
				if(m_canAcceptConnections) {
					// we exited from the loop. stop server
					m_pIpcServer->Stop();
					m_pIpcServer->Join();
				}
				break;
			case CONTEXT_PARENT:
				break;
			case CONTEXT_ERROR:
				break;
			}
		} else {
			Log("Cannot start daemon [%s]", m_daemonName.c_str());
			return ERROR_FATAL;
		}
	} else {
		Log("Daemon is already running");

		m_pIpcConnection = new CIPCConnection(m_lockName);
		if (m_pIpcConnection == NULL) {
			return ERROR_OOM;
		}

		Log("Connecting to daemon on socket[%s]", m_lockName.c_str());
		// try connect
		if (m_pIpcConnection->connect()) {
			//DAEMON IS RUNNING
			parentLoop();
		} else {
			Log("FATAL: Couldn't neither create daemon or connect to one");
			return ERROR_FATAL;
		}
	}
	return ERROR_NO_ERROR;
}

bool CDaemonProcess::OnConfigOption(std::string& name, std::string& value) {
	Log("[Config]: [%s]-[%s]", name.c_str(), value.c_str());

	if (m_configOptions.find(name) != m_configOptions.end()) {
		m_configOptions[name] = value;
		return true;
	}

	return false;
}

void CDaemonProcess::IPCServerOnNewConnection(CIPCConnection* pConnection)
{
	pConnection->setListener(this);
	m_connections.push_back(pConnection);
}

void CDaemonProcess::OnIPCConnectionClosed(CIPCConnection* pConnection) {
	Log("Connection closed 0x%p", pConnection);
	//TODO: cancel command, remove connection from lists, delete it
	//TODO: for command connections close immediately, for streams DO NOT!
}

void CDaemonProcess::OnIPCConnectionDataReady(CIPCConnection* pConnection) {
	Log("Data ready on 0x%p", pConnection);
	//TODO: put connection to 'ready' queue
	pthread_mutex_lock(&m_mutex);
	//m_readyQueue.dele
	//m_readyQueue.push_back(pConnection);
	pthread_mutex_unlock(&m_mutex);
}

void CDaemonProcess::processConnections()
{
	connections_iterator itr;
	for(itr = m_connections.begin(); itr != m_connections.end(); itr++) {

	}
}


