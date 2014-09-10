#include "TinyTcp.h"

//����������Ϣ��ʼ��
TinyTcp::TinyTcp(Config cfg)
{
	zconfig=cfg;
}

/*��ʼ����*/
int TinyTcp::StartSerivce()
{
	while(1)
	{
		GetExMessage();      /*��ȡ�ͻ�������*/
		CheckWaitSendData(); /*�ӹܵ��ж�ȡ�����͵�����*/
		//CheckTimeOut();      /*���ͨѶ��ʱ��socket*/
	}
	return 0;
}

/*���г�ʼ���Ĳ���*/
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
		/*������socket�ṹ���г�ʼ��*/
		memset(&socketInfo[i], 0, sizeof(SocketInfo));
		socketInfo[i].socketIndex = i;
		socketInfo[i].nextIndex= i+1;
		memset(socketInfo[i].recvBuff,'\0',sizeof(socketInfo[i].recvBuff));
	}
	 socketInfo[zconfig.maxConnectNum - 1].nextIndex = -1;
	/*��ʼ�������˿�*/	
	for(int i = 0; i < zconfig.listenPortNum; i++ )
	{
		ret = InitListenSocket(zconfig.listenPorts[i]);
		if (0 != ret)
		{
			return ret;
		}

		index = CreateSocketInfo();

		socketInfo[index].socketHandle = listenSocket;/*����Ϊ���� this.listenSocket*/
		socketInfo[index].socketType = ListenSocket;		/*typeΪListenSocket*/	
		socketInfo[index].isRecv = RECV_DATA;
		socketInfo[index].connectPort =zconfig.listenPorts[i];
		socketInfo[index].createTime = (unsigned int)time(NULL);
		/*û������ConnectNum*/

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
/*��ʼ��Socket��Ϣ*/
int TinyTcp::InitListenSocket(short listenPort )
{
	struct sockaddr_in localAddr;
	int ret;
	int iOptVal = 0;
	int iOptLen = sizeof(int);

	WSADATA WSAData;
	WSAStartup(0x202, (LPWSADATA)&WSAData);

	/*��ʼ��socket*/
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);//this->listenSocket
	if(listenSocket < 0 )
	{
		printf("Init Socket Error!\n");
		return -1;
	}
	//��ʼ����ַ
	memset((void *)&localAddr, 0, sizeof(localAddr));
	localAddr.sin_addr.s_addr = htonl( INADDR_ANY);
	localAddr.sin_family      = AF_INET;
	localAddr.sin_port        = htons(listenPort);

	/*��socket����Ϊ����ģʽ*/
	int flag = 1;
	if ( setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) < 0 )
	{
		return -1;
	}

	iOptLen = sizeof(int);
	iOptVal = 0;

	ret = bind(listenSocket, (struct sockaddr *)&localAddr, sizeof(localAddr));
	/*��ָ���˿�*/
	if( 0 > ret )
	{
		printf("bind port %d error!\n", listenPort);
		return -1;
	}

	/*���ý��ջ���Ĵ�С*/
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

	//����Ϊ������ģʽ
	unsigned long block = 1;
	ret = ioctlsocket(listenSocket, FIONBIO, (unsigned long*)&block);
	if(ret == SOCKET_ERROR)
	{
		printf("operate on socket %d error iononblock!\n", listenSocket);
		return -1;
	}

	/*���ý��ն��д�С*/
	ret = listen(listenSocket, 1024);
	if(-1 == ret)
	{
		printf("listen %d connection error!", 1024);
		return -1;
	}
	return 0;
}
/*��һ�����е�scoketinfo ����index*/
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
/*ɾ��һ��SocketInfo*/
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
	/*�ر�Socket����socket�����Ϣ���*/
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

/*�����ڴ�ܵ�*/
int TinyTcp::CreatePipe()
{
	base *base_=new base;
	CSharedMem* pSharedMem  = base_->CreateShareMem(zconfig.WritePipeKey, CCodeQueue::CountQueueSize(PIPESIZE), false); 

	CCodeQueue::pCurrentShm = pSharedMem;
	codeQueueWrite= new CCodeQueue(PIPESIZE);//����д����

	pSharedMem= base_->CreateShareMem(zconfig.ReadPipeKey, CCodeQueue::CountQueueSize(PIPESIZE), false);
	CCodeQueue::pCurrentShm = pSharedMem;	

	codeQueueRead = new CCodeQueue(PIPESIZE);//����������

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

	/*����һ�����е�socket�ṹ*/
	int index = CreateSocketInfo();

	char *tempIP = inet_ntoa(sockAddr.sin_addr);
	nowSocketInfo->connectPort = ntohs(sockAddr.sin_port);
	socketInfo[index].clientPort = sockAddr.sin_port;
	strcpy(socketInfo[index].clientIp, tempIP);

	socketInfo[index].clientAddr = inet_addr(socketInfo[index].clientIp);	

	socketInfo[index].createTime = time(NULL);					/*��¼��socket������ʱ��*/
	socketInfo[index].timeStamp      = socketInfo[index].createTime;
	socketInfo[index].socketType = ConnectSocket;
	socketInfo[index].socketHandle     = clientSocket;
	socketInfo[index].isRecv = RECV_DATA;						/*����socket��״̬�Ǵ���������*/
	socketInfo[index].connectPort = nowSocketInfo->connectPort;

	/*����Ϊ������ģʽ*/
	unsigned long block = 1;
	int ret = ioctlsocket(clientSocket, FIONBIO, (unsigned long*)&block);
	if(ret == SOCKET_ERROR)
	{
		printf("operate on socket %d error iononblock!\n", clientSocket);
		return -1;
	}
	 nowSocketInfo = &socketInfo[index];//��ǰָ��ָ�����������������socketInfo
	 return 0;
}

int TinyTcp::CheckTimeOut()
{
	time_t tempTimeGap;
	time(&nowTime); /*���㵱ǰʱ��*/

	/*���ϴμ��ʱ����ȣ�����ﵽ�˼��������м��*/
	if( nowTime - lastTime >= CHECK_TIME )
	{
		/*���¼��ʱ��*/
		lastTime = nowTime;

		for( int i = checkBeginIndex; i <= maxSocketIndex; i++ )
		{
			nowSocketInfo = &socketInfo[i];
			if (0 == nowSocketInfo->isRecv)
			{
				continue;
			}
			if (1 == nowSocketInfo->isSend) /*���Mainsvrd�Ѿ����socket���͹����ݣ����ж����һ�����հ���ʱ��*/
			{
				tempTimeGap = nowTime - nowSocketInfo->timeStamp;
				/*�ѵ�ǰʱ������һ��socket�յ�����ʱ����ȣ����������ָ����ʱ������ر�socket*/
				if (tempTimeGap >= zconfig.socketTimeOut)
				{
					/*��socketͨѶ��ʱ*/
					printf("Client[%s] socket id = %d port %d not recv packet %d seconds, Close.\n", 
						nowSocketInfo->clientIp, nowSocketInfo->socketHandle, 
						nowSocketInfo->connectPort, tempTimeGap);
					ClearSocketInfo();					
				}
			}
			else /*�ÿͻ����Ѿ����������ˣ�����Mainsvrd��û����������һ��������ʱ�ĳ�ʱ���̣���Ҫ�Ƿ�ֹ���⹥��*/
			{
				tempTimeGap = nowTime - nowSocketInfo->createTime;
				if (zconfig.connTimeOut < tempTimeGap)
				{
					/*��socketͨѶ��ʱ*/
					printf("Client[%s] connect port %d Timeout %d seconds, close!\n",
						nowSocketInfo->clientIp, nowSocketInfo->connectPort, tempTimeGap);
					ClearSocketInfo();					
				}
			}
		}
	}

	return 0;
}


/*��ȡ���淢�͹��������ݵ�*/
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
	/*��������socket״̬*/
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

	/*��ʼ�����糬ʱ���ã�ָ��ʱ��û���յ�IP���Զ����أ�*/
	timeWait.tv_sec  = 0;
	timeWait.tv_usec = 10 * 1000;

	/*��ѯ����socket��״̬*/
	ret = select( (int)MaxFD + 1, &readFds, &writeFds, NULL, &timeWait);
	if( 0 >= ret )/*û�������¼�����*/
	{
		return 0;
	}
	/*�������socket��״̬����*/
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
				/*���µ��û�����������*/
				NewSocket = accept(FDTemp, (struct sockaddr *)&sockAddr, (int *)&sockAddrSize);
 
				if( NewSocket == INVALID_SOCKET ) /*�ͻ������������Ժ��������ر���*/ 
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
		//ClearSocketInfo();//�����Լ���� CheckTimeOut�������Լ������
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
	int decodesize=DecodeMsg(nowSocketInfo->recvBuff,msg);//�õ�������ݴ�С��Ȼ�����ָ��ĸı�

	printf("recv:%s\n",msg.msg);
	int ret=codeQueueWrite->AppendOneCode((unsigned char*)nowSocketInfo->recvBuff,decodesize);//����д����
	if(ret<0)
	{
		return -1;
	}

	//ʼ�������ݴ�buff��ʼ
	//memset(nowSocketInfo->recvBuff,'\0',decodesize);//������ֵΪ�յ�
	memmove(nowSocketInfo->recvBuff,nowSocketInfo->recvBuff+decodesize,RECV_BUFF_LEN-decodesize);
	nowSocketInfo->recvSize-=decodesize;

	CheckWaitSendData();
	return 0;
}

/*�ӹ����ڴ�ȡ������������*/
int TinyTcp::CheckWaitSendData()
{
	int   ret;
	int   i;
	unsigned int readLen;
	for(i = 0; i < MAX_SEND_PACKET; i++)/*һ����෢��MAXSENDPKGS�����ݰ�*/
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
/*�������ݵ��ͻ��ˣ�Ҫ���������е�uin����������ʾ������SocketInfo�������ҵ���inde,Ȼ����ͨ������������x*/
int TinyTcp::SendClientData(char* buff, int nlen)
{
	//�Ȱ�Ҫ���͵����ݵ�uin�ҳ���,��ͨ��uin�ҵ�SocketInfo��Index
	TcpMsgPacket msg;
	DecodeMsg(buff,msg);
	int index=FindSocketInfo(msg.uin);
	nowSocketInfo=&socketInfo[index];//��Ҫ�ҵ���Ӧ��socket�����������ܹ���ȷ��������

	int sendLen=SendSocketData(nowSocketInfo->socketHandle,msgBuff,nlen);
	if(sendLen==nlen)//���ͳɹ�
	{
		nowSocketInfo->isSend = 1;//MAINHAVESENDDATA
		printf("send:%s\n",msg.msg);
		return 0;
	}
	return -1;
}
int TinyTcp::SendSocketData(SOCKET socketHandle, char *sendBuff, int sendLen)
{
	int allSendLen = 0;	//�ܹ����͵�����
	int oneSendLen = 0;	//һ�η��͵�����
	for(;;)
	{
		oneSendLen = send(socketHandle, sendBuff, sendLen, 0);		
		allSendLen += oneSendLen;
		if(oneSendLen==SOCKET_ERROR)/*�����˾��˳�*/
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
/*����������������ͨѶ*/
int TinyTcp::ConnectOther(ConnectPara para)
{
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	int index=CreateSocketInfo();						//����һ�����е� ��������������
	socketInfo[index].socketHandle = listenSocket;
	socketInfo[index].socketType = ConnectSocket;		/*����Ϊ����SOCKET*/	
	socketInfo[index].isRecv = RECV_DATA;
	socketInfo[index].connectPort =9009;
	socketInfo[index].createTime = (unsigned int)time(NULL);
	socketInfo[index].uin=para.uin;//����uin,�Ա��ڷ�������ʱ�ҵ�
	int ret=connect(socketInfo[index].socketHandle,(struct sockaddr*)&para.other_addr,sizeof(para.other_addr));
	if(ret==SOCKET_ERROR)
	{
		printf("���ӷ�����ʧ��!");
		return -1;
	}
	return 0;
}