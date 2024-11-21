#include "../rdt/stdafx.h"
#include "../rdt/Global.h"
#include "TCPReceiver.h"


TCPReceiver::TCPReceiver() :expectSequenceNumberRcvd(0), seqlen(8),winlen(4)
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < winlen; i++) {
		rcvPck blank;
		blank.flag = false;
		blank.winPck.seqnum = -1;
		window.push_back(blank);
	}                                //将窗口填满，但都为空包，flag置为false，方便后续操作
}


TCPReceiver::~TCPReceiver()
{
}

void TCPReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	int base = (this->expectSequenceNumberRcvd);
	int offseqnum = (packet.seqnum - base + this->seqlen) % this->seqlen;

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum && offseqnum<winlen) {

		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		window.at(offseqnum).flag = true;
		window.at(offseqnum).winPck = packet;

		printf("接收方收到报文后的窗口:[ ");
		for (int i = 0; i < window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (base + i) % seqlen);
			else printf("%dN ", (base + i) % seqlen);
		}
		printf("]\n");  //接收到报文前的窗口序列

		//取出Message，向上递交给应用层

		while (window.front().flag == true) {
			Message msg;
			memcpy(msg.data, window.front().winPck.payload, sizeof(window.front().winPck.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			// 开始更新我们期待的下一个序号
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqlen;
			rcvPck blank;
			blank.flag = false;
			blank.winPck.seqnum = -1;

			lastAckPkt = window.front().winPck;  // 记录上一个

			window.pop_front();
			window.push_back(blank); //窗口向右滑动一格

		}

		base = expectSequenceNumberRcvd;
		printf("接收方滑动后窗口:[ ");
		for (int i = 0; i < window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (base + i) % seqlen);
			else printf("%dN ", (base + i) % seqlen);
		}
		printf("]\n");  //接收报文后的窗口序列

		lastAckPkt.acknum = lastAckPkt.seqnum;
		lastAckPkt.seqnum = -1;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		// this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % this->seqlen; //接收序号在0-7之间切换
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);

			// 我们需要缓存一下
			if (window.size()>offseqnum && window.at(offseqnum).flag == false) {

				window.at(offseqnum).flag = true;
				window.at(offseqnum).winPck = packet;

				printf("接收方收到失序报文后窗口:[ ");
				for (int i = 0; i < window.size(); i++) {
					if (window.at(i).flag == true)
						printf("%dY ", (base + i) % seqlen);
					else printf("%dN ", (base + i) % seqlen);
				}
				printf("]\n");  //接收失序报文后的窗口序列

				for (int i = 1; i < 10; i++) cout << 1 << endl;

			}


		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}