#ifndef __TinyTcp_h__
#define __TinyTcp_h__

#include "common.h"
#include "codequeue.h"
using namespace Common;
#pragma comment(lib,"Base.lib")
#pragma comment(lib,"wsock32")
#define  MAX_MSG_SIZE 1024			/*最大的BUFFsize*/
#define  IP_LEN 20					/*IP地址的长度*/
#define  MAX_BUFF_LEN   0x80000		/*TinyTcp消息缓冲区*/

#define  RECV_BUFF_LEN 1024			/*socketinfo 接收消息缓冲区*/
#define  SEND_BUFF_LEN 1024			/*socketinfo 发送消息缓冲区*/
#define  LISTEN_PORT_NUM 10	
#define	 MAX_PORT_NUM	 10
#define  PIPESIZE		0x1000000
#define  RECV_DATA       1                          /*标志socket要接收数据*/
#define  SEND_DATA       2                          /*标志socket要发送数据*/
#define  CHECK_TIME       5
#define  MAX_SEND_PACKET  10
#pragma pack(1)
typedef struct _Config								/*程序的配置信息*/
{
	int          socketTimeOut;						/*socket的超时时间*/
	int			 connTimeOut;
	int          listenPortNum;						/*保存端口信息*/    
	int          listenPorts[MAX_PORT_NUM];         /*保存端口信息*/     
	int			 tcpBuffLen;
	int			 maxConnectNum;						/*最大连接数*/
	
	int			 WritePipeKey;
	int			 ReadPipeKey;
}Config;


/*SOCKET的类型*/
typedef enum _SocketType{
	ListenSocket=1,
	ConnectSocket=2
}SocketType;

/*用来存放用户连接上来的SOCKET信息的*/
typedef struct _SocketInfo{
	SOCKET	   socketHandle;			/*SOCKET句柄*/
	SocketType socketType;				/*socket类型：监听socket、连接socket*/
	int        isRecv;					/*是否收包,SOCKET标志*/
	int        isSend;					/*是否发包,SOCKET标志*/
	
	time_t     createTime;				/*创建时间*/
	time_t	   timeStamp;				/*时间戳*/
	long       uin;						/*SOCKET的唯一标示*/
	
	int		   maxSocketIndex;			/*最大索引*/
	int        socketUsedCount;			/*已经使用了的socket数量*/
	int        socketIndex;				/*socket的索引*/
	int		   nextIndex;				/*下一个scoket的索引*/

	short      clientPort;
	char       clientIp[20];			/*客户端IP*/
	long	   clientAddr;				/*客户端地址*/
	int		   connectPort;				/*连接的端口*/
	
	char       recvBuff[RECV_BUFF_LEN];	/*接收数据的BUFF*/
	int        recvSize;				/*已经接收的长度，相当于偏移位置*/	
}SocketInfo;


/*命令的种类*/
typedef enum _CommonType
{
	Login=1,
	Logout=2,
	Add=3,
}CommonType;
/*消息的类型*/
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
typedef struct  _TcpMsgPacket				/*自动根据内容大小计算包的长度，好做偏移*/
{
	long			sequence;				/*数据包的唯一标示符*/
	long			uin;					/*每一个发送数据方的唯一标识符*/
	time_t			sendTime;				/*发送的时间*/
	time_t			createTime;				/*创建包的时间*/
	CommonType		commonType;				/*命令类型*/
	MsgType			msgType;				/*数据类型*/
	char			hostIP[IP_LEN];			/*发送数据主机的P*/
	char			desIP[IP_LEN];			/*目的主机的IP*/
	char			msg[MAX_MSG_SIZE];		/*数据包的具体内容 -----采用json数据格式*/
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
	int			uin;//标识每一个连接的
}ConnectPara;

#pragma pack()

/*封装数据的函数*/
static int EncodeMsg(TcpMsgPacket &msg,char *buff)
{

	int size=msg.GetSize();
	char * temp=buff;

	memcpy(buff, (char*)&size, sizeof(size));/*前4个字节存放包长度*/
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

	//从这里开始数据长度就是不一样的了
	memcpy(buff, (char*)&msg.hostIP, sizeof(msg.hostIP));
	buff+=sizeof(msg.hostIP);

	memcpy(buff, (char*)&msg.desIP, sizeof(msg.desIP));
	buff+=sizeof(msg.desIP);

	memcpy(buff, (char*)&msg.msg, strlen(msg.msg));

	buff=temp;
	return size+4;//之所以要+4是因为我在头部封装了一个4字节表示长度的
}

/*解封装数据函数,返回的值是数据包的大小*/
static int DecodeMsg(char *inBuff,TcpMsgPacket &msg)
{
	char * temp=inBuff;
	
	int size=0;
	int ret;
	memcpy((int *)&size, inBuff, sizeof(size));/*把前4个字节读取出来，这个值得含义是数据包的长度*/
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

	//从这里开始数据长度就是不一样的了
	memcpy((char*)&msg.hostIP,inBuff, sizeof(msg.hostIP));
	inBuff+=sizeof(msg.hostIP);
	size-=sizeof(msg.hostIP);

	memcpy( (char*)&msg.desIP, inBuff, sizeof(msg.desIP));
	inBuff+=sizeof(msg.desIP);
	size-=sizeof(msg.desIP);

	memset((char*)&msg.msg,'\0',sizeof(msg.msg));//全部置为0
	memcpy( (char*)&msg.msg, inBuff, size);
	size-=strlen(msg.msg);
	
	inBuff=temp;
	return ret+4;//解析时，还有头部表示长度的四个字节没解析 所以要加上这四个字节
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




/*用于TCP底层通讯的类*/
class TinyTcp 
{

public:
	int Initialize();
	int StartSerivce();
	int ConnectOther(ConnectPara para);								/*连接其他主机，这样才能进行通讯*/
public:
	TinyTcp();														/*构造函数*/
	TinyTcp(Config cfg);	
	~TinyTcp();														/*析构函数*/
private:  
	int  CreatePipe();												/*创建内存管道，应该包括Write and Read 的两部分*/		

	int  CheckTimeOut();											/*检测客户端socket是否超时*/       
	int  GetExMessage();											/*接收外部数据*/
	int  InitListenSocket(short ListenPort);						/*初始化监听socket*/
	int  RecvClientData(short Index);								/*接收客户端发送的数据*/
	int  SendClientData(char* buff, int len);						/*发送数据到客户端*/        
	int  SendSocketData(SOCKET socketHandle, char *sendBuff, int sendLen);/*发送数据*/
	int  FindSocketInfo(long uin);									/*通过uin找到SocketInfo的index*/
	int  CreateSocketInfo();										/*创建一个SocketInfo*/
	void DeleteSocketInfo(short nindex);							/*销毁一个SOcketInfo*/
	void ClearSocketInfo();
	Config	   zconfig;												/*配置信息*/
	
	int CheckWaitSendData();

	

private:
	int OnFrameAccepted(SOCKET clientSocket);
	int OnFrameClientMessage(int index, int recvlen);

	int                 maxSocketIndex;
	int					socketUsedCount;
	int					checkBeginIndex;
	short				clientCount;
	time_t              lastTime;							/*上次检测超时的时间*/
	time_t              nowTime;							/*当前时间*/
	struct  timeval     timeWait;							/*网络事件超时设置*/ 
	SOCKET              listenSocket;						/*监听socket*/        
	struct sockaddr_in  sockAddr;							/*网络地址*/
	int					beginSocketIndex;
	SocketInfo          * socketInfo;						/*socketinfo结构数组，用于记录客户端的信息*/        
	SocketInfo          * nowSocketInfo;					/*当前的socket结构指针*/
	CCodeQueue          * codeQueueWrite;					/*写数据队列，上一层从这里面读取数据*/
	CCodeQueue          * codeQueueRead;					/*读数据队列，上一层将要发送的数据写这里面*/
	CCodeQueue          * m_PipeWrite;
	char                msgBuff[MAX_BUFF_LEN];			    /*消息包缓冲*/
	
	//char*             allRecvBuff;						/*所有接受到的数据*/
	int                 timeout;							/*超时*/
	//int				recvClientBuffLen;					/*接收到Client数据的长度*/	


private:
	fd_set              readFds;							/*读集合*/ 
	fd_set              writeFds;							/*写集合*/

private:
	bool writePipeExist;
	bool readPipeExist;
};

#endif //__TinyTcp_h__