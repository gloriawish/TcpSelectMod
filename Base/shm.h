
#include <time.h>
//#define YEARSEC 31536000

typedef enum
{
	Init = 0,
	Recover = 1
} EIMode;

class CSharedMem
{
public:
	CSharedMem();
	CSharedMem(int nKey, size_t nSize, bool bInitFlag);
	~CSharedMem();


	int Initialize(int nKey, size_t nSize);
	void* operator new( size_t nSize);
	void  operator delete(void *pMem);
	void*  CreateSegment( size_t nSize);
	static unsigned char* pbCurrentShm;
	EIMode GetInitMode();


private:
	void ResumeShm(int nKey, size_t nSize);
	int m_nShmKey;
	size_t m_nShmSize;

	unsigned char* m_pbCurrentSegMent;

	EIMode m_InitMode;
};
