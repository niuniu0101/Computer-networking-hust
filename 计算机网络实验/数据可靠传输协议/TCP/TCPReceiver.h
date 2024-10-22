#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "../rdt/RdtReceiver.h"
#include <set>
#include<deque>

// 如何缓存呢
struct rcvPck {
	bool flag;       //指示该位置是否被占用，ture表示占用
	Packet winPck;   //存放数据包
};

class TCPReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	int seqlen;                     //序号宽度
	int winlen;                     // 窗口长度
	Packet lastAckPkt;				//上次发送的确认报文
	deque<rcvPck> window;

public:
	TCPReceiver();
	virtual ~TCPReceiver();

public:

	void receive(const Packet& packet);	//接收报文，将被NetworkService调用
};

#endif