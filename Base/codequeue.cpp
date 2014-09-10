#include "shm.h"
#include "codequeue.h"
#include <stdio.h>
CSharedMem* CCodeQueue::pCurrentShm = NULL;

//计算队列的大小
size_t CCodeQueue::CountQueueSize(int iBufSize)
{
	size_t iTempSize = 0;

	iTempSize += sizeof(CCodeQueue);
	if( iBufSize > 0 )
	{
		iTempSize += iBufSize;
	}

	return iTempSize;
}
//重载操作符
void* CCodeQueue::operator new(size_t nSize)
{
	unsigned char* pTemp;

	if( !pCurrentShm )
	{
		return (void*)NULL;
	}

	pTemp = (unsigned char *)(pCurrentShm->CreateSegment(nSize));

	return (void *)pTemp;
}
void CCodeQueue::operator delete(void *pBase)
{
}

//狗仔函数
CCodeQueue::CCodeQueue()
{
	m_stQueueHead.m_nBegin = 0;
	m_stQueueHead.m_nEnd = 0;
	m_stQueueHead.m_nSize = 0;
	m_stQueueHead.m_iCodeBufOffSet = 0;	

	if( (unsigned long)(&m_stQueueHead.m_nBegin)%sizeof(int) != 0 )
	{
		printf("begin addr -> codequeue error! must lock queue");
	}

	if( (unsigned long)(&m_stQueueHead.m_nEnd)%sizeof(int) != 0 )
	{
		printf("end addr -> codequeue error! must lock queue");
	}
}
CCodeQueue::CCodeQueue(int nTotalSize)
{
	if( pCurrentShm->GetInitMode() == Init )
	{
		Initialize( nTotalSize );
	}
	else
	{
		Resume(nTotalSize);
	}
}


CCodeQueue::~CCodeQueue()
{
}

int CCodeQueue::Initialize( int nTotalSize )
{
	unsigned char *pbyCodeBuf = NULL;

	m_stQueueHead.m_nSize = nTotalSize;
	m_stQueueHead.m_nBegin = 0;
	m_stQueueHead.m_nEnd = 0;

	pbyCodeBuf = (unsigned char *)(pCurrentShm->CreateSegment((size_t)nTotalSize));

	if( !pbyCodeBuf )
	{
		m_stQueueHead.m_iCodeBufOffSet = 0;
		return -1;
	}

	m_stQueueHead.m_iCodeBufOffSet = (int)((unsigned char *)pbyCodeBuf - (unsigned char *)this); 

	return 0;
}

int CCodeQueue::Resume(int nTotalSize)
{
	if( !pCurrentShm )
	{
		return -1;
	}

	pCurrentShm->CreateSegment((size_t)nTotalSize);

	return 0;
}

void CCodeQueue::GetCriticalData(int *piBeginIdx, int *piEndIdx)
{
	if( piBeginIdx )
	{
		*piBeginIdx = m_stQueueHead.m_nBegin;
	}
	if( piEndIdx )
	{
		*piEndIdx = m_stQueueHead.m_nEnd;
	}
}

void CCodeQueue::SetCriticalData(int iBeginIdx, int iEndIdx)
{
	if( iBeginIdx >= 0 && iBeginIdx < m_stQueueHead.m_nSize )
	{
		m_stQueueHead.m_nBegin = iBeginIdx;
	}
	if( iEndIdx >= 0 && iEndIdx < m_stQueueHead.m_nSize )
	{
		m_stQueueHead.m_nEnd = iEndIdx;
	}
}

int CCodeQueue::GetUsedLen(int &iTotalSize)
{
	int nUsedLen = 0;
	int nTempBegin = -1;
	int nTempEnd = -1;

	GetCriticalData( &nTempBegin, &nTempEnd );

	if( nTempBegin > nTempEnd )
	{
		nUsedLen = m_stQueueHead.m_nSize - (nTempBegin - nTempEnd);
	}
	else
	{
		nUsedLen = nTempEnd - nTempBegin;
	}
	iTotalSize = m_stQueueHead.m_nSize;
	return nUsedLen;
}

int CCodeQueue::IsQueueFull()
{
	int nTempMaxLength = 0;
	int nTempBegin = -1;
	int nTempEnd = -1;

	GetCriticalData( &nTempBegin, &nTempEnd );

	if( nTempBegin == nTempEnd )
	{
		nTempMaxLength = m_stQueueHead.m_nSize;
	}
	else if( nTempBegin > nTempEnd )
	{
		nTempMaxLength = nTempBegin - nTempEnd;		
	}
	else
	{
		nTempMaxLength = (m_stQueueHead.m_nSize - nTempEnd) + nTempBegin;
	}

	//重要：最大长度应该减去预留部分长度，保证首尾不会相接
	nTempMaxLength -= QUEUERESERVELENGTH;

	if( nTempMaxLength > 0 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

//Error code: -1 invalid para; -2 not enough; -3 data crashed
//This function only set the end idx of code queue
int CCodeQueue::AppendOneCode(const unsigned char *pInCode, int sInLength)
{
	int nTempMaxLength = 0;
	int nTempBegin = -1;
	int nTempEnd = -1;
	//char pcTempBuf[8192];
	unsigned char *pbyCodeBuf;

	unsigned char *pTempSrc = NULL;
	unsigned char *pTempDst = NULL;
	int i;
	int nTempRet;

	if( !pInCode || sInLength <= 0 )
	{
		return FAILED_SYSTEM;
	}
	if( m_stQueueHead.m_iCodeBufOffSet == 0 || m_stQueueHead.m_nSize <= 0 )
	{
		return FAILED_SYSTEM;
	}
	pbyCodeBuf = (unsigned char *)((unsigned char *)this + m_stQueueHead.m_iCodeBufOffSet);

	 //首先判断是否队列已满
	if( IsQueueFull() )        //if( m_stQueueHead.m_nFullFlag )
	{
		return FAILED_QUEUE_FULL;
	}

	GetCriticalData(&nTempBegin, &nTempEnd);

	if( nTempBegin < 0 || nTempBegin >= m_stQueueHead.m_nSize
		|| nTempEnd < 0 || nTempEnd >= m_stQueueHead.m_nSize )
	{
		printf("In CCodeQueue::AppendOneCode, data crashed: begin = %d, end = %d",
			nTempBegin, nTempEnd);

		nTempEnd = nTempBegin;
		SetCriticalData( -1, nTempEnd );

		return FAILED_SYSTEM;
	}

	if( nTempBegin == nTempEnd )
	{
		nTempMaxLength = m_stQueueHead.m_nSize;
	}
	else if( nTempBegin > nTempEnd )
	{
		nTempMaxLength = nTempBegin - nTempEnd;        
	}
	else
	{
		nTempMaxLength = (m_stQueueHead.m_nSize - nTempEnd) + nTempBegin;
	}

	//重要：最大长度应该减去预留部分长度，保证首尾不会相接
	nTempMaxLength -= QUEUERESERVELENGTH;

	if( (int)(sInLength + sizeof(int)) > nTempMaxLength )
	{
		return FAILED_QUEUE_FULL;
	}

	nTempRet = nTempEnd;

	pTempDst = &pbyCodeBuf[0];
	pTempSrc = (unsigned char *)&sInLength;

	for( i = 0; i < sizeof(sInLength); i++ )
	{
		pTempDst[nTempEnd] = pTempSrc[i];
		nTempEnd = (nTempEnd + 1) % m_stQueueHead.m_nSize;
	}

	if( nTempBegin > nTempEnd )
	{
		memcpy((void *)&pbyCodeBuf[nTempEnd], (const void *)pInCode, (size_t)sInLength);
	}
	else
	{
		if( (int)sInLength > (m_stQueueHead.m_nSize - nTempEnd) )
		{
			memcpy((void *)&pbyCodeBuf[nTempEnd], (const void *)&pInCode[0], (size_t)(m_stQueueHead.m_nSize - nTempEnd) );
			memcpy((void *)&pbyCodeBuf[0],(const void *)&pInCode[(m_stQueueHead.m_nSize - nTempEnd)], (size_t)(sInLength + nTempEnd - m_stQueueHead.m_nSize) );
		}
		else
		{
			memcpy((void *)&pbyCodeBuf[nTempEnd], (const void *)&pInCode[0], (size_t)sInLength);
		}
	}
	nTempEnd = (nTempEnd + sInLength) % m_stQueueHead.m_nSize;

	SetCriticalData( -1, nTempEnd );

	return nTempRet;
}


//This function just change the begin idx of code queue
int CCodeQueue::GetHeadCode(unsigned char *pOutCode, unsigned int *psOutLength)
{
	unsigned int nMaxBufferLen;
	int nTempMaxLength = 0;
	int nTempBegin = -1;
	int nTempEnd = -1;
	unsigned char *pTempSrc;
	unsigned char *pTempDst;
	int i;
	unsigned char *pbyCodeBuf;

	if( !pOutCode || !psOutLength )
	{
		if( psOutLength )
		{
			*psOutLength = 0;
		}
		return ERR_SYSTEM;
	}
	if( m_stQueueHead.m_iCodeBufOffSet == 0 || m_stQueueHead.m_nSize <= 0 )
	{
		*psOutLength = 0;
		return ERR_SYSTEM;
	}

	nMaxBufferLen = *psOutLength;

	pbyCodeBuf = (unsigned char *)((unsigned char *)this + m_stQueueHead.m_iCodeBufOffSet);

	GetCriticalData(&nTempBegin, &nTempEnd);

	if( nTempBegin == nTempEnd )
	{
		*psOutLength = 0;
		return ERR_FOR_NOPACKET;
	}

	if( nTempBegin < nTempEnd )
	{
		nTempMaxLength = nTempEnd - nTempBegin;
	}
	else
	{
		nTempMaxLength = m_stQueueHead.m_nSize - nTempBegin + nTempEnd;
	}

	if( nTempMaxLength < sizeof(int) )
	{
		*psOutLength = 0;
		nTempBegin = nTempEnd;
		SetCriticalData(nTempBegin, -1);
		return ERR_SYSTEM;
	}

	pTempDst = (unsigned char *)psOutLength;

	pTempSrc = (unsigned char *)&pbyCodeBuf[0];
	for( i = 0; i < sizeof(int); i++ )
	{
		pTempDst[i] = pTempSrc[nTempBegin];
		nTempBegin = (nTempBegin+1) % m_stQueueHead.m_nSize; 
	}

	if( (*psOutLength) > (int)(nTempMaxLength - sizeof(int)) || *psOutLength < 0 )
	{
		*psOutLength = 0;
		nTempBegin = nTempEnd;
		SetCriticalData(nTempBegin, -1);
		return ERR_SYSTEM;
	}
	if( (*psOutLength) > nMaxBufferLen )
	{
		*psOutLength = 0;
		return ERR_FOR_SMALLBUFF;
	}

	pTempDst = (unsigned char *)&pOutCode[0];

	if( nTempBegin < nTempEnd )
	{
		memcpy((void *)pTempDst, (const void *)&pTempSrc[nTempBegin], (size_t)(*psOutLength));
	}
	else
	{
		//如果出现分片，则分段拷贝
		if( m_stQueueHead.m_nSize - nTempBegin < (int)(*psOutLength) )
		{
			memcpy((void *)pTempDst, (const void *)&pTempSrc[nTempBegin], (size_t)(m_stQueueHead.m_nSize - nTempBegin));
			pTempDst += (m_stQueueHead.m_nSize - nTempBegin);
			memcpy((void *)pTempDst, (const void *)&pTempSrc[0], (size_t)(*psOutLength+nTempBegin-m_stQueueHead.m_nSize));
		}
		else    //否则，直接拷贝
		{
			memcpy((void *)pTempDst, (const void *)&pTempSrc[nTempBegin], (size_t)(*psOutLength));
		}
	}
	nTempBegin = (nTempBegin + (*psOutLength)) % m_stQueueHead.m_nSize;

	SetCriticalData(nTempBegin, -1);

	return 0;
}

int CCodeQueue::GetOneCode(int iCodeOffset, int nCodeLength, unsigned char *pOutCode, unsigned int *psOutLength)
{
	int nTempShort = 0;
	int iTempMaxLength = 0;
	int iTempBegin;
	int iTempEnd;
	int nTempFullFlag = -1;
	int iTempIdx;
	unsigned char *pTempSrc;
	unsigned char *pTempDst;
	unsigned char *pbyCodeBuf;

	if( m_stQueueHead.m_iCodeBufOffSet == 0 || m_stQueueHead.m_nSize <= 0 )
	{
		return -1;
	}

	pbyCodeBuf = (unsigned char *)((unsigned char *)this + m_stQueueHead.m_iCodeBufOffSet);

	if( !pOutCode || !psOutLength )
	{
		printf("In GetOneCode, invalid input paraments.\n");
		return -1;
	}

	if( iCodeOffset < 0 || iCodeOffset >= m_stQueueHead.m_nSize)
	{
		printf("In GetOneCode, invalid code offset %d.\n", iCodeOffset);
		return -1;
	}
	if( nCodeLength < 0 || nCodeLength >= m_stQueueHead.m_nSize )
	{
		printf("In GetOneCode, invalid code length %d.\n", nCodeLength);
		return -1;
	}

	GetCriticalData(&iTempBegin, &iTempEnd);

	if( iTempBegin == iTempEnd )
	{
		*psOutLength = 0;
		return 0;
	}

	if( iTempBegin == iCodeOffset )
	{
		return GetHeadCode(pOutCode, psOutLength);
	}

	printf("Warning: Get code is not the first one, there might be sth wrong.\n");

	if( iCodeOffset < iTempBegin || iCodeOffset >= iTempEnd )
	{
		printf("In GetOneCode, code offset out of range.\n");
		*psOutLength = 0;
		return -1;
	}

	if( iTempBegin < iTempEnd )
	{
		iTempMaxLength = iTempEnd - iTempBegin;        
	}
	else
	{
		iTempMaxLength = m_stQueueHead.m_nSize - iTempBegin + iTempEnd;
	}

	if( iTempMaxLength < sizeof(int) )
	{
		*psOutLength = 0;
		iTempBegin = iTempEnd;
		SetCriticalData(iTempBegin, -1);
		return -3;
	}

	pTempDst = (unsigned char *)&nTempShort;
	pTempSrc = (unsigned char *)&pbyCodeBuf[0];
	iTempIdx = iCodeOffset;
	for( int i = 0; i < sizeof(int); i++ )
	{
		pTempDst[i] = pTempSrc[iTempIdx];
		iTempIdx = (iTempIdx+1) % m_stQueueHead.m_nSize; 
	}

	if( nTempShort > (int)(iTempMaxLength - sizeof(int)) || nTempShort < 0 || nTempShort != nCodeLength )
	{
		printf("Can't get code, code length not matched.\n");
		*psOutLength = 0;
		return -2;
	}

	//SetBeginIdx( iCodeOffset );
	SetCriticalData(iCodeOffset, -1);

	return GetHeadCode( pOutCode, psOutLength );
}

bool CCodeQueue::IsQueueEmpty()
{
	int iTempBegin;
	int iTempEnd;
	int nTempFullFlag = -1;

	GetCriticalData(&iTempBegin, &iTempEnd);
	if( iTempBegin == iTempEnd )
	{
		return true;
	}

	return false;
}

int CCodeQueue::DumpToFile(const char *szFileName)
{
	unsigned char* pbyCodeBuffer = (unsigned char *)((unsigned char *)this + m_stQueueHead.m_iCodeBufOffSet);
	if( !szFileName || !pbyCodeBuffer )
	{
		return -1;
	}

	FILE *fpDumpFile = fopen(szFileName, "w");
	int iPageSize = 4096, iPageCount = 0, i;
	unsigned char *pPage = pbyCodeBuffer;

	if( !fpDumpFile )
	{
		return -1;
	}

	fwrite((const void *)&m_stQueueHead, sizeof(m_stQueueHead), 1, fpDumpFile);
	iPageCount = m_stQueueHead.m_nSize/iPageSize;
	for( i = 0; i < iPageCount; i++ )
	{
		fwrite((const void *)pPage, iPageSize, 1, fpDumpFile);
		pPage += iPageSize;
	}
	fwrite((const void *)pPage, m_stQueueHead.m_nSize - iPageSize*iPageCount, 1, fpDumpFile);

	fclose(fpDumpFile);

	return 0;
}

int CCodeQueue::LoadFromFile(const char *szFileName)
{
	unsigned char* pbyCodeBuffer = (unsigned char *)((unsigned char *)this + m_stQueueHead.m_iCodeBufOffSet);
	if( !szFileName || !pbyCodeBuffer )
	{
		return -1;
	}

	FILE *fpDumpFile = fopen(szFileName, "r");
	int iPageSize = 4096, iPageCount = 0, i;
	unsigned char *pPage = pbyCodeBuffer;

	if( !fpDumpFile )
	{
		return -1;
	}

	fread((void *)&m_stQueueHead, sizeof(m_stQueueHead), 1, fpDumpFile);
	iPageCount = m_stQueueHead.m_nSize/iPageSize;
	for( i = 0; i < iPageCount; i++ )
	{
		fread((void *)pPage, iPageSize, 1, fpDumpFile);
		pPage += iPageSize;
	}
	fread((void *)pPage, m_stQueueHead.m_nSize - iPageSize*iPageCount, 1, fpDumpFile);

	fclose(fpDumpFile);

	return 0;	
}

int CCodeQueue::CleanQueue()
{
	m_stQueueHead.m_nBegin = 0;
	m_stQueueHead.m_nEnd = 0;

	return 0;
}


