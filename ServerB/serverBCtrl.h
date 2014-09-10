#include "common.h"
#include "codequeue.h"
#include "TinyTcp.h"
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"NetFrame.lib")
#define PIPESIZE     0x1000000
class serverBCtrl
{
private:
	CCodeQueue* WriteQueue;//д����
	CCodeQueue* ReadQueue;//������

	//����д�Ĺ����ڴ��
	CSharedMem* ShmPipeWrite;
	CSharedMem* ShmPipeRead;
public:
	serverBCtrl();
	serverBCtrl(int writeKey,int readKey);/*witekey ��Ӧ NetFrame�� ReadKey,readKey��Ӧ NetFrame��WriteKey*/
	int SendMsg(TcpMsgPacket msg);//д����
	TcpMsgPacket* GetMsg();//������
};