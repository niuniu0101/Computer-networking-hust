#include "../rdt/stdafx.h"
#include "../rdt/Global.h"
#include "SRSender.h"


SRSender::SRSender() :expectSequenceNumberSend(0), waitingState(false), base(0), winlen(4), seqlen(8)
{
}


SRSender::~SRSender()
{
}



bool SRSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = 1;
	else this->waitingState = 0;
	return this->waitingState;
}




bool SRSender::send(const Message& message) {
	if (this->getWaitingState()) { //�����ͷ�������ʱ���ܾ�����
		return false;
	}

	cout << "\n���ͷ����ͱ���ǰ�Ĵ���: [ ";
	for (int i = 0; i < this->winlen; i++) {
		if (i < window.size()) {
			if (window.at(i).flag == true)
				printf("%dY", (this->base + i) % seqlen);
			else printf("%dN", (this->base + i) % seqlen);
		}
		else printf("%d", (this->base + i) % seqlen);  // ��Ϊ���Ǵ����ڻ��п����λ��
		if ((base + i) % seqlen == expectSequenceNumberSend) cout << "*"; 
		cout << " ";
	}
	cout << "] \n"; 

	this->packetWaitingAck.acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	waitPck tempPck;
	tempPck.flag = false;
	tempPck.winPck = packetWaitingAck;
	window.push_back(tempPck);          //�������Ҵ�ȷ�ϵİ����봰�ڶ���
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);

	pns->startTimer(SENDER, Configuration::TIME_OUT, this->expectSequenceNumberSend);			//�������ͷ���ʱ������ʱû����һ����������Ӧ�Ķ�ʱ��

	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	cout << "\n���ͷ����ͱ��ĺ�Ĵ���: [ ";
	for (int i = 0; i < this->winlen; i++) {
		if (i < window.size()) {
			if (window.at(i).flag == true)
				printf("%dY", (this->base + i) % seqlen);
			else printf("%dN", (this->base + i) % seqlen);
		}
		else printf("%d", (this->base + i) % seqlen);
		if ((base + i) % seqlen == expectSequenceNumberSend) cout << "*";
		cout << " ";
	}
	cout << "] \n";

	return true;
}

void SRSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offseqnum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;
	//���У�����ȷ������ȷ��������ڷ��ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ��������
	if (checkSum == ackPkt.checksum && offseqnum < window.size() && window.at(offseqnum).flag == false) {
		//�жϽ��յ�ACK�Ƿ�Ϊ����ACK
		window.at(offseqnum).flag = true;//����յ�ack
		pns->stopTimer(SENDER, ackPkt.acknum);   //�رն�Ӧ���ݰ��Ķ�ʱ��

		cout << "���ͷ��յ�ack:" << ackPkt.acknum << endl;

		printf("���ͷ�����:[ ");
		for (int i = 0; i < this->winlen; i++) {
			if (i < window.size()) {
				if (window.at(i).flag == true)
					printf("%dY", (this->base + i) % seqlen);
				else printf("%dN", (this->base + i) % seqlen);
			}
			else printf("%d", (this->base + i) % seqlen);
			if ((base + i) % seqlen == expectSequenceNumberSend) cout << "*";
			cout << " ";
		}
		printf("]\n");  //����ACKǰ�Ĵ�������
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);

		while (window.size() != 0 && window.front().flag == true) {//��������
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //�����ڻ�����û���յ�����ACK��λ��

		printf("���ͷ������󴰿�:[ ");
		for (int i = 0; i < this->winlen; i++) {
			if (i < window.size()) {
				if (window.at(i).flag == true)
					printf("%dY", (this->base + i) % seqlen);
				else printf("%dN", (this->base + i) % seqlen);
			}
			else printf("%d", (this->base + i) % seqlen);
			if ((base + i)%seqlen == expectSequenceNumberSend) cout << "*";
			cout << " ";
		}
		printf("]\n");  //����ACK��Ĵ�������

	}
	else if (checkSum != ackPkt.checksum)
		pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
	else
		pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
}

void SRSender::timeoutHandler(int seqNum) {
	int offseqnum = (seqNum - this->base + this->seqlen) % this->seqlen;
	pns->stopTimer(SENDER, seqNum);  									//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, window.at(offseqnum).winPck);
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", window.at(offseqnum).winPck);
}