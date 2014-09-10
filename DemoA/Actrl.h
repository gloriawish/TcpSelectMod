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
	CCodeQueue* m_PipeWrite;//д����
	CCodeQueue* m_PipeRead;//������

	//����д�Ĺ����ڴ��
	CSharedMem* g_pShmPipeWrite;
	CSharedMem* g_pShmPipeRead;
public:
	static CSharedMem *pCurrentShm;//��ǰ��
	Actrl();
	int AppendCode(const unsigned char *pInCode, int sInLength);//д����
	bool GetCode();//������
};