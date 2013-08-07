/*
 * DataPump.cpp
 *
 *  Created on: May 19, 2013
 *      Author: ruinmmal
 */

#include "DataPump.h"
#include "Log.h"

#include <string.h>

#include "Thread.h"

#include "gateway/error.hpp"
#include "gateway/string.hpp"
#include "gateway/logger.hpp"


//using namespace stek::oasis::ic::dbgateway;

CDataPump::CDataPump(data_parameter_t* params, int nbParams, setting_m_t* settings, int nbSettings)
	: m_pParamDb(NULL), m_pEventDb(NULL), m_pParamStm(NULL), m_pEventStm(NULL),
	  m_params(params), m_settings(settings), m_nbParams(nbParams), m_nbSettings(nbSettings)
{
	m_sParamSqlQuery = std::string("select paramid,value,channelid from (select  *  from tblparamdata as filter inner join (" \
					"select paramId, max(registerdate) as registerdate from tblparamdata group by paramId)" \
					"as filter1 on filter.ParamId = filter1.paramId and filter.registerdate = filter1.registerdate)");

	m_sEventSqlQuery = std::string("select Argument3,Argument1 from (" \
					"select  *  from tbleventbus as filter inner join" \
					" (select Argument3, max(registerdate) as registerdate from tbleventbus where TypeId = \"11\" group by Argument3 )	as filter1" \
					" on filter.Argument3 = filter1.Argument3 and filter.registerdate = filter1.registerdate)");
	stek::oasis::ic::dbgateway::object_t obj("device");
	socket = new file_t(obj);
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
	mark_t eot( "EOT" );
	Log("[dbgateway] send EOT");

	try {
		socket->write( eot.str() + command_t::eoc.str() );
		socket->close();
	} catch (...) {
		Log("Exception during CDataPump::CloseDb(). Ignoring");
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
	//Operate socket. Open socket to gateway
    try {
		socket->socket();
		Log("CONNECT FROM PID %d", ::getpid());
		printf("CONNECT FROM PID %d", ::getpid());
		socket->connect( "/tmp/dbgateway.socket" );
    } catch (...) {
    	Log("Exception connectiong to dbgateway. Ignoring");
    }

	//operate db

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
	numofrows=0;
	
	availchannels=(int*)calloc(1024,sizeof(int)); //todo make dynamic
	attachedparams=(int*)calloc(1024,sizeof(int));

	while ((rc =sqlite3_step(m_pParamStm)) ==  SQLITE_ROW) {
		
		int paramId = sqlite3_column_int(m_pParamStm, 0);
		double value = sqlite3_column_double(m_pParamStm, 1);
		int channelid = sqlite3_column_int(m_pParamStm, 2);
		availchannels[numofrows]=channelid;
		attachedparams[numofrows]=paramId;
		numofrows = numofrows+1;
		Log( "Found parameter: P:%d V:%g ch:%d", paramId, value,channelid);


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

	Log( "number of parameters:%d", numofrows);
	return valuesChanged;
}


bool CDataPump::CheckParamsUpdatedgateway() {
//	int rc;
	bool valuesChanged = false;
	
	
//operate dbgateway service

	Log( "asking gateway for params in cache: %d", numofrows);
        size_t n = numofrows;
        for ( size_t i = 0; i < n; i++  ) {
        	//Log("1");
	        rdp_command_t * cmd = new rdp_command_t();
        	//Log("2");
            cmd->channels.push_back( channel_t( availchannels[i] ) );
        	//Log("3");
	        osstream_t ostr;
        	//Log("4");
	        cmd->bin_write( ostr );
        	//Log("5");
            socket->write( buffer_t( std::string(ostr.str()) ) );
        	//Log("6");

            buffer_t buffer( 1000 );
        	//Log("7");
            ssize_t read = -1;
        	//Log("8");

            read = ::read( socket->fd(), buffer.data(), buffer.size() );
        	//Log("9");
            if ( read != -1 ) {
             buffer.resize( read );
            }


            int valint = 0;
	        float *valuep = (float*)(&valint);
            valint |= ((int)buffer[40]) << 24;/*fixme*/
            valint |= ((int)buffer[41]) << 16;/*fixme*/
            valint |= ((int)buffer[42]) << 8;/*fixme*/
            valint |= (int)buffer[43];/*fixme*/

            Log("[modbusd-dbgateway] got parameter from cache value:%f[%02X %02X %02X %02X]", *valuep ,buffer[40], buffer[41], buffer[42], buffer[43]);
		
			for(int k = 0;k < m_nbParams; k++) {
				if(m_params[k].m_paramId == attachedparams[i]) {
					if(m_params[k].m_value != *valuep) {
						m_params[k].m_value = *valuep;
						valuesChanged = true;
					}
					break;
				}
			}
	}
	return valuesChanged;
}


bool CDataPump::CheckSettingsUpdated() {
	int rc;
	bool valuesChanged = false;
	numofrowsevnt=0;

	while ((rc =sqlite3_step(m_pEventStm)) ==  SQLITE_ROW) {
		const unsigned char * paramId = sqlite3_column_text(m_pEventStm, 0);
		double value = sqlite3_column_double(m_pEventStm, 1);
//		Log( "Found setting: S:%s V:%g", paramId, value);
		size_t len = strlen((const char*)paramId);
		attachedsettings[numofrowsevnt].assign((char *)paramId, 0, len);
		Log( "Found setting string_t: S:%s V:%g Len:%d", attachedsettings[numofrowsevnt].c_str(), value, len);
		numofrowsevnt ++;

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

bool CDataPump::CheckSettingsUpdatedgateway() {
//	int rc;
	bool valuesChanged = false;

	Log( "asking gateway for events in cache: %d", numofrowsevnt);

    for ( size_t i = 0; i < numofrowsevnt; i++ ) {
        rds_command_t * cmd = new rds_command_t();
        cmd->names.push_back( attachedsettings[i] );

        osstream_t ostr;
        cmd->bin_write( ostr );
        socket->write( buffer_t( std::string(ostr.str()) ) );

        buffer_t buffer( 1000 );
        ssize_t read = -1;

        read = ::read( socket->fd(), buffer.data(), buffer.size() );
        if ( read != -1 ) {
         buffer.resize( read );
        }
        /* sizeof Argument1 from RDS packet.*/
        int arg1_len = 0;
        arg1_len |= ((int)buffer[40]) << 24;/*fixme*/
        arg1_len |= ((int)buffer[41]) << 16;/*fixme*/
        arg1_len |= ((int)buffer[42]) << 8;/*fixme*/
        arg1_len |= (int)buffer[43];/*fixme*/
        /* Argument1 field from RDS packet.*/
        char tmpstr[100];
        for(int n=0 ; n<arg1_len ; n++)
        	tmpstr[n] = buffer[44+n];
        tmpstr[arg1_len] = 0;
        double value = atof(tmpstr);

        Log("[modbusd-dbgateway] got Setting from cache arg1_len:%d arg1:%f arg1s:%s.", arg1_len, value, tmpstr);

		for(int k = 0;k < m_nbSettings; k++) {
			if(attachedsettings[i].compare(m_settings[k].m_name) == 0) {
		        Log("[modbusd-dbgateway] got Setting from cache %s.", attachedsettings[i].c_str());
				if(m_settings[k].m_value != value) {
					m_settings[k].m_value = value;
					valuesChanged = true;
				}
				break;
			}
		}
    }
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
	bool firsttime=true;

	while(true) {
		//TODO: check for bad values in

		if (firsttime)	{
			paramsChanged = CheckParamsUpdated();
			settingsChanged = CheckSettingsUpdated();
			firsttime=false;
		} else {
			try {
				paramsChanged = CheckParamsUpdatedgateway();
				settingsChanged = CheckSettingsUpdatedgateway();
			} catch (...) {
				firsttime=true;
				Log("Exception getting parameters or settings through dbgateway. Ignoring");
			}
		}



		if(paramsChanged || settingsChanged) {
			Log("Some values changed");
			NotifyDataUpdated(paramsChanged, settingsChanged);
		}

		timeout.tv_sec = 10;
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


