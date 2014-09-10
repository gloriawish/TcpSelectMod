#include "common.h"
#define LOCK_WAIT_TIME 1000L
using namespace std;
using namespace Common;
//创建共享内存通讯的函数
CSharedMem* base::CreateShareMem(unsigned int nShmKey, size_t iSize, bool bInitMode)
{
	char lpszEngineName[20];
	sprintf_s(lpszEngineName, "%u", nShmKey);

	int m_nIndex = 0;

	BOOL				bReturn = FALSE;
	int					nMemSize = 0;
	SECURITY_ATTRIBUTES	sa = {0};
	SECURITY_DESCRIPTOR	sd = {0};
	int					nRet = 0;
	int					i = 0;
	int iKey = 1;

	size_t iBufferSize = iSize;
	iBufferSize += sizeof(CSharedMem);

	bool bMemExist;
	HANDLE hFileMapping;


	// 1 修改本进程的访问权限
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, TRUE);
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle       = FALSE;
	sa.lpSecurityDescriptor = &sd;

	// 2 创建互斥对象
	HANDLE hMutex;
	TCHAR sEngineName[MAX_PATH] = {0};

	_stprintf_s(sEngineName, _T("%s_BBB"), A2T((LPSTR)lpszEngineName));
	hMutex = ::CreateMutex(&sa, FALSE, sEngineName);
	nRet = GetLastError();

	// 3 创建共享内存
	hFileMapping = CreateFileMapping(
		//		(HANDLE)0xFFFFFFFF,		// 64位下不能使用
		INVALID_HANDLE_VALUE,
		&sa, 
		PAGE_READWRITE, 
		0, 
		iBufferSize, 
		A2T((LPSTR)lpszEngineName)
		);

	nRet = GetLastError();
	if ((INVALID_HANDLE_VALUE == hFileMapping) || (NULL == hFileMapping))
	{
		CloseHandle(hFileMapping);
		hFileMapping = INVALID_HANDLE_VALUE;
		return NULL;
	}

	unsigned char* pShareMem = (unsigned char*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == pShareMem)
	{
		DWORD dwErr = GetLastError ();
		return NULL;
	}
	
	if (::WaitForSingleObject(hMutex, LOCK_WAIT_TIME) != WAIT_OBJECT_0)
	{
		::ReleaseMutex(hMutex);
		printf(("等待信号[%ld]失败:error=[%d]\n"), hMutex, GetLastError());
		return NULL;
	}

	// 4 如果这块共享内存是新创建的,将所有读指针设置为-1,将自己的读指针指到0位置上
	if (ERROR_SUCCESS == nRet)
	{		
		memset(pShareMem, 0, iBufferSize);
		//sprintf(szBuff, "share memory %s is created\n", lpszEngineName);
		//printf(szBuff);
		bMemExist = false;
	}
	// 如果这块内存是已经创建的，现在只是打开，则将自己的读指针设置为写指针
	else if (ERROR_ALREADY_EXISTS == nRet)
	{
		//sprintf(szBuff, "share memory %s is already exist! attach it!\n", lpszEngineName);
		//printf(szBuff);
		bMemExist = true;
	}

	::ReleaseMutex(hMutex);//释放信号量

	CSharedMem::pbCurrentShm = pShareMem;
	iKey++;
	if( bMemExist == false )
	{
		bInitMode = true;
	}
	return new CSharedMem(iKey, iBufferSize, bInitMode);
}

// 求模式串T的next函数值并存入数组 next。
void Str::get_nextval(const char *T, int next[])
{
	int j = 0, k = -1;
	next[0] = -1;
	while ( T[j/*+1*/] != '\0' )
	{
		if (k == -1 || T[j] == T[k])
		{
			++j; ++k;
			if (T[j]!=T[k])
				next[j] = k;
			else
				next[j] = next[k];
		}// if
		else
			k = next[k];
	}

}
/*KMP模式匹配算法*/
int Str::KMP(const char *Text,const char* Pattern) //const 表示函数内部不会改变这个参数的值。
{
	if( !Text||!Pattern||  Pattern[0]=='\0'  ||  Text[0]=='\0' )//
		return -1;//空指针或空串，返回-1。
	int len=0;
	const char * c=Pattern;
	while(*c++!='\0')//移动指针比移动下标快。
	{    
		++len;//字符串长度。
	}
	int *next=new int[len+1];
	get_nextval(Pattern,next);//求Pattern的next函数值

	int index=0,i=0,j=0;
	while(Text[i]!='\0'  && Pattern[j]!='\0' )
	{
		if(Text[i]== Pattern[j])
		{
			++i;// 继续比较后继字符
			++j;
		}
		else
		{
			index += j-next[j];
			if(next[j]!=-1)
				j=next[j];// 模式串向右移动
			else
			{
				j=0;
				++i;
			}
		}
	}//while

	delete []next;
	if(Pattern[j]=='\0')
		return index;// 匹配成功
	else
		return -1;      
}