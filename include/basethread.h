// BaseThread.h: interface for the CBaseThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BASE_THREAD_H_
#define _BASE_THREAD_H_

#include "Mutex.h"

namespace pluto {

}
class CBaseThread
{
public:
	enum ThreadStatus
	{
		RUNNING,
		STOPPED
	};
public:
	CBaseThread();
	virtual ~CBaseThread();

	//线程运行的接口
	static void * Run(void * p_pvParm);

	//线程实际业务工作接口
	virtual int	Work();

	//线程启动接口
	int Start();

	//退出通知函数接口
	bool NotifyExit();

	//强行退出
	int Stop();

	//获取是否退出信息
	bool IsExit();

	//获取当前线程ID
	pthread_t GetThreadID() const
	{
		return m_stThreadID;
	};

	bool IsStopped()
	{
		//m_csMutex.Lock();
		if(m_stThreadStatus == (CBaseThread::STOPPED)) return true ;
		//m_csMutex.UnLock() ;

		return false ;
	};

	bool IsRunning()
	{
		//m_csMutex.Lock();
		if(m_stThreadStatus == CBaseThread::RUNNING) return true ;
		//m_csMutex.UnLock() ;

		return false ;
	};

	void SetStatus(ThreadStatus p_stStatus)
	{
		//m_csMutex.Lock() ;
		m_stThreadStatus = p_stStatus ;
		m_iStatusTime = time(NULL);
		//m_csMutex.UnLock() ;
	}

	/*****************************************************
	 *  description :	获取状态时间
	******************************************************/
	time_t GetStatusTime() const
	{
		return m_iStatusTime;
	};
protected:
	ThreadStatus  m_stThreadStatus ;
	pthread_t     m_stThreadID ;
	volatile bool		  m_bExitFlag ;
	time_t		  m_iStatusTime;
	// CMutex        m_csMutex ;
};

}

#endif //
