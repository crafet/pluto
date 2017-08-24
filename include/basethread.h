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

	//�߳����еĽӿ�
	static void * Run(void * p_pvParm);

	//�߳�ʵ��ҵ�����ӿ�
	virtual int	Work();

	//�߳������ӿ�
	int Start();

	//�˳�֪ͨ�����ӿ�
	bool NotifyExit();

	//ǿ���˳�
	int Stop();

	//��ȡ�Ƿ��˳���Ϣ
	bool IsExit();

	//��ȡ��ǰ�߳�ID
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
	 *  description :	��ȡ״̬ʱ��
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
