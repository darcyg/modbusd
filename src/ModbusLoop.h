/*
 * CModbusLoop.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef CMODBUSLOOP_H_
#define CMODBUSLOOP_H_

#include "Thread.h"

class CModbusLoop: public CThread {
public:
	CModbusLoop();
	virtual ~CModbusLoop();
	virtual void* Run();

};

#endif /* CMODBUSLOOP_H_ */
