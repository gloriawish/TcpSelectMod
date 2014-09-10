#ifndef __TinyTcp_h__
#define __TinyTcp_h__

#include "common.h"
#include "codequeue.h"
using namespace Common;
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"wsock32")
#define  MAX_MSG_SIZE 1024			/*����BUFFsize*/
#define  IP_LEN 20					/*IP��ַ�ĳ���*/
#define  MAX_BUFF_LEN   0x80000		/*TinyTcp��Ϣ������*/

#define  RECV_BUFF_LEN 1024			/*socketinfo ������Ϣ������*/
#define  SEND_BUFF_LEN 1024			/*socketinfo ������Ϣ������*/
#define  LISTEN_PORT_NUM 10	
#define	 MAX_PORT_NUM	 10
#define  PIPESIZE		0x1000000
#define  RECV_DATA       1                          /*��־socketҪ��������*/
#define  SEND_DATA       2                          /*��־socketҪ��������*/
#define  CHECK_TIME       5
#define  MAX_SEND_PACKET  10
#pragma pack(1)
typedef struct _Config								/*�����������Ϣ*/
{
	int          socketTimeOut;						/*socket�ĳ�ʱʱ��*/
	int			 connTimeOut;
	int          listenPortNum;						/*����˿���Ϣ*/    
	int          listenPorts[MAX_PORT_NUM];         /*����˿���Ϣ*/     
	int			 tcpBuffLen;
	int			 maxConnectNum;						/*���������*/
	
	int			 WritePipeKey;
	int			 ReadPipeKey;
}Config;


/*SOCKET������*/
typedef enum _SocketType{
	ListenSocket=1,
	ConnectSocket=2
}SocketType;

/*��������û�����������SOCKET��Ϣ��*/
typedef struct _SocketInfo{
	SOCKET	   socketHandle;			/*SOCKET���*/
	SocketType socketType;				/*socket���ͣ�����socket������socket*/
	int        isRecv;					/*�Ƿ��հ�,SOCKET��־*/
	int        isSend;					/*�Ƿ񷢰�,SOCKET��־*/
	
	time_t     createTime;				/*����ʱ��*/
	time_t	   timeStamp;				/*ʱ���*/
	long       uin;						/*SOCKET��Ψһ��ʾ*/
	
	int		   maxSocketIndex;			/*�������*/
	int        socketUsedCount;			/*�Ѿ�ʹ���˵�socket����*/
	int        socketIndex;				/*socket������*/
	int		   nextIndex;				/*��һ��scoket������*/

	short      clientPort;
	char       clientIp[20];			/*�ͻ���IP*/
	long	   clientAddr;				/*�ͻ��˵�ַ*/
	int		   connectPort;				/*���ӵĶ˿�*/
	
	char       recvBuff[RECV_BUFF_LEN];	/*�������ݵ�BUFF*/
	int        recvSize;				/*�Ѿ����յĳ��ȣ��൱��ƫ��λ��*/	
}SocketInfo;


/*���������*/
typedef enum _CommonType
{
	Login=1,
	Logout=2,
	Add=3,
}CommonType;
/*��Ϣ������*/
typedef enum _MsgType
{
	ClientMsg=1,
	ServerMsg=2,
	ProxyMsg=3,
	SysMsg=4
}MsgType;
/*
{name:zhujun,age:12}
*/
typedef struct  _TcpMsgPacket				/*�Զ��������ݴ�С������ĳ��ȣ�����ƫ��*/
{
	long			sequence;				/*���ݰ���Ψһ��ʾ��*/
	long			uin;					/*ÿһ���������ݷ���Ψһ��ʶ��*/
	time_t			sendTime;				/*���͵�ʱ��*/
	time_t			createTime;				/*��������ʱ��*/
	CommonType		commonType;				/*��������*/
	MsgType			msgType;				/*��������*/
	char			hostIP[IP_LEN];			/*��������������P*/
	char			desIP[IP_LEN];			/*Ŀ��������IP*/
	char			msg[MAX_MSG_SIZE];		/*���ݰ��ľ������� -----����json���ݸ�ʽ*/
	int GetSize()
	{
		return sizeof(sequence)+sizeof(uin)+sizeof(sendTime)+sizeof(createTime)+sizeof(commonType)+sizeof(msgType)+sizeof(hostIP)+sizeof(desIP)+strlen(msg);
	}
	_TcpMsgPacket()
	{
		memset(hostIP,'\0',sizeof(hostIP));
		memset(desIP,'\0',sizeof(desIP));
		memset(msg,'\0',sizeof(msg));
	}
}TcpMsgPacket;


typedef struct _ConnectPara{
	sockaddr_in other_addr;
	int			uin;//��ʶÿһ�����ӵ�
}ConnectPara;

#pragma pack()

/*��װ���ݵĺ���*/
static int EncodeMsg(TcpMsgPacket &msg,char *buff)
{

	int size=msg.GetSize();
	char * temp=buff;

	memcpy(buff, (char*)&size, sizeof(size));/*ǰ4���ֽڴ�Ű�����*/
	buff+=sizeof(size);

	memcpy(buff, (char*)&msg.sequence, sizeof(msg.sequence));
	buff+=sizeof(msg.sequence);

	memcpy(buff, (char*)&msg.uin, sizeof(msg.uin));
	buff+=sizeof(msg.uin);

	memcpy(buff, (char*)&msg.sendTime, sizeof(msg.sendTime));
	buff+=sizeof(msg.sendTime);

	memcpy(buff, (char*)&msg.createTime, sizeof(msg.createTime));
	buff+=sizeof(msg.createTime);

	memcpy(buff, (char*)&msg.commonType, sizeof(msg.commonType));
	buff+=sizeof(msg.commonType);

	memcpy(buff, (char*)&msg.msgType, sizeof(msg.msgType));
	buff+=sizeof(msg.msgType);

	//�����￪ʼ���ݳ��Ⱦ��ǲ�һ������
	memcpy(buff, (char*)&msg.hostIP, sizeof(msg.hostIP));
	buff+=sizeof(msg.hostIP);

	memcpy(buff, (char*)&msg.desIP, sizeof(msg.desIP));
	buff+=sizeof(msg.desIP);

	memcpy(buff, (char*)&msg.msg, strlen(msg.msg));

	buff=temp;
	return size+4;//֮����Ҫ+4����Ϊ����ͷ����װ��һ��4�ֽڱ�ʾ���ȵ�
}

/*���װ���ݺ���,���ص�ֵ�����ݰ��Ĵ�С*/
static int DecodeMsg(char *inBuff,TcpMsgPacket &msg)
{
	char * temp=inBuff;
	
	int size=0;
	int ret;
	memcpy((int *)&size, inBuff, sizeof(size));/*��ǰ4���ֽڶ�ȡ���������ֵ�ú��������ݰ��ĳ���*/
	inBuff+=sizeof(size);
	ret=size;
	memcpy((char*)&msg.sequence, inBuff, sizeof(msg.sequence));
	inBuff+=sizeof(msg.sequence);
	size-=sizeof(msg.sequence);

	memcpy((char*)&msg.uin, inBuff, sizeof(msg.uin));
	inBuff+=sizeof(msg.uin);
	size-=sizeof(msg.uin);

	memcpy((char*)&msg.sendTime, inBuff, sizeof(msg.sendTime));
	inBuff+=sizeof(msg.sendTime);
	size-=sizeof(msg.sendTime);

	memcpy((char*)&msg.createTime, inBuff, sizeof(msg.createTime));
	inBuff+=sizeof(msg.createTime);
	size-=sizeof(msg.createTime);

	memcpy((char*)&msg.commonType, inBuff, sizeof(msg.commonType));
	inBuff+=sizeof(msg.commonType);
	size-=sizeof(msg.commonType);

	memcpy((char*)&msg.msgType, inBuff, sizeof(msg.msgType));
	inBuff+=sizeof(msg.msgType);
	size-=sizeof(msg.msgType);

	//�����￪ʼ���ݳ��Ⱦ��ǲ�һ������
	memcpy((char*)&msg.hostIP,inBuff, sizeof(msg.hostIP));
	inBuff+=sizeof(msg.hostIP);
	size-=sizeof(msg.hostIP);

	memcpy( (char*)&msg.desIP, inBuff, sizeof(msg.desIP));
	inBuff+=sizeof(msg.desIP);
	size-=sizeof(msg.desIP);

	memset((char*)&msg.msg,'\0',sizeof(msg.msg));//ȫ����Ϊ0
	memcpy( (char*)&msg.msg, inBuff, size);
	size-=strlen(msg.msg);
	
	inBuff=temp;
	return ret+4;//����ʱ������ͷ����ʾ���ȵ��ĸ��ֽ�û���� ����Ҫ�������ĸ��ֽ�
}

static int GetIndexByUIN(SocketInfo *socketArray,int len,const long keyuin)
{
	for(int i=0;i<len;i++)
	{
		if(socketArray[i].uin==keyuin)
		{
			return i;
		}
	}
	return -1;
}




/*����TCP�ײ�ͨѶ����*/
class TinyTcp 
{

public:
	int Initialize();
	int StartSerivce();
	int ConnectOther(ConnectPara para);								/*���������������������ܽ���ͨѶ*/
public:
	TinyTcp();														/*���캯��*/
	TinyTcp(Config cfg);	
	~TinyTcp();														/*��������*/
private:  
	int  CreatePipe();												/*�����ڴ�ܵ���Ӧ�ð���Write and Read ��������*/		

	int  CheckTimeOut();											/*���ͻ���socket�Ƿ�ʱ*/       
	int  GetExMessage();											/*�����ⲿ����*/
	int  InitListenSocket(short ListenPort);						/*��ʼ������socket*/
	int  RecvClientData(short Index);								/*���տͻ��˷��͵�����*/
	int  SendClientData(char* buff, int len);						/*�������ݵ��ͻ���*/        
	int  SendSocketData(SOCKET socketHandle, char *sendBuff, int sendLen);/*��������*/
	int  FindSocketInfo(long uin);									/*ͨ��uin�ҵ�SocketInfo��index*/
	int  CreateSocketInfo();										/*����һ��SocketInfo*/
	void DeleteSocketInfo(short nindex);							/*����һ��SOcketInfo*/
	void ClearSocketInfo();
	Config	   zconfig;												/*������Ϣ*/
	
	int CheckWaitSendData();

	

private:
	int OnFrameAccepted(SOCKET clientSocket);
	int OnFrameClientMessage(int index, int recvlen);

	int                 maxSocketIndex;
	int					socketUsedCount;
	int					checkBeginIndex;
	short				clientCount;
	time_t              lastTime;							/*�ϴμ�ⳬʱ��ʱ��*/
	time_t              nowTime;							/*��ǰʱ��*/
	struct  timeval     timeWait;							/*�����¼���ʱ����*/ 
	SOCKET              listenSocket;						/*����socket*/        
	struct sockaddr_in  sockAddr;							/*�����ַ*/
	int					beginSocketIndex;
	SocketInfo          * socketInfo;						/*socketinfo�ṹ���飬���ڼ�¼�ͻ��˵���Ϣ*/        
	SocketInfo          * nowSocketInfo;					/*��ǰ��socket�ṹָ��*/
	CCodeQueue          * codeQueueWrite;					/*д���ݶ��У���һ����������ȡ����*/
	CCodeQueue          * codeQueueRead;					/*�����ݶ��У���һ�㽫Ҫ���͵�����д������*/
	CCodeQueue          * m_PipeWrite;
	char                msgBuff[MAX_BUFF_LEN];			    /*��Ϣ������*/
	
	//char*             allRecvBuff;						/*���н��ܵ�������*/
	int                 timeout;							/*��ʱ*/
	//int				recvClientBuffLen;					/*���յ�Client���ݵĳ���*/	


private:
	fd_set              readFds;							/*������*/ 
	fd_set              writeFds;							/*д����*/

private:
	bool writePipeExist;
	bool readPipeExist;
};

#endif //__TinyTcp_h__