#include <stdio.h>
#include <WinSock.h>
#pragma comment(lib,"wsock32")
#include <iostream>
#define BUFF_SIZE 64
#include "TinyTcp.h"
using namespace std;
void main()
{
	WSADATA wsadata;
	WORD version=MAKEWORD(2,2);
	int ret=WSAStartup(version,&wsadata);
	if(ret!=0)
	{
		cout<<"初始化失败!";
		return;
	}


	SOCKADDR_IN server_addr;//服务器地址
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.S_un.S_addr=inet_addr("192.168.30.162");
	//server_addr.sin_addr.s_addr = htonl( INADDR_ANY);
	server_addr.sin_port=htons(8000);

	SOCKET client_sock=socket(AF_INET,SOCK_STREAM,0);
	ret=connect(client_sock,(LPSOCKADDR)&server_addr,sizeof(server_addr));
	if(ret==SOCKET_ERROR)
	{
		cout<<"连接服务器失败!";
		return;
	}
	time_t now ;
	time(&now) ;
	TcpMsgPacket msg;
	msg.sequence=88888888;
	msg.uin=123456789;
	msg.sendTime=now;
	msg.createTime=now;
	msg.commonType=Login;
	msg.msgType=ClientMsg;
	memcpy(msg.hostIP,"127.0.0.2",sizeof("127.0.0.2"));
	memcpy(msg.desIP,"127.0.0.1",sizeof("127.0.0.1"));
	char buff[1024];
	char msgdata[1024];
	int x=0;
	while(1)
	{	
		memset(msg.msg,'\0',sizeof(msg.msg));
		cout<<"请输入要发送的数据\n";	
		cin>>msgdata;
		memcpy(msg.msg,msgdata,strlen(msgdata));
		int len=EncodeMsg(msg,buff);
		ret=send(client_sock,buff,len,0);
		if(ret==SOCKET_ERROR)
		{
			cout<<"发送失败!";
			//return;
		}
	}




}