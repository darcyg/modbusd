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
#include <stdint.h>
#include <queue>
#include <string>
#include <sqlite3.h>
#include <syslog.h>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include "gateway/system.hpp"
#include "gateway/command.hpp"
#include "gateway/system.hpp"

#define PARAM_STATION_STATE 1050100090
#define PARAM_ERROR_CODE    1050100100


typedef struct {
	int m_paramId;
	double m_value;
	uint16_t m_startReg;
	uint16_t m_nbRegs;
} data_parameter_t;

typedef struct {
	int m_id;
	const char* m_name;
	double m_value;
	uint16_t m_startReg;
	uint16_t m_nbRegs;
} setting_m_t;


class IOnDataUpdateListener {
public:
	/*
	 * Called when either parameters or settings are updated. if no update is avaialbel
	 * nbParams or nbSettngs == 0. Never called with both set to 0
	 */
	virtual void OnDataUpdated(const data_parameter_t* params, int nbParams, const setting_m_t* settings, int nbSettings) = 0;
	virtual void OnDataAvailable(bool isAvailable) = 0;
protected:
	virtual ~IOnDataUpdateListener() {}
};

using namespace stek::oasis::ic::dbgateway;


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
	setting_m_t* m_settings;
	int m_nbParams;
	int m_nbSettings;
	//int sock; //connection to dbgateway
	stek::oasis::ic::dbgateway::file_t* socket;
	//params
	int* availchannels;
	int* attachedparams;
	int numofrows; 
	//events
	string_t attachedsettings[256];
	int numofrowsevnt;

	// cache register addresses for params 1050100090 -- "код состояния"
	// and 1050100100 -- "код неисправности"
	int m_reg_stationState;
	int m_reg_errorCode;

protected:
	typedef std::list<IOnDataUpdateListener*>::iterator listeners_iterrator;

protected:
	void CloseDb();
	void NotifyDataUpdated(bool isParam, bool isSettings);
	void NotifyDataAvailable(bool isAvailable);
	bool CheckParamsUpdated();
	bool CheckSettingsUpdated();
	bool CheckParamsUpdatedgateway();
	bool CheckSettingsUpdatedgateway();

	bool CheckDataValid();
	void RemapStateAndError();
public:
	CDataPump(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings);
	virtual ~CDataPump();
	virtual void* Run();
	virtual bool Create(std::string paramDbName, std::string eventDbName);
	void RegisterDataUpdateListener(IOnDataUpdateListener* pListener);
	void DeregisterDataUpdateListener(IOnDataUpdateListener* pListener);
};

#endif /* DATAPUMP_H_ */
