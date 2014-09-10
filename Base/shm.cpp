#include "shm.h"

unsigned char* CSharedMem::pbCurrentShm = NULL;

void* CSharedMem::operator new( size_t nSize )
{
	unsigned char* pTemp = NULL;

	if(!pbCurrentShm )
	{
		return (void*)NULL;
	}

	pTemp = pbCurrentShm;

	return (void * )pTemp;
}
void CSharedMem::operator delete(void* pMem )
{

}

CSharedMem::CSharedMem()
{
	m_pbCurrentSegMent = pbCurrentShm + sizeof(CSharedMem);
}
void CSharedMem::ResumeShm(int nKey, size_t nSize )
{
	m_pbCurrentSegMent = pbCurrentShm + sizeof(CSharedMem);
	if (nKey != m_nShmKey || nSize == m_nShmSize)
	{
		Initialize(nKey, nSize);
		return;
	}

	m_InitMode = Recover;    
}

CSharedMem::CSharedMem(int nKey, size_t nSize, bool bInitFlag )
{
	if( bInitFlag )
	{
		m_pbCurrentSegMent = pbCurrentShm + sizeof(CSharedMem);
		Initialize(nKey, nSize);
	}
	else
	{
		ResumeShm(nKey, nSize);
	}
}

CSharedMem::~CSharedMem()
{
}

int CSharedMem::Initialize(int nKey, size_t nSize )
{
	m_InitMode = Init;

	m_nShmKey = nKey;
	m_nShmSize = nSize;
	return 0;
}

void* CSharedMem::CreateSegment( size_t nSize )
{
	int nTempUsedLength = 0;
	unsigned char* pTemp;

	if(nSize <= 0 )
	{
		return NULL;
	}

	nTempUsedLength = ( int )(m_pbCurrentSegMent - (unsigned char * )this);
	if(m_nShmSize - nTempUsedLength < nSize )
	{
		return NULL;
	}

	pTemp = m_pbCurrentSegMent;
	m_pbCurrentSegMent += nSize;

	return (void * )pTemp;
}

EIMode CSharedMem::GetInitMode()
{
	return m_InitMode;
}
