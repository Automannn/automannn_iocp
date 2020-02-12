//
// Created by 14394 on 2020/2/9.
//

#include <inaddr.h>
#include "IOCPServerSocket.h"

using namespace std;

/**�����ص�IO��Ҫ�õ��Ľṹ��
 * ��ʱ��¼IO����
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
 * ��¼�����׽��ֵ����ݣ������׽��ֱ��� �Լ� �׽��ֵĶ�Ӧ�ͻ��˵ĵ�ַ
 * �������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У��Ա��ڻط�
 * */
typedef struct {
    SOCKET  socket;
    SOCKADDR_STORAGE clientAddr;
}PerHandleData,*LpperHandleData;

const int DefaultPort=8060;
std::vector<PerHandleData*> clientGroup;//��¼�ͻ��˵�������

HANDLE hMutex = CreateMutex(NULL,FALSE,NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompleionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);


CServerSocket::CServerSocket(){
    cout<<"socket���ʼ�����!"<<endl;
}

CServerSocket::~CServerSocket() {
    cout<<"socket��Դ�������!"<<endl;
}

int CServerSocket::prepareEnvironment() {
    WORD wVersionRequested = MAKEWORD(2,2); //����2.2�汾��WinSock��
    WSADATA wsadata;//����Windows Socket�Ľṹ��Ϣ
    DWORD err = WSAStartup(wVersionRequested,&wsadata);
    if(0!=err){//����׽��ֿ��Ƿ�����ɹ�
        std::cerr<<"����windows�׽��ֿ�ʧ��!\n";
        system("pause");
        return -1;
    }
    if (LOBYTE(wsadata.wVersion)!=2||HIBYTE(wsadata.wVersion)!=2){//����Ƿ�����������汾���׽��ֿ�
        WSACleanup();
        std::cerr<<"����windows�׽��ֿ� 2.2ʧ��!\n";
        system("pause");
        return -1;
    }
    //���� IOCP���ں˶���
    //����˵��:�Ѿ��򿪵��ļ�������߿վ��(һ���ǿͻ��˵ľ��),�Ѿ����ڵ�IOCP�������ɼ�����������ͬʱִ������߳���
    HANDLE  completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
    if(NULL == completionPort){//����IO�ں˶���ʧ��
        std::cerr<<"����IO��ɶ˿�ʧ��!Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }

    //����IOCP�߳�--�߳�����������̳߳�
    //ȷ���������ĺ��� ����
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);

    //���ڴ������ĺ������������߳�
    for(DWORD i=0;i<(mySysInfo.dwNumberOfProcessors*2);++i){
        //���������������̣߳�������ɶ˿ڴ��ݵ����߳�
        HANDLE threadHandle = CreateThread(NULL,0,ServerWorkThread,completionPort,0,NULL);
        if(NULL == threadHandle){
            cerr<<"�����߳̾��ʧ��.Error:"<<GetLastError()<<endl;
            system("pause");
            return  -1;
        }
        CloseHandle(threadHandle);
    }

    //������ʽ�׽���
    SOCKET  srvSocket = socket(AF_INET,SOCK_STREAM,0);
    //��SOCKET������
    SOCKADDR_IN srvAddr;
    srvAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    srvAddr.sin_family=AF_INET;
    srvAddr.sin_port=htons(DefaultPort);

    int bindResult = bind(srvSocket,(SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (SOCKET_ERROR==bindResult){
        cerr<<"��ʧ��.Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }

    //��socket����Ϊ����ģʽ
    int listenResult = listen(srvSocket,10);
    if(SOCKET_ERROR ==listenResult){
        cerr<<"����ʧ��.Error:"<<GetLastError()<<endl;
        system("pause");
        return -1;
    }
    cout<<"����������׼�����������ڵȴ��ͻ��˵Ľ���...\n";

    //�������ڷ������ݵ��߳�
    HANDLE sendThread = CreateThread(NULL,0,ServerSendThread,0,0,NULL);
    CloseHandle(sendThread);
    while (true){
        PerHandleData*  perHandleData=NULL;
        SOCKADDR_IN saRemote;
        int remoteLen;
        SOCKET acceptSocket;

        //�������ӣ������� ��ɶ˿�
        remoteLen = sizeof(saRemote);
        acceptSocket = accept(srvSocket,(SOCKADDR*)&saRemote,&remoteLen);
        if(SOCKET_ERROR==acceptSocket){
            cerr<<"�����׽���ʧ��.Error:"<<GetLastError()<<endl;
            system("pause");
            return -1;
        }
        //�����������׽��ֹ����ĵ����������Ϣ�ṹ
        perHandleData = (LpperHandleData)GlobalAlloc(GPTR, sizeof(PerHandleData));
        perHandleData->socket=acceptSocket;
        memcpy(&perHandleData->clientAddr,&saRemote,remoteLen);
        clientGroup.push_back(perHandleData); //�������ͻ�������ָ��ŵ��ͻ�������

        //�������׽��ֺ���ɶ˿ڹ���
        CreateIoCompletionPort((HANDLE)(perHandleData->socket), completionPort,(ULONG_PTR)(perHandleData), 0);

        //��ʼ�ڽ����׽����ϴ���I/Oʹ���ص�IO����
        //���½��׽�����Ͷ��һ�������첽
        //WSARecv �� WSASend������ЩIO������ɺ󣬹������̻߳�ΪIO�����ṩ����
        //��IO ��������(IO�ص�)
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
            cerr<<"��ȡ�������״̬ʧ��. Error:"<<GetLastError()<<endl;
            return -1;
        }

        LpperIOData perIoData=NULL;
        perIoData = (LpperIOData)CONTAINING_RECORD(ipOverLapped,PerIOData,overlapped);
        //������׽������Ƿ��д�����
        if(0==bytesTransferred){
            closesocket(perHandleData->socket);
            GlobalFree(perHandleData);
            GlobalFree(perIoData);
            continue;
        }
        //��ʼ���ݴ����������Կͻ��˵�����
        WaitForSingleObject(hMutex,INFINITE);
        printf("there is a client says:");
        if(perIoData->databuff.buf){
            printf(perIoData->databuff.buf);
            printf("\n");
        } else{
            printf("\n");
        }
        ReleaseMutex(hMutex);

        //Ϊ��һ���ص����� ������ I/O ��������
        perIoData = (LpperIOData)GlobalAlloc(GPTR, sizeof(LpperIOData));
        ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED)); //����ڴ�
        perIoData->databuff.len= 1024;
        perIoData->databuff.buf=perIoData->buffer;
        perIoData->operationType=0;//read;
        WSARecv(perHandleData->socket, &(perIoData->databuff), 1, (&recvBytes), &flags, &(perIoData->overlapped), NULL);
    }
    return 0;
}

//������Ϣ���߳�ִ�к���
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