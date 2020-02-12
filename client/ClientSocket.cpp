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
    //加载socket动态链接库
    WORD wVersionRequested;
    WSADATA wsaData;//用于接收Windows socket的结构信息
    wVersionRequested = MAKEWORD(2,2);//请求2.2版本的 sinSock库
    int err = WSAStartup(wVersionRequested,&wsaData);
    if(err!=0){ //调用winsock库失败
        return -1;
    }
    if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){  //检查申请的版本号
        WSACleanup();
        return -1;
    }

    clientSocket = socket(AF_INET,SOCK_STREAM,0);
    if(clientSocket == INVALID_SOCKET){
        std::cout<<"生成socket失败.Error:"<<WSAGetLastError()<<std::endl;
        WSACleanup();
        return -1;
    }

    //连接远程主机
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port = htons(DefaultPort);
    while (SOCKET_ERROR==connect(clientSocket, (SOCKADDR *)(&addrSrv), sizeof(SOCKADDR))){
        std::cout<<"服务器连接失败，是否重连?(Y/N):";
        char choice;
        while(cin>>choice&&(choice!='N'&&choice!='Y')){
            std::cout<<"输入错误，请重新输入:";
            std::cin.sync();
            std::cin.clear();
        }
        if(choice=='Y'){
            continue;
        }else{
            std::cout<<"推出系统中...";
            system("pause");
            return 0;
        }
    }
    std::cin.sync();

    std::cout<<"本客户端已准备就绪，用户可直接输入文字向服务器反馈信息。\n";
    send(clientSocket,"客户端连接进入.",200,0);

    bufferMutex = CreateSemaphore(NULL,1,1,NULL);

    HANDLE sendThread = CreateThread(NULL,0,SendMessageThread,NULL,0,NULL);
    HANDLE receiveThread = CreateThread(NULL,0,ReceiveMessageThread,NULL,0,NULL);

    WaitForSingleObject(sendThread,INFINITE);
    closesocket(clientSocket);
    CloseHandle(sendThread);
    CloseHandle(receiveThread);
    CloseHandle(bufferMutex);

    WSACleanup();//终止对套接字库的使用

    std::cout<<"结束会话..."<<std::endl;
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
            cout<<"接收线程关闭!";
            break;
        }
    }
    return  0;
}