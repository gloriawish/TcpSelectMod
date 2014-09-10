#include "TinyTcp.h"
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"NetFrame.lib")
void main()
{

	Config cfg;
	cfg.socketTimeOut=1;
	cfg.connTimeOut=10;
	cfg.listenPortNum=10;
	cfg.maxConnectNum=1024;
	cfg.tcpBuffLen=1024;
	cfg.WritePipeKey=44444;/*��ȡ���ݴ�����ڴ���ȡ*/
	cfg.ReadPipeKey=55555;/*��������������ڴ��д*/ 
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

	printf("ServerA_Tcp ������!\n");
	TinyTcp *tcp=new TinyTcp(cfg);
	tcp->Initialize();
	tcp->StartSerivce();
	
}