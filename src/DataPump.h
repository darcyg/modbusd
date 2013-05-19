/*
 * DataPump.h
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#ifndef DATAPUMP_H_
#define DATAPUMP_H_

#include "Thread.h"
#include <sqlite3.h>
#include <string>
#include <list>

typedef struct {
	int m_paramId;
	double m_value;
	unsigned int m_startReg;
	unsigned int m_nbRegs;
} data_parameter_t;

class IOnDataUpdateListener {
public:
	virtual void OnDataUpdated(const data_parameter_t* params, int size) = 0;
protected:
	virtual ~IOnDataUpdateListener() {}
};

class CDataPump: public CThread {

protected:
	sqlite3* m_pDb;
	sqlite3_stmt* m_pStm;
	std::string m_sSqlQuery;
	std::list<IOnDataUpdateListener*> m_listeners;

	data_parameter_t* m_params;
	int m_nbParams;
protected:
	typedef std::list<IOnDataUpdateListener*>::iterator listeners_iterrator;

protected:
	void CloseDb();
	void NotifyDataUpdated();
public:
	CDataPump(data_parameter_t* params, int nbParams);
	virtual ~CDataPump();
	virtual void* Run();
	virtual bool Create(std::string dbName);
	void RegisterDataUpdateListener(IOnDataUpdateListener* pListener);
	void DeregisterDataUpdateListener(IOnDataUpdateListener* pListener);
};

#endif /* DATAPUMP_H_ */
