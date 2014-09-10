#include "Actrl.h"
#include "TinyTcp.h"
using namespace std;
int main()
{	
	/*
	time_t now ;
	time(&now) ;
	Actrl *obj=new Actrl;
	TcpMsgPacket msg,decodemsg;
	msg.sequence=88888888;
	msg.uin=123456789;
	msg.sendTime=now;
	msg.createTime=now;
	msg.commonType=Login;
	msg.msgType=ClientMsg;
	memcpy(msg.hostIP,"127.0.0.2",sizeof("127.0.0.2"));
	memcpy(msg.desIP,"127.0.0.1",sizeof("127.0.0.1"));
	memcpy(msg.msg,"hello world!",sizeof("hello world!"));
	char buff[1024];
	int len=EncodeMsg(msg,buff);
	
	int decodelen=DecodeMsg(buff,decodemsg);
	*/
	
	Config cfg;
	cfg.socketTimeOut=1;
	cfg.connTimeOut=10;
	cfg.listenPortNum=10;
	cfg.maxConnectNum=1024;
	cfg.tcpBuffLen=1024;
	cfg.WritePipeKey=44444;
	cfg.ReadPipeKey=55555;
	cfg.listenPorts[0]=8000;
	cfg.listenPorts[1]=8001;
	cfg.listenPorts[2]=8002;
	cfg.listenPorts[3]=8003;
	cfg.listenPorts[4]=8004;
	cfg.listenPorts[5]=8005;
	cfg.listenPorts[6]=8006;
	cfg.listenPorts[7]=8007;
	cfg.listenPorts[8]=8008;
	cfg.listenPorts[9]=8009;

	TinyTcp *tcp=new TinyTcp(cfg);
	tcp->Initialize();
	tcp->StartSerivce();
	
	
}