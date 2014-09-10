#include "TinyTcp.h"

//利用配置信息初始化
TinyTcp::TinyTcp(Config cfg)
{
	zconfig=cfg;
}

/*开始服务*/
int TinyTcp::StartSerivce()
{
	while(1)
	{
		GetExMessage();      /*读取客户端输入*/
		CheckWaitSendData(); /*从管道中读取待发送的数据*/
		//CheckTimeOut();      /*检测通讯超时的socket*/
	}
	return 0;
}

/*进行初始化的操作*/
int TinyTcp::Initialize()
{
	int ret;
	int index;
	maxSocketIndex = 0;
	socketUsedCount = 0;
	clientCount = 0;

	time(&nowTime);
	time(&lastTime);
	//recvClientBuffLen = 10240;
	beginSocketIndex = 0;
	socketInfo = new SocketInfo[zconfig.maxConnectNum];	

	for(int i = 0; i <zconfig.maxConnectNum; i++ )
	{
		/*对所有socket结构进行初始化*/
		memset(&socketInfo[i], 0, sizeof(SocketInfo));
		socketInfo[i].socketIndex = i;
		socketInfo[i].nextIndex= i+1;
		memset(socketInfo[i].recvBuff,'\0',sizeof(socketInfo[i].recvBuff));
	}
	 socketInfo[zconfig.maxConnectNum - 1].nextIndex = -1;
	/*初始化监听端口*/	
	for(int i = 0; i < zconfig.listenPortNum; i++ )
	{
		ret = InitListenSocket(zconfig.listenPorts[i]);
		if (0 != ret)
		{
			return ret;
		}

		index = CreateSocketInfo();

		socketInfo[index].socketHandle = listenSocket;/*设置为监听 this.listenSocket*/
		socketInfo[index].socketType = ListenSocket;		/*type为ListenSocket*/	
		socketInfo[index].isRecv = RECV_DATA;
		socketInfo[index].connectPort =zconfig.listenPorts[i];
		socketInfo[index].createTime = (unsigned int)time(NULL);
		/*没有设置ConnectNum*/

	}
	checkBeginIndex = zconfig.listenPortNum;
	ret = CreatePipe();
	if( 0 != ret )
	{
		printf("CreatePipe Failed! So TCP Server Init Failed!\n");
		return ret;
	}
	return 0;
}
/*初始化Socket信息*/
int TinyTcp::InitListenSocket(short listenPort )
{
	struct sockaddr_in localAddr;
	int ret;
	int iOptVal = 0;
	int iOptLen = sizeof(int);

	WSADATA WSAData;
	WSAStartup(0x202, (LPWSADATA)&WSAData);

	/*初始化socket*/
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);//this->listenSocket
	if(listenSocket < 0 )
	{
		printf("Init Socket Error!\n");
		return -1;
	}
	//初始化地址
	memset((void *)&localAddr, 0, sizeof(localAddr));
	localAddr.sin_addr.s_addr = htonl( INADDR_ANY);
	localAddr.sin_family      = AF_INET;
	localAddr.sin_port        = htons(listenPort);

	/*将socket设置为重用模式*/
	int flag = 1;
	if ( setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) < 0 )
	{
		return -1;
	}

	iOptLen = sizeof(int);
	iOptVal = 0;

	ret = bind(listenSocket, (struct sockaddr *)&localAddr, sizeof(localAddr));
	/*绑定指定端口*/
	if( 0 > ret )
	{
		printf("bind port %d error!\n", listenPort);
		return -1;
	}

	/*设置接收缓冲的大小*/
	iOptLen = sizeof(int);//socklen_t
	iOptVal = zconfig.tcpBuffLen;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&iOptVal, iOptLen))
	{
		printf("Set Send buffer size to %d failed!\n", iOptVal);
		return -1;
	}
	if (!getsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&iOptVal, (int *)&iOptLen))
	{
		printf("Set Send buf of socket is %d.\n", iOptVal);
	}

	//设置为非阻塞模式
	unsigned long block = 1;
	ret = ioctlsocket(listenSocket, FIONBIO, (unsigned long*)&block);
	if(ret == SOCKET_ERROR)
	{
		printf("operate on socket %d error iononblock!\n", listenSocket);
		return -1;
	}

	/*设置接收队列大小*/
	ret = listen(listenSocket, 1024);
	if(-1 == ret)
	{
		printf("listen %d connection error!", 1024);
		return -1;
	}
	return 0;
}
/*找一个空闲的scoketinfo 返回index*/
int TinyTcp::CreateSocketInfo()
{
	int freeIndex = -1;
	if( beginSocketIndex >= 0 )
	{
		freeIndex = beginSocketIndex;
		beginSocketIndex = socketInfo[beginSocketIndex].nextIndex;
		++socketUsedCount;
	}
	if( freeIndex > maxSocketIndex )
	{
		maxSocketIndex = freeIndex;  
	}

	return freeIndex;
}
/*删除一个SocketInfo*/
void TinyTcp::DeleteSocketInfo(short index)
{
	if( index >= zconfig.maxConnectNum )
	{
		return;
	}
	socketInfo[index].nextIndex = beginSocketIndex;
	beginSocketIndex = index;
	--socketUsedCount;
}
void TinyTcp::ClearSocketInfo()
{
	/*关闭Socket，把socket相关信息清空*/
	if( nowSocketInfo->socketHandle <= 0 )
	{
		return;
	}
	closesocket(nowSocketInfo->socketHandle);

	int index = nowSocketInfo->socketIndex;
	int nextIndex = nowSocketInfo->nextIndex;

	memset(nowSocketInfo, 0, sizeof(SocketInfo));
	nowSocketInfo->socketIndex = index;
	nowSocketInfo->nextIndex = nextIndex;
	memset(nowSocketInfo->recvBuff,'\0',sizeof(nowSocketInfo->recvBuff));

	DeleteSocketInfo(nowSocketInfo->socketIndex);
}

/*创建内存管道*/
int TinyTcp::CreatePipe()
{
	base *base_=new base;
	CSharedMem* pSharedMem  = base_->CreateShareMem(zconfig.WritePipeKey, CCodeQueue::CountQueueSize(PIPESIZE), false); 

	CCodeQueue::pCurrentShm = pSharedMem;
	codeQueueWrite= new CCodeQueue(PIPESIZE);//创建写队列

	pSharedMem= base_->CreateShareMem(zconfig.ReadPipeKey, CCodeQueue::CountQueueSize(PIPESIZE), false);
	CCodeQueue::pCurrentShm = pSharedMem;	

	codeQueueRead = new CCodeQueue(PIPESIZE);//创建读队列

	if( !codeQueueWrite || !codeQueueRead )
	{
		return -1;
	}
	return 0;
}

int TinyTcp::OnFrameAccepted(SOCKET clientSocket)
{
	if( beginSocketIndex == -1 )
	{
		printf("error: socket id is so big %d, connnect %d!\n", 
			clientSocket, nowSocketInfo->connectPort);
		closesocket(clientSocket);
		return -1;
	}

	/*搜索一个空闲的socket结构*/
	int index = CreateSocketInfo();

	char *tempIP = inet_ntoa(sockAddr.sin_addr);
	nowSocketInfo->connectPort = ntohs(sockAddr.sin_port);
	socketInfo[index].clientPort = sockAddr.sin_port;
	strcpy(socketInfo[index].clientIp, tempIP);

	socketInfo[index].clientAddr = inet_addr(socketInfo[index].clientIp);	

	socketInfo[index].createTime = time(NULL);					/*记录该socket的生成时间*/
	socketInfo[index].timeStamp      = socketInfo[index].createTime;
	socketInfo[index].socketType = ConnectSocket;
	socketInfo[index].socketHandle     = clientSocket;
	socketInfo[index].isRecv = RECV_DATA;						/*设置socket的状态是待接收数据*/
	socketInfo[index].connectPort = nowSocketInfo->connectPort;

	/*设置为非阻塞模式*/
	unsigned long block = 1;
	int ret = ioctlsocket(clientSocket, FIONBIO, (unsigned long*)&block);
	if(ret == SOCKET_ERROR)
	{
		printf("operate on socket %d error iononblock!\n", clientSocket);
		return -1;
	}
	 nowSocketInfo = &socketInfo[index];//当前指针指向搜索到的这个空闲socketInfo
	 return 0;
}

int TinyTcp::CheckTimeOut()
{
	time_t tempTimeGap;
	time(&nowTime); /*计算当前时间*/

	/*和上次检测时间相比，如果达到了检测间隔则进行检测*/
	if( nowTime - lastTime >= CHECK_TIME )
	{
		/*更新检测时间*/
		lastTime = nowTime;

		for( int i = checkBeginIndex; i <= maxSocketIndex; i++ )
		{
			nowSocketInfo = &socketInfo[i];
			if (0 == nowSocketInfo->isRecv)
			{
				continue;
			}
			if (1 == nowSocketInfo->isSend) /*如果Mainsvrd已经向该socket发送过数据，则判断最后一个接收包的时间*/
			{
				tempTimeGap = nowTime - nowSocketInfo->timeStamp;
				/*把当前时间和最近一次socket收到包的时间相比，如果超过了指定的时间间隔则关闭socket*/
				if (tempTimeGap >= zconfig.socketTimeOut)
				{
					/*该socket通讯超时*/
					printf("Client[%s] socket id = %d port %d not recv packet %d seconds, Close.\n", 
						nowSocketInfo->clientIp, nowSocketInfo->socketHandle, 
						nowSocketInfo->connectPort, tempTimeGap);
					ClearSocketInfo();					
				}
			}
			else /*该客户端已经连接上来了，但是Mainsvrd还没有向它发送一个包，这时的超时更短，主要是防止恶意攻击*/
			{
				tempTimeGap = nowTime - nowSocketInfo->createTime;
				if (zconfig.connTimeOut < tempTimeGap)
				{
					/*该socket通讯超时*/
					printf("Client[%s] connect port %d Timeout %d seconds, close!\n",
						nowSocketInfo->clientIp, nowSocketInfo->connectPort, tempTimeGap);
					ClearSocketInfo();					
				}
			}
		}
	}

	return 0;
}


/*获取外面发送过来的数据的*/
int TinyTcp::GetExMessage()
{
	int       ret;
	int       i;
	int       flags = 1;
	int       sockAddrSize;
	SOCKET    NewSocket;
	SOCKET    FDTemp;
	SOCKET    MaxFD;

	MaxFD        = 0;
	sockAddrSize = sizeof(sockaddr);

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	/*设置所有socket状态*/
	for( i = 0; i <= maxSocketIndex; i++ )
	{
		FDTemp = socketInfo[i].socketHandle;
		if( 0 == FDTemp )
		{
			continue;
		}
 
		if( RECV_DATA == socketInfo[i].isRecv )
		{
			FD_SET(FDTemp, &readFds);
		}
		else
		{
			printf("Error: m_pstSocketInfo[%d]->m_iSocket = %d"
				" m_pstSocketInfo[%d]->m_iSocketFlag = %d!\n",
				i, socketInfo[i].socketHandle,
				i, socketInfo[i].isRecv);
		}
	}

	/*初始化网络超时设置（指定时间没有收到IP包自动返回）*/
	timeWait.tv_sec  = 0;
	timeWait.tv_usec = 10 * 1000;

	/*查询所有socket的状态*/
	ret = select( (int)MaxFD + 1, &readFds, &writeFds, NULL, &timeWait);
	if( 0 >= ret )/*没有网络事件发生*/
	{
		return 0;
	}
	/*检测所有socket的状态更新*/
	for( i = 0; i <= maxSocketIndex; i++ )
	{
		nowSocketInfo = &socketInfo[i];
		FDTemp = nowSocketInfo->socketHandle;
		if( 0 == FDTemp )
		{
			continue;
		}
		if(FD_ISSET(FDTemp, &readFds))
		{
			if(ListenSocket == nowSocketInfo->socketType )
			{
				/*有新的用户连接上来了*/
				NewSocket = accept(FDTemp, (struct sockaddr *)&sockAddr, (int *)&sockAddrSize);
 
				if( NewSocket == INVALID_SOCKET ) /*客户端连接上来以后又立即关闭了*/ 
				{
					printf("client connect port %d and disconnected!\n", nowSocketInfo->connectPort);
					continue;
				}
				OnFrameAccepted(NewSocket);
			}
			else
			{
				RecvClientData(i);
			}
		}
	}
	return 0;
}

int TinyTcp::RecvClientData(short index)
{
	int recvlen;

	int pos=nowSocketInfo->recvSize;
	recvlen = recv(nowSocketInfo->socketHandle, nowSocketInfo->recvBuff+pos, sizeof(nowSocketInfo->recvBuff)-pos, 0);

	if(recvlen<=0)
	{
		//ClearSocketInfo();//不用自己清除 CheckTimeOut函数会自己清除的
		return -1;
	}
	nowSocketInfo->recvSize+=recvlen;

	int ret = OnFrameClientMessage(index, recvlen);
	/*CheckWaitSendData();*/
	return ret;
}

int TinyTcp::OnFrameClientMessage(int Index, int Recvlen)
{

	TcpMsgPacket msg;
	int decodesize=DecodeMsg(nowSocketInfo->recvBuff,msg);//得到解包数据大小，然后进行指针的改变

	printf("recv:%s\n",msg.msg);
	int ret=codeQueueWrite->AppendOneCode((unsigned char*)nowSocketInfo->recvBuff,decodesize);//加入写队列
	if(ret<0)
	{
		return -1;
	}

	//始终让数据从buff开始
	//memset(nowSocketInfo->recvBuff,'\0',decodesize);//把它赋值为空的
	memmove(nowSocketInfo->recvBuff,nowSocketInfo->recvBuff+decodesize,RECV_BUFF_LEN-decodesize);
	nowSocketInfo->recvSize-=decodesize;

	CheckWaitSendData();
	return 0;
}

/*从共享内存取出数据来发送*/
int TinyTcp::CheckWaitSendData()
{
	int   ret;
	int   i;
	unsigned int readLen;
	for(i = 0; i < MAX_SEND_PACKET; i++)/*一次最多发送MAXSENDPKGS个数据包*/
	{
		readLen = MAX_BUFF_LEN;
		ret = codeQueueRead->GetHeadCode((unsigned char *)msgBuff, &readLen);
		if(0 > ret)
		{
			if (ERR_FOR_SMALLBUFF == ret)
			{
				printf("Error: In S2C pipe a packet len is %d so long.\n", readLen);
				break;
			}
			else
			{
				break;
			}
		}

		if(readLen == 0 )
		{
			break;
		}

		SendClientData((char *)msgBuff, readLen);
	}
	return 0;
}
/*发送数据到客户端，要根据数据中的uin或者其他标示符，在SocketInfo数组中找到它inde,然后再通过它发送数据x*/
int TinyTcp::SendClientData(char* buff, int nlen)
{
	//先把要发送的数据的uin找出来,并通过uin找到SocketInfo的Index
	TcpMsgPacket msg;
	DecodeMsg(buff,msg);
	int index=FindSocketInfo(msg.uin);
	nowSocketInfo=&socketInfo[index];//需要找到对应的socket的索引，才能够正确发送数据

	int sendLen=SendSocketData(nowSocketInfo->socketHandle,msgBuff,nlen);
	if(sendLen==nlen)//发送成功
	{
		nowSocketInfo->isSend = 1;//MAINHAVESENDDATA
		printf("send:%s\n",msg.msg);
		return 0;
	}
	return -1;
}
int TinyTcp::SendSocketData(SOCKET socketHandle, char *sendBuff, int sendLen)
{
	int allSendLen = 0;	//总共发送的数据
	int oneSendLen = 0;	//一次发送的数据
	for(;;)
	{
		oneSendLen = send(socketHandle, sendBuff, sendLen, 0);		
		allSendLen += oneSendLen;
		if(oneSendLen==SOCKET_ERROR)/*出错了就退出*/
		{
			break;
		}
		if( allSendLen ==sendLen)
		{
			break;
		}
	}
	return allSendLen;
} 

int TinyTcp::FindSocketInfo(long uin)
{
	return GetIndexByUIN(socketInfo,zconfig.maxConnectNum,uin);
}
/*连接其它主机进行通讯*/
int TinyTcp::ConnectOther(ConnectPara para)
{
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	int index=CreateSocketInfo();						//搜索一个空闲的 用来连接其它的
	socketInfo[index].socketHandle = listenSocket;
	socketInfo[index].socketType = ConnectSocket;		/*设置为连接SOCKET*/	
	socketInfo[index].isRecv = RECV_DATA;
	socketInfo[index].connectPort =9009;
	socketInfo[index].createTime = (unsigned int)time(NULL);
	socketInfo[index].uin=para.uin;//设置uin,以便在发送数据时找到
	int ret=connect(socketInfo[index].socketHandle,(struct sockaddr*)&para.other_addr,sizeof(para.other_addr));
	if(ret==SOCKET_ERROR)
	{
		printf("连接服务器失败!");
		return -1;
	}
	return 0;
}