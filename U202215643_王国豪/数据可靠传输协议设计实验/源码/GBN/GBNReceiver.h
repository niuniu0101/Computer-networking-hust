#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "../rdt/RdtReceiver.h"
class GBNReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������,����ack
	int seqlen;                     //��ſ��
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
public:
	GBNReceiver();
	virtual ~GBNReceiver();
public:
	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif