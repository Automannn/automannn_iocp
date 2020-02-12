//
// Created by 14394 on 2020/2/10.
//

#ifndef AUTOMANNN_IOCP_CLIENTSOCKET_H
#define AUTOMANNN_IOCP_CLIENTSOCKET_H

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <windows.h>

using namespace std;
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI SendMessageThread(LPVOID ipParam);
DWORD WINAPI ReceiveMessageThread(LPVOID ipParam);

class IClientSocket {
public:
    virtual int prepareEnvironment()=0;
//    virtual int onCreateSocket();
   // virtual int onConnect();
};

class CClientSocket :public IClientSocket{
public:
    CClientSocket();
    ~CClientSocket();
public:
    int prepareEnvironment() override;
    //int onCreateSocket();
  //  int connect() ;
};



#endif //AUTOMANNN_IOCP_CLIENTSOCKET_H
