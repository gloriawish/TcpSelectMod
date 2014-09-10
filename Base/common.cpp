#include "common.h"
#define LOCK_WAIT_TIME 1000L
using namespace std;
using namespace Common;
//���������ڴ�ͨѶ�ĺ���
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


	// 1 �޸ı����̵ķ���Ȩ��
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, TRUE);
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle       = FALSE;
	sa.lpSecurityDescriptor = &sd;

	// 2 �����������
	HANDLE hMutex;
	TCHAR sEngineName[MAX_PATH] = {0};

	_stprintf_s(sEngineName, _T("%s_BBB"), A2T((LPSTR)lpszEngineName));
	hMutex = ::CreateMutex(&sa, FALSE, sEngineName);
	nRet = GetLastError();

	// 3 ���������ڴ�
	hFileMapping = CreateFileMapping(
		//		(HANDLE)0xFFFFFFFF,		// 64λ�²���ʹ��
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
		printf(("�ȴ��ź�[%ld]ʧ��:error=[%d]\n"), hMutex, GetLastError());
		return NULL;
	}

	// 4 �����鹲���ڴ����´�����,�����ж�ָ������Ϊ-1,���Լ��Ķ�ָ��ָ��0λ����
	if (ERROR_SUCCESS == nRet)
	{		
		memset(pShareMem, 0, iBufferSize);
		//sprintf(szBuff, "share memory %s is created\n", lpszEngineName);
		//printf(szBuff);
		bMemExist = false;
	}
	// �������ڴ����Ѿ������ģ�����ֻ�Ǵ򿪣����Լ��Ķ�ָ������Ϊдָ��
	else if (ERROR_ALREADY_EXISTS == nRet)
	{
		//sprintf(szBuff, "share memory %s is already exist! attach it!\n", lpszEngineName);
		//printf(szBuff);
		bMemExist = true;
	}

	::ReleaseMutex(hMutex);//�ͷ��ź���

	CSharedMem::pbCurrentShm = pShareMem;
	iKey++;
	if( bMemExist == false )
	{
		bInitMode = true;
	}
	return new CSharedMem(iKey, iBufferSize, bInitMode);
}

// ��ģʽ��T��next����ֵ���������� next��
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
/*KMPģʽƥ���㷨*/
int Str::KMP(const char *Text,const char* Pattern) //const ��ʾ�����ڲ�����ı����������ֵ��
{
	if( !Text||!Pattern||  Pattern[0]=='\0'  ||  Text[0]=='\0' )//
		return -1;//��ָ���մ�������-1��
	int len=0;
	const char * c=Pattern;
	while(*c++!='\0')//�ƶ�ָ����ƶ��±�졣
	{    
		++len;//�ַ������ȡ�
	}
	int *next=new int[len+1];
	get_nextval(Pattern,next);//��Pattern��next����ֵ

	int index=0,i=0,j=0;
	while(Text[i]!='\0'  && Pattern[j]!='\0' )
	{
		if(Text[i]== Pattern[j])
		{
			++i;// �����ȽϺ���ַ�
			++j;
		}
		else
		{
			index += j-next[j];
			if(next[j]!=-1)
				j=next[j];// ģʽ�������ƶ�
			else
			{
				j=0;
				++i;
			}
		}
	}//while

	delete []next;
	if(Pattern[j]=='\0')
		return index;// ƥ��ɹ�
	else
		return -1;      
}