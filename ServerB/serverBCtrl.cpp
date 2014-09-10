#include "serverBCtrl.h"

serverBCtrl::serverBCtrl()
{

}

serverBCtrl::serverBCtrl(int writekey,int readkey)
{
	base *basefun=new base;
	//���������ڴ�
	//Step:1
	ShmPipeWrite  = basefun->CreateShareMem(writekey, CCodeQueue::CountQueueSize(PIPESIZE), false); 
	CCodeQueue::pCurrentShm = ShmPipeWrite;

	WriteQueue = new CCodeQueue(PIPESIZE);//����д����

	//Step:2
	ShmPipeRead = basefun->CreateShareMem(readkey, CCodeQueue::CountQueueSize(PIPESIZE), false);
	CCodeQueue::pCurrentShm = ShmPipeRead;	

	ReadQueue = new CCodeQueue(PIPESIZE);//����������
}
/*ͨ�����������Ҫ���͵�����д���ڴ�*/
int serverBCtrl::SendMsg(TcpMsgPacket msg)
{
	char buff[10240];
	int len=EncodeMsg(msg,buff);
	int ret = WriteQueue->AppendOneCode((unsigned char*)&buff, len);
	if( ret < 0 )
	{
		return -1;
	}
	return ret;
}

//�������ڴ��е�����,���س�������Ҫ��TcpMsgPacket��ʽ
TcpMsgPacket *serverBCtrl::GetMsg()
{
	TcpMsgPacket msg;
	int ret;
	char buff[10240];
	unsigned int readLen = 10240;
	ret = ReadQueue->GetHeadCode((unsigned char*)buff,&readLen);
	if (ret==ERR_FOR_NOPACKET)
	{
		return NULL;
	}
	else if(ret<0)
	{
		printf("error\n");
		return NULL;;
	}
	if( readLen < 0 )
	{
		return NULL;
	}

	DecodeMsg(buff,msg);/*��������Ϊʵ��*/

	return &msg;

}