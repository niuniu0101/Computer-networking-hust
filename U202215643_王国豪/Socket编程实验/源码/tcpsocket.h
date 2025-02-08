#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_
#include "winsock2.h"
#include<iostream>
#include<stdio.h>
#include <fstream>
#include <sstream>
#include<list>
#include <string>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
#define port 5050

#define err(errMsg) printf("[line%d]%s failed code %d",__LINE__,errMsg, WSAGetLastError());

// �������
bool init_Socket();
// �ر������
bool close_Socket();

// ����������socket
SOCKET createServerSocket(string webaddr,int webport);

// �����ͷ���socket
SOCKET createClientSocket(const char *ip);

// �������ǵļ�����ַ���˿ڣ���Ŀ¼
void getaddr(string& addr, int& po, string& document_root);

// ���÷�����״̬
void ioct(SOCKET& fd);

// ���ͱ�����Ϣ
void send_message(SOCKET & sessionSocket, string recvbuf,string path);

#endif // ! _TCPSOCKET_H_