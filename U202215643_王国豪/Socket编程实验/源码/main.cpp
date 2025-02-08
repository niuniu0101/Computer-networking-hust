#include"../newserver/tcpsocket.h"

int main() {
	cout << "Winsock初始化成功" << endl;
	cout << "监听Socket创建成功" << endl;
	init_Socket();
	string webaddr, document_root; int webport;
	getaddr(webaddr, webport, document_root);
	cout << "配置成功" << endl;
	SOCKET serfd =  createServerSocket(webaddr,webport);
	ioct(serfd); // 设置为
	printf("等待客服端连接请求....\n");

	fd_set rfds, wfds;
	list<pair<SOCKET, sockaddr_in>> sessionSockets;
	//设置接收缓冲区
	char recvBuf[4096];
	SOCKET sessionSocket;
	while (1) {
		FD_ZERO(&rfds), FD_ZERO(&wfds);
		FD_SET(serfd, &rfds);
		for (auto itor = sessionSockets.begin(); itor != sessionSockets.end(); itor++) {
			auto u = *itor;
			FD_SET(u.first, &rfds);
			FD_SET(u.first, &wfds);
		}
		// 这里有一个坑，就是说我们每次select以后，只会保留存在的描述符
		int readre = select(0, &rfds, NULL, NULL, NULL);
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);
		int wre = select(0, NULL, &wfds, NULL, NULL);
		cout << "此时的读请求有  " << readre << endl;
		if (FD_ISSET(serfd, &rfds)) {
			nTotal--; 
			// 每次都要创建一个新的地址
			sockaddr_in clientAddr; 	clientAddr.sin_family = AF_INET;
			int addrLen = sizeof(clientAddr); // 16个字节
			sessionSocket = accept(serfd, (LPSOCKADDR)&clientAddr, &addrLen);
			// 通过accept接受到客户端会话的套接字，专用于客户端通信
			if (sessionSocket != INVALID_SOCKET) {
				printf("Socket listen one client request!\n");
			}
			ioct(sessionSocket); // 设置为非阻塞
			sessionSockets.push_back({ sessionSocket,clientAddr });
		}
		int rtn;
		//检查会话SOCKET是否有数据到来
		if (nTotal > 0 && (nTotal!=wre)) {   // 我们有几个连接就会有几个可以写的信号
			for (auto itor = sessionSockets.begin(); itor != sessionSockets.end();) {
				if (FD_ISSET((*itor).first, &rfds)) { //如果遍历到的当前socket有数据到来
					// 打印一下端口号
					sockaddr_in nowaddr= (*itor).second;
					std::cout << "IP Address: " << inet_ntoa(nowaddr.sin_addr) << std::endl;
					std::cout << "Port: " << ntohs(nowaddr.sin_port) << std::endl;
					//receiving data from client
					memset(recvBuf, '\0', 4096);
					rtn = recv((*itor).first, recvBuf, 4096, 0);
					if (rtn > 0) {
						printf("Received %d bytes from client: \n%s\n", rtn, recvBuf);
						if (!FD_ISSET((*itor).first, &wfds)) {
							err("cant send data");
						}
						send_message((*itor).first, recvBuf, document_root.c_str());
						cout << "发送完毕" << endl;
					}
					else if (rtn==0) {
						closesocket((*itor).first);
						itor = sessionSockets.erase(itor);
						printf("成功关闭一个client\n");
						continue;
					}
					else {
						err("cant receive from client");
					}
				}
				itor++;
			}
		}
		Sleep(1000);
	}

	closesocket(serfd);
	close_Socket();
	return 0;
}