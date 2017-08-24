// BaseThread.cpp: implementation of the CBaseThread class.
//
//////////////////////////////////////////////////////////////////////
#include <pthread.h>
#include "basethread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace pluto {


CBaseThread::CBaseThread()
{
	m_stThreadStatus = CBaseThread::STOPPED ;
	m_bExitFlag = false ;
}

CBaseThread::~CBaseThread()
{

}

void * CBaseThread::Run(void * p_pvParm)
{
	CBaseThread * pWorkObj ; 
	int  iRetCode ;
	
	pWorkObj = (CBaseThread *)p_pvParm ;
	iRetCode = pWorkObj->Work() ;
	
	pWorkObj->SetStatus(STOPPED);
	
	return NULL;
}

//业务接口类.....虚函数
int	CBaseThread::Work()
{
	
	return 0 ;
}

//线程启动....
int CBaseThread::Start()
{
	int iResult ;

	pthread_attr_t stThreadAttr ;

	pthread_attr_init(&stThreadAttr);
	pthread_attr_setstacksize(&stThreadAttr,20*1024*1024);
	pthread_attr_setdetachstate(&stThreadAttr,PTHREAD_CREATE_DETACHED);

	m_stThreadStatus = CBaseThread::RUNNING ;
	m_iStatusTime = time(NULL);
	iResult = pthread_create(&m_stThreadID,&stThreadAttr,CBaseThread::Run,this);
	
	if(iResult != 0 )
	{
		//m_csMutex.Lock() ;
		m_stThreadStatus = CBaseThread::STOPPED ;
		m_bExitFlag = false ;
		//m_csMutex.UnLock() ;
	}
	
	pthread_attr_destroy(&stThreadAttr);

	return iResult == 0 ? 0:-1 ;
}

//退知退出函数
bool CBaseThread::NotifyExit()
{
	//m_csMutex.Lock();
	m_bExitFlag = true ;
	//m_csMutex.UnLock() ;

	return true;
}

int CBaseThread::Stop()
{
	if(m_stThreadStatus == CBaseThread::RUNNING)
	{
		pthread_cancel(m_stThreadID);
		//pthread_kill(m_stThreadID,9);
		NotifyExit();
		return true ;
	}
	
	return false ;
}


bool CBaseThread::IsExit()
{
	bool bRetResult = false ;
	
	//m_csMutex.Lock();
	bRetResult = m_bExitFlag ;
	//m_csMutex.UnLock() ;

	return bRetResult ;
}

}
