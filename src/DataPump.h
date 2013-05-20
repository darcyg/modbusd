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

typedef struct {
	char* m_name;
	double m_value;
	unsigned int m_startReg;
	unsigned int m_nbRegs;
} setting_t;


class IOnDataUpdateListener {
public:
	/*
	 * Called when either parameters or settings are updated. if no update is avaialbel
	 * nbParams or nbSettngs == 0. Never called with both set to 0
	 */
	virtual void OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_t* settings, int nbSettings) = 0;
protected:
	virtual ~IOnDataUpdateListener() {}
};

class CDataPump: public CThread {

protected:
	sqlite3* m_pParamDb;
	sqlite3* m_pEventDb;
	sqlite3_stmt* m_pParamStm;
	sqlite3_stmt* m_pEventStm;
	std::string m_sParamSqlQuery;
	std::string m_sEventSqlQuery;
	std::list<IOnDataUpdateListener*> m_listeners;

	data_parameter_t* m_params;
	setting_t* m_settings;
	int m_nbParams;
	int m_nbSettings;
protected:
	typedef std::list<IOnDataUpdateListener*>::iterator listeners_iterrator;

protected:
	void CloseDb();
	void NotifyDataUpdated(bool isParam, bool isSettings);
	bool CheckParamsUpdated();
	bool CheckSettingsUpdated();
public:
	CDataPump(data_parameter_t* params, int nbParams, setting_t* settings, int nbSettings);
	virtual ~CDataPump();
	virtual void* Run();
	virtual bool Create(std::string paramDbName, std::string eventDbName);
	void RegisterDataUpdateListener(IOnDataUpdateListener* pListener);
	void DeregisterDataUpdateListener(IOnDataUpdateListener* pListener);
};

#endif /* DATAPUMP_H_ */
