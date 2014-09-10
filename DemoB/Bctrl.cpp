#include "Bctrl.h"
#include <stdio.h>

using namespace Common;
CSharedMem *Bctrl::pCurrentShm;
Bctrl::Bctrl()
{
	base *BBBBB=new base;
	//���������ڴ�
	//Step:1
	g_pShmPipeWrite  = BBBBB->CreateShareMem(g_writeKey, CCodeQueue::CountQueueSize(PIPESIZE), false); 
	CCodeQueue::pCurrentShm = g_pShmPipeWrite;

	m_PipeWrite = new CCodeQueue(PIPESIZE);//����д����

	//Step:2
	g_pShmPipeRead = BBBBB->CreateShareMem(g_readKey, CCodeQueue::CountQueueSize(PIPESIZE), false);
	CCodeQueue::pCurrentShm = g_pShmPipeRead;	

	m_PipeRead = new CCodeQueue(PIPESIZE);//����������

}

int Bctrl::AppendCode(const unsigned char *pInCode, int sInLength)
{
	int iTempRt = m_PipeWrite->AppendOneCode(pInCode, sInLength);
	if( iTempRt < 0 )
	{
		return -1;
	}
	return 0;
}

//�������ڴ��е�����
bool Bctrl::GetCode()
{
	int nRst;
	char buff[1024];
	unsigned int nLen = 1024;
	nRst = m_PipeRead->GetHeadCode((unsigned char*)buff,&nLen);
	if (nRst==ERR_FOR_NOPACKET)
	{
		return false;;
	}
	else if(nRst<0)
	{
		printf("error\n");
		return false;;
	}
	if( nLen < 0 )
	{
		return false;
	}
	buff[nLen] = 0;
	printf("%s\n",buff);
	return true;

	
}