/*
 * Log.cpp
 *
 *  Created on: May 16, 2013
 *      Author: ruinmmal
 */
#include <string>

#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include "Utils.h"
#include "Log.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

static char const *priov[] = {
"EMERG:",   "ALERT:",  "CRIT:", "ERR:", "WARNING:", "NOTICE:", "INFO:", "DEBUG:"
};


static std::string gContext("C");
static int gLogfd = -1;

static char buffer[2048];

EExecutionContext gCnx = CONTEXT_PARENT;

void Log(std::string format, ...)
{
	va_list vl, vl1, vl2;
	va_start(vl, format);
	va_start(vl1, format);
	va_start(vl2, format);
	std::string s = std::string("[") + gContext + ":" + itoa(getpid()) + "] " + format;
	vsyslog(LOG_ERR, s.c_str(), vl);
	if(s[s.length() - 1] != '\n') {
		s += std::string("\n");
	}
	if(gCnx == CONTEXT_PARENT) {
		vprintf(s.c_str(), vl2);
	}
	if(gLogfd != -1) {
		int size = vsnprintf(buffer, 2048, s.c_str(), vl1);
		write(gLogfd, buffer, size);
	}
	va_end(vl2);
	va_end(vl1);
	va_end(vl);
}

void SetLogContext(EExecutionContext context) {
	gCnx = context;
	switch(context) {
	case CONTEXT_DAEMON:
		gContext = std::string("D");
		break;
	case CONTEXT_PARENT:
		gContext = std::string("C");
		break;
	case CONTEXT_ERROR:
		gContext = std::string("E");
		break;
	}
}

void LogInit(std::string filename) {
	gLogfd = open(filename.c_str(), O_APPEND|O_WRONLY|O_CREAT, 0666);
}

int GetLogFd() {
	return gLogfd;
}


static __ssize_t writer(void *cookie, char const *data, size_t leng)
{
    (void)cookie;
//    int     p = LOG_DEBUG, len;
//    do len = strlen(priov[p]);
//    while (memcmp(data, priov[p], len) && --p >= 0);
//
//    if (p < 0) p = LOG_INFO;
//    else data += len, leng -= len;
//    while (*data == ' ') ++data, --leng;
//
//    syslog(p, "%.*s", leng, data);
	if(gLogfd != -1) {
		write(gLogfd, data, leng);
	}
    return  leng;
}

static cookie_io_functions_t log_fns = {
    NULL, writer, NULL, NULL
};

void LogRedirect(FILE **pfp)
{
    setvbuf(*pfp = fopencookie(NULL, "w", log_fns), NULL, _IOLBF, 0);
}



