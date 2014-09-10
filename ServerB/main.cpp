#include "serverBCtrl.h"
#include <stdio.h>
#include <iostream>
using namespace std;
void main()
{
	int writekey=88888;

	int readkey=99999;

	serverBCtrl *obj=new serverBCtrl(writekey,readkey);


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

	while(1)
	{
		/*加工数据*/
		memset(msg.msg,'\0',sizeof(msg.msg));
		cout<<"请输入要发送的数据\n";	
		cin>>msgdata;
		memcpy(msg.msg,msgdata,strlen(msgdata));
		EncodeMsg(msg,buff);


		int ret=obj->SendMsg(msg);
		
	}

	

}