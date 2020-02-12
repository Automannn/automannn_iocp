//
// Created by 14394 on 2020/2/10.
//

#include "main.h"



int main(){
    CClientSocket* clientSocket= new CClientSocket();
    clientSocket->prepareEnvironment();
    delete clientSocket;
}