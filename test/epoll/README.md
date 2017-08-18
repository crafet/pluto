epoll tutorial

1. 结构体定义
>
	结构体sockaddr与socketaddr_in本质上是一致的。sockaddr的定义是
	struct sockaddr
	{
       unsigned short  sa_family;   //地址族， 一般为AF_INET
       char            sa_data[14];   //14字节的协议地址
    }
	而sockaddr_in的定义则相当于是将14个字节给分拆成多个字段中
	
	struct sockaddr_in
    {
       short int               sin_family;   //地址族
       unsigned short int      sin_port;      //端口号
       struct in_addr          sin_addr;      //ip地址
       unsigned char           sin_zero[8];  //填充
    }
	下面的三个字段相当于sockaddr中的sa_data结构。
	struct  in_addr {
		unsigned  long  s_addr；
	};
	
	可以使用inet_aton,inet_ntoa将点分十进制与一个整数进行转换
	使用inet_aton/inet_ntoa需要包含的头文件是#include<arpa/inet.h>
	inet_ntoa(char* in_addr)，参数不需要给到s_addr，只要给到in_addr就够了。
	
	在本机上运行，最终会得到inet_ntoa的结果是"0.0.0.0"
	
2. epoll结构体
定义的epoll_event的结构体内容如下
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
	这里的epoll_data是union结构，其中一般用来存放fd信息。
	epoll_event中的events则是用于监测的事件，data则是用于存放fd信息。
3. socket的read接口
### 查看manual的关于return value的说明
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
	
	返回0，代表EOF，read的字节数少于期望的值正常，会由于多种情况导致。如果返回值为0，说明不需要在这个sock上进行读了。
	如果返回-1，此时需要判断error这个字段的值。
			如果返回error=EINTR那么此时需要继续while loop
			如果error=EAGAIN or error=EWOULDBLOCK，表示在这个nonblock的fd上没读写事件，会立即返回。
	总之，对于返回的-1情况都不再需要loop，等待下一次的事件通知。

4. ev anatomy tutorial
一般的接口使用方式为
>
	将watcher与callbackfun绑定
	ev_init(watcher, callbackfunc)
	设置watcher需要监听的读写事件
	ev_io_set(watcher, READ/WRITE)
	将watcher绑定到loop上。
	ev_io_start(loop, watcher)
	调用ev_run,将loop运行起来
	ev_run(loop, 0)
ev_TYPE定义了各种watcher，这里的TYPE可以是IO，可以是timer等类型。使用者只要负责将生成watcher即可。