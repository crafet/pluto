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
       struct in_addr          in_addr;      //ip��ַ
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