#include "serverACtrl.h"
#include <stdio.h>
#include <iostream>
using namespace std;

serverACtrl *obj = NULL;
DWORD WINAPI funProc(void* para)
{
	int writekey=55555;//55555;

	int readkey=44444;//44444;

	obj=new serverACtrl(writekey,readkey);


	time_t now ;
	time(&now) ;
	TcpMsgPacket msg;
	msg.sequence=88888888;
	msg.uin=123456789;
	msg.sendTime=now;
	msg.createTime=now;
	msg.commonType=Login;
	msg.msgType=ClientMsg;
	memcpy(msg.hostIP,"58.198.176.217",sizeof("58.198.176.217"));
	memcpy(msg.desIP,"58.198.176.217",sizeof("58.198.176.217"));
	char buff[1024];
	char msgdata[1024];

	printf("��ʼ����ServerA_Tcp�ڴ�ܵ��е�����!\n");
	while(1)
	{
		TcpMsgPacket *msg=obj->GetMsg();
		if(msg!=0)
		{
			printf("��ȡ����Ϣ:%s\n",msg->msg);
		}
	}
	return 0;
}
void main()
{
	CreateThread(NULL,0,funProc,NULL,NULL,NULL);
	Sleep(500);
	
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

	while(1)
	{
		//�ӹ�����
		memset(msg.msg,'\0',sizeof(msg.msg));
		cout<<"������Ҫ���͵�����\n";	
		cin>>msgdata;
		memcpy(msg.msg,msgdata,strlen(msgdata));
		EncodeMsg(msg,buff);

		int ret=obj->SendMsg(msg);
	}

}