#include "stdafx.h"
#include "Global.h"
#include "TCPSender.h"
#include<deque>

TCPSender::TCPSender():expectSequenceNumberSend(0),waitingState(false),base(0),winlen(4),seqlen(8),Rdnum(0)
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




bool TCPSender::send(const Message &message) {
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


void TCPSender::receive(const Packet &ackPkt) {
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

			this->Rdnum = 0;  //�յ���ȷ��ʱ���������������
			if (window.size() != 0) {
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//�Ի�׼������ſ�����ʱ��
			}

		}
		else if (checkSum != ackPkt.checksum)
			pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
		else if (ackPkt.acknum == (this->base + this->seqlen - 1) % this->seqlen) {
			pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
			this->Rdnum++;
			if (this->Rdnum == 3 && window.size() > 0) {
				pUtils->printPacket("���ͷ����������ش����ƣ��ش����緢����û��ȷ�ϵı��Ķ�", window.front());
				pns->sendToNetworkLayer(RECEIVER, window.front());
				this->Rdnum = 0;
			}
		}

}

void TCPSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط����緢����û��ȷ�ϵı��Ķ�", window.front());
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, window.front());	             //���·������緢����û��ȷ�ϵı��Ķ�
}
