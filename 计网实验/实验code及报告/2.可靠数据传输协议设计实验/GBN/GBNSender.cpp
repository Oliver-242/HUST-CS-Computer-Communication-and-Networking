#include "stdafx.h"
#include "Global.h"
#include "GBNSender.h"
#include<deque>

GBNSender::GBNSender():expectSequenceNumberSend(0),waitingState(false),base(0),winlen(4),seqlen(8)
{
}


GBNSender::~GBNSender()
{
}



bool GBNSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = 1;
	else this->waitingState = 0;
	return this->waitingState;
}




bool GBNSender::send(const Message &message) {
	if (this->getWaitingState()) { //�����ͷ�������ʱ���ܾ�����
		return false;
	}

	this->packetWaitingAck.acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);               //�������͵İ����봰�ڶ���
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);

	if(this->base == this->expectSequenceNumberSend)
		pns->startTimer(SENDER, Configuration::TIME_OUT,this->base);			//�������ͷ���ʱ��
	
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;
	
	return true;
}

void GBNSender::receive(const Packet &ackPkt) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

		//���У�����ȷ������ȷ��������ڷ��ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ��������
		if (checkSum == ackPkt.checksum && offacknum < window.size()) {
			//�жϽ��յ�ACK�Ƿ�Ϊ����ACK

			printf("���ͷ�����:[ ");
			for (int i = 0; i < this->winlen; i++) {
				printf("%d ", (this->base + i) % this->seqlen);
			}
			printf("]\n");  //����ACKǰ�Ĵ�������

			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);

			pns->stopTimer(SENDER, this->base);   //����ÿ�ο����Ķ�ʱ��������baseΪ��׼�ģ�������base�رն�ʱ��
			while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//��������
				window.pop_front();
				this->base = (this->base + 1) % this->seqlen;
			}  //���ѳɹ����յ�ACK��֮ǰ���˳����У��൱�ڽ������𽥻�����ACK+1��λ��

			printf("���ͷ������󴰿�:[ ");
			for (int i = 0; i < this->winlen; i++) {
				printf("%d ", (this->base + i) % this->seqlen);
			}
			printf("]\n");  //���ճɹ���Ĵ���ֵ

			if (window.size() != 0) {
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//�Ի�׼������ſ�����ʱ��
			}

		}
		else if (checkSum != ackPkt.checksum)
			pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
		else
			pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
}

void GBNSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//�����������ͷ���ʱ��
	for (int i = 0; i < window.size(); i++) {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط����ڱ���", window.at(i));
		pns->sendToNetworkLayer(RECEIVER, window.at(i));
	}		                                                             //���·��ʹ����ڵ����д�ȷ�����ݰ�

}
