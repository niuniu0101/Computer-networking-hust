#include"../newserver/tcpsocket.h"

int main() {
	cout << "Winsock��ʼ���ɹ�" << endl;
	cout << "����Socket�����ɹ�" << endl;
	init_Socket();
	string webaddr, document_root; int webport;
	getaddr(webaddr, webport, document_root);
	cout << "���óɹ�" << endl;
	SOCKET serfd =  createServerSocket(webaddr,webport);
	ioct(serfd); // ����Ϊ
	printf("�ȴ��ͷ�����������....\n");

	fd_set rfds, wfds;
	list<pair<SOCKET, sockaddr_in>> sessionSockets;
	//���ý��ջ�����
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
		// ������һ���ӣ�����˵����ÿ��select�Ժ�ֻ�ᱣ�����ڵ�������
		int readre = select(0, &rfds, NULL, NULL, NULL);
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);
		int wre = select(0, NULL, &wfds, NULL, NULL);
		cout << "��ʱ�Ķ�������  " << readre << endl;
		if (FD_ISSET(serfd, &rfds)) {
			nTotal--; 
			// ÿ�ζ�Ҫ����һ���µĵ�ַ
			sockaddr_in clientAddr; 	clientAddr.sin_family = AF_INET;
			int addrLen = sizeof(clientAddr); // 16���ֽ�
			sessionSocket = accept(serfd, (LPSOCKADDR)&clientAddr, &addrLen);
			// ͨ��accept���ܵ��ͻ��˻Ự���׽��֣�ר���ڿͻ���ͨ��
			if (sessionSocket != INVALID_SOCKET) {
				printf("Socket listen one client request!\n");
			}
			ioct(sessionSocket); // ����Ϊ������
			sessionSockets.push_back({ sessionSocket,clientAddr });
		}
		int rtn;
		//���ỰSOCKET�Ƿ������ݵ���
		if (nTotal > 0 && (nTotal!=wre)) {   // �����м������Ӿͻ��м�������д���ź�
			for (auto itor = sessionSockets.begin(); itor != sessionSockets.end();) {
				if (FD_ISSET((*itor).first, &rfds)) { //����������ĵ�ǰsocket�����ݵ���
					// ��ӡһ�¶˿ں�
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
						cout << "�������" << endl;
					}
					else if (rtn==0) {
						closesocket((*itor).first);
						itor = sessionSockets.erase(itor);
						printf("�ɹ��ر�һ��client\n");
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