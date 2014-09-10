#include "common.h"
#include "codequeue.h"
#pragma comment(lib,"Base.lib")
#define PIPESIZE     0x1000000
#define g_writeKey   55555
#define g_readKey    44444
class Bctrl
{
private:
	CCodeQueue* m_PipeWrite;//写队列
	CCodeQueue* m_PipeRead;//读队列

	//读和写的共享内存块
	CSharedMem* g_pShmPipeWrite;
	CSharedMem* g_pShmPipeRead;
public:
	static CSharedMem *pCurrentShm;//当前流
	Bctrl();
	int AppendCode(const unsigned char *pInCode, int sInLength);//写操作
	bool GetCode();//读操作
};