//
// Created by 14394 on 2020/2/9.
//

#include "main.h"
int main(){
    CServerSocket* serverSocket = new CServerSocket();
    serverSocket->prepareEnvironment();
    delete serverSocket;
}