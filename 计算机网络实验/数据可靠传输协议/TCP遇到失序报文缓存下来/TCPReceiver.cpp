#include "../rdt/stdafx.h"
#include "../rdt/Global.h"
#include "TCPReceiver.h"


TCPReceiver::TCPReceiver() :expectSequenceNumberRcvd(0), seqlen(8),winlen(4)
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < winlen; i++) {
		rcvPck blank;
		blank.flag = false;
		blank.winPck.seqnum = -1;
		window.push_back(blank);
	}                                //����������������Ϊ�հ���flag��Ϊfalse�������������
}


TCPReceiver::~TCPReceiver()
{
}

void TCPReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	int base = (this->expectSequenceNumberRcvd);
	int offseqnum = (packet.seqnum - base + this->seqlen) % this->seqlen;

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum && offseqnum<winlen) {

		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);

		window.at(offseqnum).flag = true;
		window.at(offseqnum).winPck = packet;

		printf("���շ��յ����ĺ�Ĵ���:[ ");
		for (int i = 0; i < window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (base + i) % seqlen);
			else printf("%dN ", (base + i) % seqlen);
		}
		printf("]\n");  //���յ�����ǰ�Ĵ�������

		//ȡ��Message�����ϵݽ���Ӧ�ò�

		while (window.front().flag == true) {
			Message msg;
			memcpy(msg.data, window.front().winPck.payload, sizeof(window.front().winPck.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			// ��ʼ���������ڴ�����һ�����
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqlen;
			rcvPck blank;
			blank.flag = false;
			blank.winPck.seqnum = -1;

			lastAckPkt = window.front().winPck;  // ��¼��һ��

			window.pop_front();
			window.push_back(blank); //�������һ���һ��

		}

		base = expectSequenceNumberRcvd;
		printf("���շ������󴰿�:[ ");
		for (int i = 0; i < window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (base + i) % seqlen);
			else printf("%dN ", (base + i) % seqlen);
		}
		printf("]\n");  //���ձ��ĺ�Ĵ�������

		lastAckPkt.acknum = lastAckPkt.seqnum;
		lastAckPkt.seqnum = -1;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

		// this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % this->seqlen; //���������0-7֮���л�
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);

			// ������Ҫ����һ��
			if (window.size()>offseqnum && window.at(offseqnum).flag == false) {

				window.at(offseqnum).flag = true;
				window.at(offseqnum).winPck = packet;

				printf("���շ��յ�ʧ���ĺ󴰿�:[ ");
				for (int i = 0; i < window.size(); i++) {
					if (window.at(i).flag == true)
						printf("%dY ", (base + i) % seqlen);
					else printf("%dN ", (base + i) % seqlen);
				}
				printf("]\n");  //����ʧ���ĺ�Ĵ�������

				for (int i = 1; i < 10; i++) cout << 1 << endl;

			}


		}
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���

	}
}