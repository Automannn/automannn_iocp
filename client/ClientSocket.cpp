//
// Created by 14394 on 2020/2/10.
//

#include "ClientSocket.h"


SOCKET clientSocket;
HANDLE bufferMutex;

const int DefaultPort = 8060;

CClientSocket::CClientSocket(){}

CClientSocket::~CClientSocket() {}

int CClientSocket::prepareEnvironment() {
    //����socket��̬���ӿ�
    WORD wVersionRequested;
    WSADATA wsaData;//���ڽ���Windows socket�Ľṹ��Ϣ
    wVersionRequested = MAKEWORD(2,2);//����2.2�汾�� sinSock��
    int err = WSAStartup(wVersionRequested,&wsaData);
    if(err!=0){ //����winsock��ʧ��
        return -1;
    }
    if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){  //�������İ汾��
        WSACleanup();
        return -1;
    }

    clientSocket = socket(AF_INET,SOCK_STREAM,0);
    if(clientSocket == INVALID_SOCKET){
        std::cout<<"����socketʧ��.Error:"<<WSAGetLastError()<<std::endl;
        WSACleanup();
        return -1;
    }

    //����Զ������
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port = htons(DefaultPort);
    while (SOCKET_ERROR==connect(clientSocket, (SOCKADDR *)(&addrSrv), sizeof(SOCKADDR))){
        std::cout<<"����������ʧ�ܣ��Ƿ�����?(Y/N):";
        char choice;
        while(cin>>choice&&(choice!='N'&&choice!='Y')){
            std::cout<<"�����������������:";
            std::cin.sync();
            std::cin.clear();
        }
        if(choice=='Y'){
            continue;
        }else{
            std::cout<<"�Ƴ�ϵͳ��...";
            system("pause");
            return 0;
        }
    }
    std::cin.sync();

    std::cout<<"���ͻ�����׼���������û���ֱ�����������������������Ϣ��\n";
    send(clientSocket,"�ͻ������ӽ���.",200,0);

    bufferMutex = CreateSemaphore(NULL,1,1,NULL);

    HANDLE sendThread = CreateThread(NULL,0,SendMessageThread,NULL,0,NULL);
    HANDLE receiveThread = CreateThread(NULL,0,ReceiveMessageThread,NULL,0,NULL);

    WaitForSingleObject(sendThread,INFINITE);
    closesocket(clientSocket);
    CloseHandle(sendThread);
    CloseHandle(receiveThread);
    CloseHandle(bufferMutex);

    WSACleanup();//��ֹ���׽��ֿ��ʹ��

    std::cout<<"�����Ự..."<<std::endl;
    system("pause");
    return 0;

}

DWORD WINAPI SendMessageThread(LPVOID ipParam){
    while (1){
        string  talk;
        getline(cin,talk);
        WaitForSingleObject(bufferMutex,INFINITE);
        if("quit"==talk){
            talk.push_back('\0');
            send(clientSocket,talk.c_str(),200,0);
            break;
        }
        std::cout<<"\nI Say:(\"quit\" to exit):"<<talk<<"\n";
        send(clientSocket,talk.c_str(),200,0);
        ReleaseSemaphore(bufferMutex,1,NULL);
        
        Sleep(5000);
    }
    return 0;
}
DWORD WINAPI ReceiveMessageThread(LPVOID ipParam){
    while (1){
        char recvBuf[300];
        recv(clientSocket,recvBuf,200,0);
        WaitForSingleObject(bufferMutex,INFINITE);

        std::cout<<"Server Says:"<<recvBuf<<std::endl;

        ReleaseSemaphore(bufferMutex,1,NULL);
        if(recvBuf[0]=='\0'){
            cout<<"�����̹߳ر�!";
            break;
        }
    }
    return  0;
}