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
    /***********************��socket�й�*******************************************/
    //�����׽��ֶ�̬���ӿ�
    virtual int prepareEnvironment()=0;
    //�����׽���
//    virtual SOCKET createSocket();
//    //���׽���
//    virtual int bindSocket();
//    //���׽�����Ϊ����״̬
//    virtual int listenSocket();
//    //�����׽���
//    virtual SOCKET acceptSocket();
//    //��ָ���׽��ַ�����Ϣ
//    virtual SOCKET sendSocket();
//    //��ָ���׽��ֽ�����Ϣ
//    virtual SOCKET recvSocket();

    /***********************���߳��й�**��ϵͳ�����*****************************************/
    //�����߳�
    //HANDLE CreateThread();

    /***********************���ص�IO�й�****��ϵͳ�����***************************************/
    //���׽��ַ�������
//    virtual int onWSASend();
//    //���׽��ַ������ݰ�
//    virtual int onWSASendFrom();
//    //���׽��ֽ�������
//    virtual int onWSARecv();
//    //���׽��ֽ������ݰ�
//    virtual int onWSARecvFrom();

    /***********************��IOCP�й�****��ϵͳ�����***************************************/
    //������ɶ˿�
    //HANDLE WINAPI onCreateIOCompletionPort();
    //������ɶ˿�
    //HANDLE WINAPI onRelateCreateIOCompletionPort();
    //��ȡ�������״̬
    //BOOL WINAPI onGetQueuedCompletionStatus();
    //Ͷ��һ���������״̬
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
