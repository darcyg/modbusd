/*
 * Log.h
 *
 *  Created on: May 16, 2013
 *      Author: ruinmmal
 */

#ifndef LOG_H_
#define LOG_H_

#include "DaemonProcess.h"

void LogInit(std::string filename);
int GetLogFd();
void Log(std::string format, ...);
extern void LogFatal(std::string format, ...);
void SetLogContext(EExecutionContext context);
void LogRedirect(FILE **pfp);

#endif /* LOG_H_ */
