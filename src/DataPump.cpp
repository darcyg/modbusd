/*
 * DataPump.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "DataPump.h"
#include "Log.h"

CDataPump::CDataPump(data_parameter_t* params, int nbParams)
	: m_pDb(NULL), m_pStm(NULL), m_params(params), m_nbParams(nbParams)
{
	m_sSqlQuery = std::string("select paramid,value from (select  *  from tblparamdata as filter inner join (" \
					"select paramId, max(registerdate) as registerdate from tblparamdata group by paramId)" \
					"as filter1 on filter.ParamId = filter1.paramId and filter.registerdate = filter1.registerdate)");
}

CDataPump::~CDataPump() {
	CloseDb();
}

void CDataPump::CloseDb() {
	if(m_pStm) {
		sqlite3_finalize(m_pStm);
		m_pStm = NULL;
	}
	if(m_pDb) {
		sqlite3_close(m_pDb);
		m_pDb = NULL;
	}
}

bool CDataPump::Create(std::string dbName)
{
	int rc;
	//try open DB
	rc = sqlite3_open_v2(dbName.c_str(), &m_pDb, SQLITE_OPEN_READONLY, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't open DB: %s\n", dbName.c_str());
    	CloseDb();
    	return false;
    }
    rc = sqlite3_prepare_v2(m_pDb, m_sSqlQuery.c_str(),-1, &m_pStm, NULL);

    if(rc != SQLITE_OK)
    {
    	Log("couldn't prepare SQL statement for: %s : error %s\n", dbName.c_str(), sqlite3_errmsg(m_pDb));
    	CloseDb();
    	return false;
    }

    //TODO: check for errors and cleanup
	//pthread_cond_init(&m_condProcessQueue, NULL);
	//pthread_mutex_init(&m_queueMutex, NULL);

	return CThread::Create();
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

	while(true) {
		//TODO: check for bad values in
		while (sqlite3_step(m_pStm) ==  SQLITE_ROW) {
			int paramId = sqlite3_column_int(m_pStm, 0);
			double value = sqlite3_column_double(m_pStm, 1);
			Log( "Found parameter: P:%d V:%g", paramId, value);

			for(int i = 0;i < m_nbParams; i++)
				if(m_params[i].m_paramId == paramId) {
					m_params[i].m_value = value;
					break;
				}
		}
		NotifyDataUpdated();

		timeout.tv_sec = 15;
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

void CDataPump::NotifyDataUpdated()
{
	for(listeners_iterrator it = m_listeners.begin(); it != m_listeners.end(); it++) {
		(*it)->OnDataUpdated(m_params, m_nbParams);
	}
}


