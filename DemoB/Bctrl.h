#include "common.h"
#include "codequeue.h"
#pragma comment(lib,"Base.lib")
#define PIPESIZE     0x1000000
#define g_writeKey   55555
#define g_readKey    44444
class Bctrl
{
private:
	CCodeQueue* m_PipeWrite;//д����
	CCodeQueue* m_PipeRead;//������

	//����д�Ĺ����ڴ��
	CSharedMem* g_pShmPipeWrite;
	CSharedMem* g_pShmPipeRead;
public:
	static CSharedMem *pCurrentShm;//��ǰ��
	Bctrl();
	int AppendCode(const unsigned char *pInCode, int sInLength);//д����
	bool GetCode();//������
};