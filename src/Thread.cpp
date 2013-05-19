/*
 * Thread.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "Thread.h"

void CThread::thread_oncancel_function(void* param)
{
	CThread* pThread = reinterpret_cast<CThread*>(param);
	pThread->OnCancel();
}


void* CThread::thread_function(void* param)
{
	CThread* pThread = reinterpret_cast<CThread*>(param);
	void * ret;
	pthread_cleanup_push(thread_oncancel_function, param);
	ret = pThread->Run();
	pthread_cleanup_pop(0);
	return ret;
}


CThread::CThread()
	: m_hHandle(-1), m_isJoinable(true), m_isCreated(false)
{
}



CThread::~CThread() {
	Cancel();
	Join();
}

bool CThread::Create(bool createDetached)
{
	pthread_attr_t* pAttr = NULL;

	m_isJoinable = !createDetached;

	if(createDetached) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pAttr = &attr;
	}

	int retVal = pthread_create(&m_hHandle, pAttr, thread_function, this);

	if (pAttr != NULL) {
		pthread_attr_destroy(pAttr);
	}

	return m_isCreated = (retVal == 0);
}
bool CThread::Join()
{
	if(m_isJoinable && m_isCreated) {
		void* threadRetVal;
		int retVal = pthread_join(m_hHandle, &threadRetVal);
		return retVal == 0;
	}
	return false;
}
bool CThread::Cancel()
{
	if(m_isCreated) {
		pthread_cancel(m_hHandle);
	}
	return true;
}

void CThread::OnCancel()
{

}


