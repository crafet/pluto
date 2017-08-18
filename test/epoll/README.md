epoll tutorial

1. �ṹ�嶨��
>
	�ṹ��sockaddr��socketaddr_in��������һ�µġ�sockaddr�Ķ�����
	struct sockaddr
	{
       unsigned short  sa_family;   //��ַ�壬 һ��ΪAF_INET
       char            sa_data[14];   //14�ֽڵ�Э���ַ
    }
	��sockaddr_in�Ķ������൱���ǽ�14���ֽڸ��ֲ�ɶ���ֶ���
	
	struct sockaddr_in
    {
       short int               sin_family;   //��ַ��
       unsigned short int      sin_port;      //�˿ں�
       struct in_addr          sin_addr;      //ip��ַ
       unsigned char           sin_zero[8];  //���
    }
	����������ֶ��൱��sockaddr�е�sa_data�ṹ��
	struct  in_addr {
		unsigned  long  s_addr��
	};
	
	����ʹ��inet_aton,inet_ntoa�����ʮ������һ����������ת��
	ʹ��inet_aton/inet_ntoa��Ҫ������ͷ�ļ���#include<arpa/inet.h>
	inet_ntoa(char* in_addr)����������Ҫ����s_addr��ֻҪ����in_addr�͹��ˡ�
	
	�ڱ��������У����ջ�õ�inet_ntoa�Ľ����"0.0.0.0"
	
2. epoll�ṹ��
�����epoll_event�Ľṹ����������
> 
	typedef union epoll_data {
		void        *ptr;
		int          fd;
		__uint32_t   u32;
		__uint64_t   u64;
	} epoll_data_t;

	struct epoll_event {
		__uint32_t   events; /* Epoll events */
		epoll_data_t data;   /* User data variable */
	};
	�����epoll_data��union�ṹ������һ���������fd��Ϣ��
	epoll_event�е�events�������ڼ����¼���data�������ڴ��fd��Ϣ��
3. socket��read�ӿ�
### �鿴manual�Ĺ���return value��˵��
> 
	On success, the number of bytes read is returned (zero indicates end
	of file), and the file position is advanced by this number.  It is
	not an error if this number is smaller than the number of bytes
	requested; this may happen for example because fewer bytes are
	actually available right now (maybe because we were close to end-of-
    file, or because we are reading from a pipe, or from a terminal), or
    because read() was interrupted by a signal.  See also NOTES.
	
    On error, -1 is returned, and errno is set appropriately.  In this
    case, it is left unspecified whether the file position (if any)
    changes.
	
	����0������EOF��read���ֽ�������������ֵ�����������ڶ���������¡��������ֵΪ0��˵������Ҫ�����sock�Ͻ��ж��ˡ�
	�������-1����ʱ��Ҫ�ж�error����ֶε�ֵ��
			�������error=EINTR��ô��ʱ��Ҫ����while loop
			���error=EAGAIN or error=EWOULDBLOCK����ʾ�����nonblock��fd��û��д�¼������������ء�
	��֮�����ڷ��ص�-1�����������Ҫloop���ȴ���һ�ε��¼�֪ͨ��

4. ev anatomy tutorial
һ��Ľӿ�ʹ�÷�ʽΪ
>
	��watcher��callbackfun��
	ev_init(watcher, callbackfunc)
	����watcher��Ҫ�����Ķ�д�¼�
	ev_io_set(watcher, READ/WRITE)
	��watcher�󶨵�loop�ϡ�
	ev_io_start(loop, watcher)
	����ev_run,��loop��������
	ev_run(loop, 0)
ev_TYPE�����˸���watcher�������TYPE������IO��������timer�����͡�ʹ����ֻҪ��������watcher���ɡ�