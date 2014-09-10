#include "common.h"
#include "codequeue.h"
#include "TinyTcp.h"
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"NetFrame.lib")
#define PIPESIZE     0x1000000
#define g_writeKey   88888
#define g_readKey    77777
class Actrl
{
private:
	CCodeQueue* m_PipeWrite;//写队列
	CCodeQueue* m_PipeRead;//读队列

	//读和写的共享内存块
	CSharedMem* g_pShmPipeWrite;
	CSharedMem* g_pShmPipeRead;
public:
	static CSharedMem *pCurrentShm;//当前流
	Actrl();
	int AppendCode(const unsigned char *pInCode, int sInLength);//写操作
	bool GetCode();//读操作
};