//
// Created by 14394 on 2020/2/9.
//

#ifndef AUTOMANNN_IOCP_IOCPSERVERSOCKET_H
#define AUTOMANNN_IOCP_IOCPSERVERSOCKET_H

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <vector>


class IServerSocket{
public:
    /***********************与socket有关*******************************************/
    //链接套接字动态链接库
    virtual int prepareEnvironment()=0;
    //创建套接字
//    virtual SOCKET createSocket();
//    //绑定套接字
//    virtual int bindSocket();
//    //将套接字设为监听状态
//    virtual int listenSocket();
//    //接收套接字
//    virtual SOCKET acceptSocket();
//    //向指定套接字发送信息
//    virtual SOCKET sendSocket();
//    //从指定套接字接收信息
//    virtual SOCKET recvSocket();

    /***********************与线程有关**由系统库完成*****************************************/
    //创建线程
    //HANDLE CreateThread();

    /***********************与重叠IO有关****由系统库完成***************************************/
    //向套接字发送数据
//    virtual int onWSASend();
//    //向套接字发送数据包
//    virtual int onWSASendFrom();
//    //从套接字接收数据
//    virtual int onWSARecv();
//    //从套接字接收数据包
//    virtual int onWSARecvFrom();

    /***********************与IOCP有关****由系统库完成***************************************/
    //创建完成端口
    //HANDLE WINAPI onCreateIOCompletionPort();
    //关联完成端口
    //HANDLE WINAPI onRelateCreateIOCompletionPort();
    //获取队列完成状态
    //BOOL WINAPI onGetQueuedCompletionStatus();
    //投递一个队列完成状态
    //BOOL WINAPI onPostQueuedCompletionStatus();
};

class CServerSocket : public IServerSocket{
public:
    CServerSocket();
    ~CServerSocket();

public:
    int prepareEnvironment();
//    SOCKET createSocket() override;
//    int bindSocket() override;
//    int listenSocket() override;
//    SOCKET acceptSocket() override;
//    SOCKET sendSocket() override;
//    SOCKET recvSocket() override;
//
//    int onWSARecv() override;
//    int onWSARecvFrom() override;
//    int onWSASend() override;
//    int onWSASendFrom() override;
};


#endif //AUTOMANNN_IOCP_IOCPSERVERSOCKET_H
