//
// Created by 14394 on 2020/2/9.
//

#include <inaddr.h>
#include "IOCPServerSocket.h"

using namespace std;

/**定义重叠IO需要用到的结构体
 * 临时记录IO数据
 * **/
const int DataBuffSize = 2*1024;
typedef struct {
    OVERLAPPED overlapped;
    WSABUF databuff;
    char buffer[DataBuffSize];
    int BufferLen;
    int operationType;
}PerIOOperationData,PerIOData,*LpperIOData,*LpperIOOperationData;

/**
 * 记录单个套接字的数据，包括套接字变量 以及 套接字的对应客户端的地址
 * 当服务器连接上客户端时，信息存储到该结构体中，以便于回访
 * */
typedef struct {
    SOCKET  socket;
    SOCKADDR_STORAGE clientAddr;
}PerHandleData,*LpperHandleData;

const int DefaultPort=8060;
std::vector<PerHandleData*> clientGroup;//记录客户端的向量组

HANDLE hMutex = CreateMutex(NULL,FALSE,NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompleionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);


CServerSocket::CServerSocket(){
    cout<<"socket类初始化完成!"<<endl;
}

CServerSocket::~CServerSocket() {
    cout<<"socket资源回收完成!"<<endl;
}

int CServerSocket::prepareEnvironment() {
    WORD wVersionRequested = MAKEWORD(2,2); //请求2.2版本的WinSock库
    WSADATA wsadata;//接收Windows Socket的结构信息
    DWORD err = WSAStartup(wVersionRequested,&wsadata);
    if(0!=err){//检查套接字库是否申请成功
        std::cerr<<"请求windows套接字库失败!\n";
        system("pause");
        return -1;
    }
    if (LOBYTE(wsadata.wVersion)!=2||HIBYTE(wsadata.wVersion)!=2){//检查是否申请了所需版本的套接字库
        WSACleanup();
        std::cerr<<"请求windows套接字库 2.2失败!\n";
        system("pause");
        return -1;
    }
    //创建 IOCP的内核对象
    //参数说明:已经打开的文件句柄或者空句柄(一般是客户端的句柄),已经存在的IOCP句柄，完成键，真正并发同时执行最大线程数
    HANDLE  completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
    if(NULL == completionPort){//创建IO内核对象失败
        std::cerr<<"创建IO完成端口失败!Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }

    //创建IOCP线程--线程里面拆给你就线程池
    //确定处理器的核心 数量
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);

    //基于处理器的核心数量创建线程
    for(DWORD i=0;i<(mySysInfo.dwNumberOfProcessors*2);++i){
        //创建服务器工作线程，并将完成端口传递到该线程
        HANDLE threadHandle = CreateThread(NULL,0,ServerWorkThread,completionPort,0,NULL);
        if(NULL == threadHandle){
            cerr<<"创建线程句柄失败.Error:"<<GetLastError()<<endl;
            system("pause");
            return  -1;
        }
        CloseHandle(threadHandle);
    }

    //建立流式套接字
    SOCKET  srvSocket = socket(AF_INET,SOCK_STREAM,0);
    //绑定SOCKET到本机
    SOCKADDR_IN srvAddr;
    srvAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    srvAddr.sin_family=AF_INET;
    srvAddr.sin_port=htons(DefaultPort);

    int bindResult = bind(srvSocket,(SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (SOCKET_ERROR==bindResult){
        cerr<<"绑定失败.Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }

    //将socket设置为监听模式
    int listenResult = listen(srvSocket,10);
    if(SOCKET_ERROR ==listenResult){
        cerr<<"监听失败.Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }
    cout<<"本服务器已准备就绪，正在等待客户端的接入...\n";

    //创建用于发送数据的线程
    HANDLE sendThread = CreateThread(NULL,0,ServerSendThread,0,0,NULL);
    CloseHandle(sendThread);
    while (true){
        PerHandleData*  perHandleData=NULL;
        SOCKADDR_IN saRemote;
        int remoteLen;
        SOCKET acceptSocket;

        //接收连接，并分配 完成端口
        remoteLen = sizeof(saRemote);
        acceptSocket = accept(srvSocket,(SOCKADDR*)&saRemote,&remoteLen);
        if(SOCKET_ERROR==acceptSocket){
            cerr<<"接收套接字失败.Error:"<<GetLastError()<<endl;
            system("pause");
            return -1;
        }
        //创建用来和套接字关联的单句柄数据信息结构
        perHandleData = (LpperHandleData)GlobalAlloc(GPTR, sizeof(PerHandleData));
        perHandleData->socket=acceptSocket;
        memcpy(&perHandleData->clientAddr,&saRemote,remoteLen);
        clientGroup.push_back(perHandleData); //将单个客户端数据指针放到客户端组中

        //将接收套接字和完成端口关联
        CreateIoCompletionPort((HANDLE)(perHandleData->socket), completionPort,(ULONG_PTR)(perHandleData), 0);

        //开始在接受套接字上处理I/O使用重叠IO机制
        //在新建套接字上投递一个或多个异步
        //WSARecv 或 WSASend请求，这些IO请求完成后，工作者线程会为IO请求提供服务
        //单IO 操作数据(IO重叠)
        LpperIOOperationData perIoData= NULL;
        perIoData = (LpperIOOperationData)GlobalAlloc(GPTR, sizeof(PerIOOperationData));
        ZeroMemory(&perIoData->overlapped, sizeof(OVERLAPPED));
        perIoData->databuff.len=1024;
        perIoData->databuff.buf=perIoData->buffer;
        perIoData->operationType=0;//read

        DWORD  RecvBytes;
        DWORD Flags =0;
        WSARecv(perHandleData->socket,&perIoData->databuff,1,&RecvBytes,&Flags,&(perIoData->overlapped),NULL);
    }

    system("pause");
    return 0;
}

DWORD WINAPI ServerWorkThread(LPVOID ipParam){
    HANDLE completionPort = (HANDLE)ipParam;
    DWORD bytesTransferred;
    LPOVERLAPPED ipOverLapped;
    LpperHandleData perHandleData=NULL;

    DWORD recvBytes;
    DWORD flags =0;
    BOOL bRet = false;
    while(true){
        bRet = GetQueuedCompletionStatus(completionPort, &bytesTransferred, (PULONG_PTR)&perHandleData, (LPOVERLAPPED*)&ipOverLapped, INFINITE);
        if(bRet ==0){
            cerr<<"获取队列完成状态失败. Error:"<<GetLastError()<<endl;
            return -1;
        }

        LpperIOData perIoData=NULL;
        perIoData = (LpperIOData)CONTAINING_RECORD(ipOverLapped,PerIOData,overlapped);
        //检查在套接字上是否有错误发生
        if(0==bytesTransferred){
            closesocket(perHandleData->socket);
            GlobalFree(perHandleData);
            GlobalFree(perIoData);
            continue;
        }
        //开始数据处理，接受来自客户端的数据
        WaitForSingleObject(hMutex,INFINITE);
        printf("there is a client says:");
        if(perIoData->databuff.buf){
            printf(perIoData->databuff.buf);
            printf("\n");
        } else{
            printf("\n");
        }
        ReleaseMutex(hMutex);

        //为下一个重叠调用 建立单 I/O 操作数据
        perIoData = (LpperIOData)GlobalAlloc(GPTR, sizeof(LpperIOData));
        ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED)); //清空内存
        perIoData->databuff.len= 1024;
        perIoData->databuff.buf=perIoData->buffer;
        perIoData->operationType=0;//read;
        WSARecv(perHandleData->socket, &(perIoData->databuff), 1, (&recvBytes), &flags, &(perIoData->overlapped), NULL);
    }
    return 0;
}

//发送信息的线程执行函数
DWORD WINAPI ServerSendThread(LPVOID ipParam){
    while (1){
        char talk[200];
        gets(talk);
        if (talk ==""){
            return 0;
        }

        WaitForSingleObject(hMutex,INFINITE);
        for(int i=0;i<clientGroup.size();++i){
            send(clientGroup[i]->socket,talk,200,0);
        }
        ReleaseMutex(hMutex);
    }
    return 0;
}