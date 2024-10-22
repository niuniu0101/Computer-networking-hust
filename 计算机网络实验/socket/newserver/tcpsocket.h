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

// 打开网络库
bool init_Socket();
// 关闭网络库
bool close_Socket();

// 创建服务器socket
SOCKET createServerSocket(string webaddr,int webport);

// 创建客服端socket
SOCKET createClientSocket(const char *ip);

// 配置我们的监听地址，端口，主目录
void getaddr(string& addr, int& po, string& document_root);

// 设置非阻塞状态
void ioct(SOCKET& fd);

// 发送报文信息
void send_message(SOCKET & sessionSocket, string recvbuf,string path);

#endif // ! _TCPSOCKET_H_