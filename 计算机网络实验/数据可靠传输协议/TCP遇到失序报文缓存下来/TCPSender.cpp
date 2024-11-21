#include "../rdt/stdafx.h"
#include "../rdt/Global.h"
#include "TCPSender.h"
#include<deque>

TCPSender::TCPSender() :expectSequenceNumberSend(0), waitingState(false), base(0), winlen(4), seqlen(8), Rdnum(0)
{
}


TCPSender::~TCPSender()
{
}



bool TCPSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = true;
	else this->waitingState = false;
	return this->waitingState;
}



// 我们的tcp是采取累计确认的
bool TCPSender::send(const Message& message) {
	if (this->getWaitingState()) { //当发送方窗口满时，拒绝发送
		return false;
	}

	cout << "\n发送方发送报文前的窗口: [ ";
	int ok = 1;
	for (int i = 0; i < this->winlen; i++) {
		int nownum = (base + i) % seqlen; cout << nownum; 
		if (nownum == expectSequenceNumberSend) {
			cout << "*"; ok = 0;
		}if (ok) cout << "N";
		cout << " ";
	}
	cout << "] \n";

	this->packetWaitingAck.acknum = -1; //忽略该字段
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);               //将待发送的包加入窗口队列
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);

	if (this->base == this->expectSequenceNumberSend)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);			//启动发送方定时器

	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	cout << "\n发送方发送报文后的窗口: [ ";
	ok = 1;
	for (int i = 0; i < this->winlen; i++) {
		int nownum = (base + i) % seqlen; cout << nownum;
		if (nownum == expectSequenceNumberSend) {
			cout << "*"; ok = 0;
		}if (ok) cout << "N";
		cout << " ";
	}
	cout << "] \n";

	return true;
}


void TCPSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	//如果校验和正确，并且确认序号是在发送方已发送并等待确认的数据包序号列中
	// 因为我们是采取累积确认
	if (checkSum == ackPkt.checksum && offacknum < window.size()) {
		//判断接收的ACK是否为可用ACK

		pUtils->printPacket("发送方正确收到确认", ackPkt);

		pns->stopTimer(SENDER, this->base);   //由于每次开启的定时器都是以base为基准的，所以以base关闭定时器


		printf("发送方接收到ack后的窗口:[ ");
		int ok = 2;
		for (int i = 0; i < this->winlen; i++) {
			int nownum = (this->base + i) % this->seqlen;
			if (nownum == expectSequenceNumberSend) ok--;
			cout << nownum;
			if (ok == 2) {
				cout << "Y";
			}
			else if (ok) cout << "N";
			if (nownum == ackPkt.acknum) {
				ok--;
			}
			if (nownum == expectSequenceNumberSend) cout << "*";
			cout << " ";
		}
		printf("]\n");


		// 因为采取的是累计确认

		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//滑动窗口
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //将已成功接收的ACK及之前的退出队列，相当于将窗口逐渐滑动到ACK+1的位置
		this->Rdnum = 0;  //收到正确包时，将冗余记数重置
		if (window.size() != 0) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//以基准包的序号开启计时器
		}

		printf("发送方滑动后的窗口:[ ");
		ok = 1;
		for (int i = 0; i < this->winlen; i++) {
			int nownum = (this->base + i) % this->seqlen;
			if (nownum == expectSequenceNumberSend) ok--;
			cout << nownum;
			if (ok == 1) {
				cout << "N";
			}
			if (nownum == expectSequenceNumberSend) cout << "*";
			cout << " ";
		}
		printf("]\n");

	}
	else if (checkSum != ackPkt.checksum)
		pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
	else if (ackPkt.acknum == (this->base + this->seqlen - 1) % this->seqlen) {
		pUtils->printPacket("发送方已正确收到过该报文确认", ackPkt);
		this->Rdnum++;
		if (this->Rdnum == 3 && window.size() > 0) {
			for (int i = 1; i < 10; i++) {
				cout << "我要开始快速重传了" << endl;
			}
			pUtils->printPacket("发送方启动快速重传机制，重传最早发送且没被确认的报文段", window.front());
			pns->sendToNetworkLayer(RECEIVER, window.front());
			this->Rdnum = 0;
		}
	}

}

void TCPSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("发送方定时器时间到，重发最早发送且没被确认的报文段", window.front());
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, window.front());	             //重新发送最早发送且没被确认的报文段
}