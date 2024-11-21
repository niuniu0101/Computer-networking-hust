#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "../rdt/RdtReceiver.h"
#include <set>
#include<deque>

// ��λ�����
struct rcvPck {
	bool flag;       //ָʾ��λ���Ƿ�ռ�ã�ture��ʾռ��
	Packet winPck;   //������ݰ�
};

class TCPReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	int seqlen;                     //��ſ��
	int winlen;                     // ���ڳ���
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
	deque<rcvPck> window;

public:
	TCPReceiver();
	virtual ~TCPReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif