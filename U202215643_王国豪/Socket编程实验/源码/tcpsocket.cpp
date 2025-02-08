#include "tcpsocket.h"

bool init_Socket()
{
	// window socket async 异步启动套接字
	// parm1:请求的版本 ,parm2: 传出的参数
	WSADATA wsadata;
	int rtn = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (rtn != 0) {
		printf("WSAStartup failed code %d", WSAGetLastError()); // 失败了可以打印错误代码
		return false;
	}
	return false;
}

bool close_Socket()
{
	int rtn = WSACleanup();
	if (rtn != 0) {
		printf("WSAclean failed code %d", WSAGetLastError()); // 失败了可以打印错误代码
		return false;
	}
	return false;
}

// 其实socket就是一个整数
SOCKET createServerSocket(string webaddr, int webport)
{
	// 1创建空的socket,买了新手机
	// parm1:地址协议族 ipv4,ipv6
	// parm2:type, 传输协议类型 流式套接字
	// parm3:protocl 具体使用哪一个协议
	SOCKET fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (fd == INVALID_SOCKET) {
		err("socket");
		return false;
	}

	// 2给socket绑定ip地址和端口号， 买卡才能打电话
	sockaddr_in addr;
	addr.sin_family = AF_INET; // 要和上面创建的socket一样
	addr.sin_port = htons(webport); // 我们知道是找到程序的哪一个程序呢，就是通过端口
	addr.sin_addr.S_un.S_addr = inet_addr(webaddr.c_str());
	// htonl(INADDR_ANY);//inet_addr("127.0.0.1");

	if (SOCKET_ERROR == bind(fd, (LPSOCKADDR)&addr, sizeof(addr))) {
		err("bind");
		return false;
	}
	else {
		printf("bind成功\n");
	}

	// 3 监听电话
	// parm2 : 可以听几个电话
	listen(fd, 20); 
	return fd;
}

SOCKET createClientSocket(const char *ip)
{
	// 1创建空的socket,买了新手机
	// parm1:地址协议族 ipv4,ipv6
	// parm2:type, 传输协议类型 流式套接字
	// parm3:protocl 具体使用哪一个协议
	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		err("socket");
		return false;
	}
	// 2 与服务器建立连接
	sockaddr_in addr;
	addr.sin_family = AF_INET; // 要和上面创建的socket一样
	addr.sin_port = htons(port); // 我们知道是找到程序的哪一个程序呢，就是通过端口
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	if (INVALID_SOCKET == connect(fd, (sockaddr*) & addr, sizeof addr)) {
		err("connect");
		return false;
	}
	else {
		printf("客户端与服务端建立连接成功\n");
	}
	return fd;
}

void getaddr(string& addr, int& po, string& document_root)
{
	cout << "请输出web服务器监听地址" << endl;
	cin >> addr;
	cout << "请输入端口号" << endl;
	cin >> po;
	cout << "请输入目录地址" << endl;
	cin >> document_root;
}

void ioct(SOCKET& fd)
{
	u_long blockMode = 1;
	if ((ioctlsocket(fd, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
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


	// 打印命令行
	int pos = 0; while (recvbuf[pos] != '\r') pos++;
	cout << "http请求命令是 " << recvbuf.substr(0, pos) << endl;

	ifstream file(newpath, ios::binary);
	if (!file.is_open()) {
		// 文件未找到，检查是否为图片请求
		printf("文件无法找到\n");
		if (!newpath.ends_with("png") && !newpath.ends_with("jpg")) {
			// 发送404错误
			std::string errorResp = "HTTP/1.1 404 Not Found\r\n";
			errorResp.append("Content-Type: text/html\r\n");
			errorResp.append("Content-Length: 23\r\n\r\n");
			errorResp.append("<h1>404 Not Found</h1>");
			send(sessionSocket, errorResp.c_str(), errorResp.length(), 0);
			return;
		}
		else {
			// 尝试打开默认的图片
			printf("你要找的图片不存在，将打开默认的图片\n");
			file.open("03.png", std::ios::binary);
			if (!file.is_open()) {
				// 如果默认图片也无法打开，返回404错误
				std::string errorResp = "HTTP/1.1 404 Not Found\r\n";
				errorResp.append("Content-Type: text/html\r\n");
				errorResp.append("Content-Length: 23\r\n\r\n");
				errorResp.append("<h1>404 Not Found</h1>");
				send(sessionSocket, errorResp.c_str(), errorResp.length(), 0);
				return;
			}
		}
	}
	
	// 获取文件内容
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


	// 构建HTTP响应
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
		printf("已发送filename:%s\n", filename.c_str());
	}

}
