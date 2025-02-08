#include "tcpsocket.h"

bool init_Socket()
{
	// window socket async �첽�����׽���
	// parm1:����İ汾 ,parm2: �����Ĳ���
	WSADATA wsadata;
	int rtn = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (rtn != 0) {
		printf("WSAStartup failed code %d", WSAGetLastError()); // ʧ���˿��Դ�ӡ�������
		return false;
	}
	return false;
}

bool close_Socket()
{
	int rtn = WSACleanup();
	if (rtn != 0) {
		printf("WSAclean failed code %d", WSAGetLastError()); // ʧ���˿��Դ�ӡ�������
		return false;
	}
	return false;
}

// ��ʵsocket����һ������
SOCKET createServerSocket(string webaddr, int webport)
{
	// 1�����յ�socket,�������ֻ�
	// parm1:��ַЭ���� ipv4,ipv6
	// parm2:type, ����Э������ ��ʽ�׽���
	// parm3:protocl ����ʹ����һ��Э��
	SOCKET fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (fd == INVALID_SOCKET) {
		err("socket");
		return false;
	}

	// 2��socket��ip��ַ�Ͷ˿ںţ� �򿨲��ܴ�绰
	sockaddr_in addr;
	addr.sin_family = AF_INET; // Ҫ�����洴����socketһ��
	addr.sin_port = htons(webport); // ����֪�����ҵ��������һ�������أ�����ͨ���˿�
	addr.sin_addr.S_un.S_addr = inet_addr(webaddr.c_str());
	// htonl(INADDR_ANY);//inet_addr("127.0.0.1");

	if (SOCKET_ERROR == bind(fd, (LPSOCKADDR)&addr, sizeof(addr))) {
		err("bind");
		return false;
	}
	else {
		printf("bind�ɹ�\n");
	}

	// 3 �����绰
	// parm2 : �����������绰
	listen(fd, 20); 
	return fd;
}

SOCKET createClientSocket(const char *ip)
{
	// 1�����յ�socket,�������ֻ�
	// parm1:��ַЭ���� ipv4,ipv6
	// parm2:type, ����Э������ ��ʽ�׽���
	// parm3:protocl ����ʹ����һ��Э��
	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		err("socket");
		return false;
	}
	// 2 ���������������
	sockaddr_in addr;
	addr.sin_family = AF_INET; // Ҫ�����洴����socketһ��
	addr.sin_port = htons(port); // ����֪�����ҵ��������һ�������أ�����ͨ���˿�
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	if (INVALID_SOCKET == connect(fd, (sockaddr*) & addr, sizeof addr)) {
		err("connect");
		return false;
	}
	else {
		printf("�ͻ��������˽������ӳɹ�\n");
	}
	return fd;
}

void getaddr(string& addr, int& po, string& document_root)
{
	cout << "�����web������������ַ" << endl;
	cin >> addr;
	cout << "������˿ں�" << endl;
	cin >> po;
	cout << "������Ŀ¼��ַ" << endl;
	cin >> document_root;
}

void ioct(SOCKET& fd)
{
	u_long blockMode = 1;
	if ((ioctlsocket(fd, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		err("ioctlsocket");
	}
}

void send_message(SOCKET & sessionSocket, string recvbuf, string path)
{
	string filename, contenttype;
	int l = 0; while (recvbuf[l] != ' ') l++;
	int r = l + 1; while (recvbuf[r] != ' ') r++;
	filename = recvbuf.substr(l + 1, r - l - 1);
	string newpath = path + filename;


	// ��ӡ������
	int pos = 0; while (recvbuf[pos] != '\r') pos++;
	cout << "http���������� " << recvbuf.substr(0, pos) << endl;

	ifstream file(newpath, ios::binary);
	if (!file.is_open()) {
		// �ļ�δ�ҵ�������Ƿ�ΪͼƬ����
		printf("�ļ��޷��ҵ�\n");
		if (!newpath.ends_with("png") && !newpath.ends_with("jpg")) {
			// ����404����
			std::string errorResp = "HTTP/1.1 404 Not Found\r\n";
			errorResp.append("Content-Type: text/html\r\n");
			errorResp.append("Content-Length: 23\r\n\r\n");
			errorResp.append("<h1>404 Not Found</h1>");
			send(sessionSocket, errorResp.c_str(), errorResp.length(), 0);
			return;
		}
		else {
			// ���Դ�Ĭ�ϵ�ͼƬ
			printf("��Ҫ�ҵ�ͼƬ�����ڣ�����Ĭ�ϵ�ͼƬ\n");
			file.open("03.png", std::ios::binary);
			if (!file.is_open()) {
				// ���Ĭ��ͼƬҲ�޷��򿪣�����404����
				std::string errorResp = "HTTP/1.1 404 Not Found\r\n";
				errorResp.append("Content-Type: text/html\r\n");
				errorResp.append("Content-Length: 23\r\n\r\n");
				errorResp.append("<h1>404 Not Found</h1>");
				send(sessionSocket, errorResp.c_str(), errorResp.length(), 0);
				return;
			}
		}
	}
	
	// ��ȡ�ļ�����
	std::ostringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	if (filename.ends_with("html")) {
		contenttype = "text/html; charset=UTF-8";
	}
	else if (filename.ends_with("css")) {
		contenttype = "text/css; charset=UTF-8";
	}
	else if (filename.ends_with("js")) {
		contenttype = "application/js";
	}
	else if (filename.ends_with("jpg")) {
		contenttype = "image/jpg";
	}
	else if (filename.ends_with("png")) {
		contenttype = "image/png";
	}
	else {
		contenttype = "application/octet-stream";
	}


	// ����HTTP��Ӧ
	std::string resp;
	resp.append("HTTP/1.1 200 OK\r\n");
	resp.append("Server: VerySimpleServer\r\n");
	resp.append("Content-Type: ").append(contenttype).append("\r\n");
	resp.append("Content-Length: ").append(std::to_string(content.length())).append("\r\n");
	resp.append("\r\n");
	resp.append(content);

	int rtn = send(sessionSocket, resp.c_str(), resp.length(), 0);
	if (rtn > 0) {
		printf("Send %d bytes to client\n", rtn);
		printf("�ѷ���filename:%s\n", filename.c_str());
	}

}
