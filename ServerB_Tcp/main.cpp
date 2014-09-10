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
	cfg.WritePipeKey=99999;/*��ȡ���ݴ�����ڴ���ȡ*/
	cfg.ReadPipeKey=88888;/*��������������ڴ��д*/ 
	cfg.listenPorts[0]=8100;
	cfg.listenPorts[1]=8101;
	cfg.listenPorts[2]=8102;
	cfg.listenPorts[3]=8103;
	cfg.listenPorts[4]=8104;
	cfg.listenPorts[5]=8105;
	cfg.listenPorts[6]=8106;
	cfg.listenPorts[7]=8107;
	cfg.listenPorts[8]=8108;
	cfg.listenPorts[9]=8109;

	printf("ServerB_Tcp ������!\n");
	TinyTcp *tcp=new TinyTcp(cfg);
	tcp->Initialize();

	sockaddr_in other_addr;//��������ַ
	other_addr.sin_family=AF_INET;
	other_addr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
	other_addr.sin_port=htons(8009);
	ConnectPara para;
	para.other_addr=other_addr;
	para.uin=123456789;/*�� ServerB�������ݵ�uidһ��*/

	//��������������
	tcp->ConnectOther(para);

	tcp->StartSerivce();
}