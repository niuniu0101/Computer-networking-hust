// GBNmain.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "../rdt/stdafx.h"
#include "../rdt/Global.h"
#include "../rdt/RdtSender.h"
#include "../rdt/RdtReceiver.h"
#include "GBNSender.h"
#include "GBNReceiver.h"


int main(int argc, char* argv[])
{
	RdtSender* ps = new GBNSender();
	RdtReceiver* pr = new GBNReceiver();
	//	pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\Users\\23258\\Desktop\\���������\\ʵ��\\second\\input.txt");
	pns->setOutputFile("C:\\Users\\23258\\Desktop\\���������\\ʵ��\\second\\output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete

	return 0;
}