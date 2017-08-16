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
       struct in_addr          in_addr;      //ip地址
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