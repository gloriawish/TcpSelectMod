#include "serverBCtrl.h"

serverBCtrl::serverBCtrl()
{

}

serverBCtrl::serverBCtrl(int writekey,int readkey)
{
	base *basefun=new base;
	//创建共享内存
	//Step:1
	ShmPipeWrite  = basefun->CreateShareMem(writekey, CCodeQueue::CountQueueSize(PIPESIZE), false); 
	CCodeQueue::pCurrentShm = ShmPipeWrite;

	WriteQueue = new CCodeQueue(PIPESIZE);//创建写队列

	//Step:2
	ShmPipeRead = basefun->CreateShareMem(readkey, CCodeQueue::CountQueueSize(PIPESIZE), false);
	CCodeQueue::pCurrentShm = ShmPipeRead;	

	ReadQueue = new CCodeQueue(PIPESIZE);//创建读队列
}
/*通过这个方法把要发送的数据写入内存*/
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

//读共享内存中的数据,返回成我们需要的TcpMsgPacket格式
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

	DecodeMsg(buff,msg);/*解析数据为实体*/

	return &msg;

}