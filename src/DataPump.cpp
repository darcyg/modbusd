/*
 * DataPump.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "DataPump.h"
#include "Log.h"

#include <string.h>

CDataPump::CDataPump(data_parameter_t* params, int nbParams, setting_t* settings, int nbSettings)
	: m_pParamDb(NULL), m_pEventDb(NULL), m_pParamStm(NULL), m_pEventStm(NULL),
	  m_params(params), m_settings(settings), m_nbParams(nbParams), m_nbSettings(nbSettings)
{
	m_sParamSqlQuery = std::string("select paramid,value from (select  *  from tblparamdata as filter inner join (" \
					"select paramId, max(registerdate) as registerdate from tblparamdata group by paramId)" \
					"as filter1 on filter.ParamId = filter1.paramId and filter.registerdate = filter1.registerdate)");

	m_sEventSqlQuery = std::string("select Argument3,Argument1 from (" \
					"select  *  from tbleventbus as filter inner join" \
					" (select Argument3, max(registerdate) as registerdate from tbleventbus where TypeId = \"11\" group by Argument3 )	as filter1" \
					" on filter.Argument3 = filter1.Argument3 and filter.registerdate = filter1.registerdate)");

}

CDataPump::~CDataPump() {
	CloseDb();
}

void CDataPump::CloseDb() {
	if(m_pParamStm) {
		sqlite3_finalize(m_pParamStm);
		m_pParamStm = NULL;
	}
	if(m_pParamDb) {
		sqlite3_close(m_pParamDb);
		m_pParamDb = NULL;
	}
	if(m_pEventStm) {
		sqlite3_finalize(m_pEventStm);
		m_pEventStm = NULL;
	}
	if(m_pEventDb) {
		sqlite3_close(m_pEventDb);
		m_pEventDb = NULL;
	}
}

bool CDataPump::Create(std::string paramDbName, std::string eventDbName)
{
	int rc;
	//try open DB
	rc = sqlite3_open_v2(paramDbName.c_str(), &m_pParamDb, SQLITE_OPEN_READONLY, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't open DB: %s\n", paramDbName.c_str());
    	CloseDb();
    	return false;
    }
    rc = sqlite3_prepare_v2(m_pParamDb, m_sParamSqlQuery.c_str(),-1, &m_pParamStm, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't prepare SQL statement for: %s : error %s\n", paramDbName.c_str(), sqlite3_errmsg(m_pParamDb));
    	CloseDb();
    	return false;
    }

	rc = sqlite3_open_v2(eventDbName.c_str(), &m_pEventDb, SQLITE_OPEN_READONLY, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't open DB: %s\n", eventDbName.c_str());
    	CloseDb();
    	return false;
    }
    rc = sqlite3_prepare_v2(m_pEventDb, m_sEventSqlQuery.c_str(),-1, &m_pEventStm, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't prepare SQL statement for: %s : error %s\n", eventDbName.c_str(), sqlite3_errmsg(m_pEventDb));
    	CloseDb();
    	return false;
    }

	return CThread::Create();
}

bool CDataPump::CheckParamsUpdated() {
	int rc;
	bool valuesChanged = false;
	while ((rc =sqlite3_step(m_pParamStm)) ==  SQLITE_ROW) {
		int paramId = sqlite3_column_int(m_pParamStm, 0);
		double value = sqlite3_column_double(m_pParamStm, 1);
		//Log( "Found parameter: P:%d V:%g", paramId, value);

		for(int i = 0;i < m_nbParams; i++) {
			if(m_params[i].m_paramId == paramId) {
				if(m_params[i].m_value != value) {
					m_params[i].m_value = value;
					valuesChanged = true;
				}
				break;
			}
		}
	}
	if(rc != SQLITE_DONE) {
		Log("SQL: %d %s", rc, sqlite3_errmsg(m_pParamDb));
	}
	sqlite3_reset(m_pParamStm);
	return valuesChanged;
}

bool CDataPump::CheckSettingsUpdated() {
	int rc;
	bool valuesChanged = false;
	while ((rc =sqlite3_step(m_pEventStm)) ==  SQLITE_ROW) {
		const unsigned char * paramId = sqlite3_column_text(m_pEventStm, 0);
		double value = sqlite3_column_double(m_pEventStm, 1);
		//Log( "Found setting: S:%s V:%g", paramId, value);

		for(int i = 0;i < m_nbSettings; i++) {
			if(strcmp(m_settings[i].m_name,(const char*)paramId) == 0) {
				if(m_settings[i].m_value != value) {
					m_settings[i].m_value = value;
					valuesChanged = true;
				}
				break;
			}
		}
	}
	if(rc != SQLITE_DONE) {
		Log("SQL: %d %s", rc, sqlite3_errmsg(m_pEventDb));
	}
	sqlite3_reset(m_pEventStm);
	return valuesChanged;
}


/*
 * the main loop.
 * 1. Get data from DB
 * 2. Fill in a copy of register table
 * 3. swap tables
 */
void* CDataPump::Run()
{
	struct timespec timeout;

	bool paramsChanged = false;
	bool settingsChanged = false;

	while(true) {
		//TODO: check for bad values in

		paramsChanged = CheckParamsUpdated();
		settingsChanged = CheckSettingsUpdated();

		if(paramsChanged || settingsChanged) {
			Log("Some values changed");
			NotifyDataUpdated(paramsChanged, settingsChanged);
		}

		timeout.tv_sec = 5;
		timeout.tv_nsec = 0;
		nanosleep(&timeout, NULL);
	}
	return NULL;
}

void CDataPump::RegisterDataUpdateListener(IOnDataUpdateListener* pListener)
{
	if(pListener != NULL) {
		m_listeners.push_back(pListener);
	}
}

void CDataPump::DeregisterDataUpdateListener(IOnDataUpdateListener* pListener)
{
	if(pListener != NULL) {
		m_listeners.remove(pListener);
	}
}

void CDataPump::NotifyDataUpdated(bool isParam, bool isSettings)
{
	for(listeners_iterrator it = m_listeners.begin(); it != m_listeners.end(); it++) {
		(*it)->OnDataUpdated(m_params, isParam ? m_nbParams : 0, m_settings, isSettings ? m_nbSettings : 0);
	}
}


