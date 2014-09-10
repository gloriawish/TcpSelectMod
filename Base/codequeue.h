#ifndef __CCodeQueue_h__
#define __CCodeQueue_h__
#define _CODEQUEUE_H_
#define QUEUERESERVELENGTH		8	//预留部分的长度
#define CODE_SUCC               0
#define ERR_SYSTEM              -9999
#define ERR_FOR_NOPACKET        -1
#define ERR_FOR_SMALLBUFF       -2
#define ERR_APPEND_NOBUFF       -3

#define FAILED_SYSTEM           ERR_SYSTEM
#define FAILED_QUEUE_FULL       ERR_APPEND_NOBUFF  
#include <string>

class CCodeQueue
{
public:
	CCodeQueue();
	CCodeQueue(int nTotalSize);
	~CCodeQueue();

	void* operator new( size_t nSize );
	void  operator delete( void *pBase );

	int Initialize( int nTotalSize );
	int AppendOneCode(const unsigned char *pInCode, int sInLength);
	int GetHeadCode(unsigned char *pOutCode, unsigned int *psOutLength);
	int GetOneCode(int iCodeOffset, int nCodeLength, unsigned char *pOutCode, unsigned int *psOutLength);
	int DumpToFile(const char *szFileName);
	int LoadFromFile(const char *szFileName);
	int CleanQueue();
	int GetUsedLen(int &iTotalSize);

	static size_t CountQueueSize(int iBufSize);
	static CSharedMem *pCurrentShm;

	bool IsQueueEmpty();

private:
	int Resume(int nTotalSize);
	int IsQueueFull();
	int SetFullFlag( int iFullFlag );

	void GetCriticalData(int *piBeginIdx, int *piEndIdx);
	void SetCriticalData(int iBeginIdx, int iEndIdx);

	struct _tagHead
	{
		int m_nSize;
		int m_iCodeBufOffSet;
		int m_nBegin;
		int m_nEnd;
	} m_stQueueHead;
};

#endif 