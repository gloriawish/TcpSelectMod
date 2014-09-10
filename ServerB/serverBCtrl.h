#include "common.h"
#include "codequeue.h"
#include "TinyTcp.h"
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"NetFrame.lib")
#define PIPESIZE     0x1000000
class serverBCtrl
{
private:
	CCodeQueue* WriteQueue;//写队列
	CCodeQueue* ReadQueue;//读队列

	//读和写的共享内存块
	CSharedMem* ShmPipeWrite;
	CSharedMem* ShmPipeRead;
public:
	serverBCtrl();
	serverBCtrl(int writeKey,int readKey);/*witekey 对应 NetFrame的 ReadKey,readKey对应 NetFrame的WriteKey*/
	int SendMsg(TcpMsgPacket msg);//写操作
	TcpMsgPacket* GetMsg();//读操作
};