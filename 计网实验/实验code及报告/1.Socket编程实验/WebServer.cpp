#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")
void sendHead(char* filename, char* extname, SOCKET s);
void sendData(char* filename, SOCKET s);

void main() {
	WSADATA wsaData;
	bool first_connection = true;

	int nRc = WSAStartup(0x0202, &wsaData); //��ʼ��Winsock
	if (nRc) {
		printf("Winsock��ʼ��ʧ��!\n");
		WSAGetLastError();
	}
	else if (wsaData.wVersion != 0x0202) {
		printf("Winsock�汾����ȷ!\n");
		WSAGetLastError();
	}
	else printf("Winsock��ʼ���ɹ�!\n");

	SOCKET srvSocket;
	sockaddr_in srvAddr;
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);  //����һ������socket
	if (srvSocket != INVALID_SOCKET) printf("Socket�����ɹ�!\n");
	else {
		printf("Socket����ʧ��!\n");
		WSAGetLastError();
	}
	printf("----------------------------------------------\n");

	int srvPort;
	char srvIP[20];
	char fileAddr[50], fileName[50]; //fileNameΪ������Ŀ¼�ĸ���
	printf("����������˿ںţ�"); //��IP�Ͷ˿ںţ��Լ���Ŀ¼·��
	scanf("%d", &srvPort);
	printf("���������IP��ַ��");
	scanf("%s", srvIP);
	printf("��������Ŀ¼��");
	scanf("%s", fileAddr);
	printf("----------------------------------------------\n");

	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(srvPort);
	srvAddr.sin_addr.s_addr = inet_addr(srvIP);

	nRc = bind(srvSocket, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));  //������
	if (nRc != SOCKET_ERROR)
		printf("Socket�󶨳ɹ�!\n");
	else {
		printf("Socket��ʧ��!\n");
		WSAGetLastError();
	}
	nRc = listen(srvSocket, 5);  //���õȴ�����״̬
	if (nRc != SOCKET_ERROR)
		printf("���õȴ�����״̬�ɹ�!\n");
	else {
		printf("���õȴ�����״̬ʧ��!\n");
		WSAGetLastError();
	}

	sockaddr_in cltAddr;
	cltAddr.sin_family = AF_INET;
	int addrLen = sizeof(cltAddr);
	while (true) {
		SOCKET cltSocket = accept(srvSocket, (LPSOCKADDR)&cltAddr, &addrLen);
		if (cltSocket != INVALID_SOCKET)
			printf("Socket��ͻ����ӳɹ�!\n");
		else {
			printf("Socket��ͻ�����ʧ��!\n");
			WSAGetLastError();
		}
		printf("----------------------------------------------\n");
		printf("�ͻ���IP��ַ��%s\n", inet_ntoa(cltAddr.sin_addr));
		printf("�ͻ��˶˿ںţ�%u\n", htons(cltAddr.sin_port));

		char rcvdata[2000] = "";
		nRc = recv(cltSocket, rcvdata, 2000, 0); //��������
		if (nRc != SOCKET_ERROR)
			printf("�������ݳɹ�!\n");
		else {
			printf("��������ʧ��!\n");
			WSAGetLastError();
		}
		printf("�ӿͻ����յ�%d�ֽ�����:\n %s\n", nRc, rcvdata);

		char rqtname[20] = "";  //�������У�����һ�У���ȡ������ļ���
		char extname[10] = "";  //�����ļ��ĺ�׺
		for (int i = 0; i < nRc; i++) {
			if (rcvdata[i] == '/') {
				for (int j = 0; j < nRc - i; j++) {
					if (rcvdata[i] != ' ') {
						rqtname[j] = rcvdata[i];
						i++;
					}
					else {
						rqtname[j + 1] = '\0';
						break;
					}
				}
				break;
			}
		}
		for (int k = 0; k < nRc; k++) {
			if (rcvdata[k] == '.') {
				for (int j = 0; j < nRc - k; j++) {
					if (rcvdata[k + 1] != ' ') {
						extname[j] = rcvdata[k + 1];
						k++;
					}
					else {
						extname[j + 1] = '\0';
						break;
					}
				}
				break;
			}
		}
		printf("----------------------------------------------\n");

		strcpy(fileName, fileAddr);
		printf("�����ļ�����%s\n", rqtname);
		strcat(fileName, rqtname);
		printf("����·����%s\n", fileName);
		sendHead(fileName, extname, cltSocket); //�����ײ�
		sendData(fileName, cltSocket); //����ʵ��
		printf("----------------------------------------------\n\n\n");

		closesocket(cltSocket);
	}

	closesocket(srvSocket);
	WSACleanup();
	return;
}

void sendHead(char* filename, char* extname, SOCKET s) {
	char content_Type[20] = ""; //����content-type

	if (strcmp(extname, "html") == 0) strcpy(content_Type, "text/html");
	if (strcmp(extname, "gif") == 0) strcpy(content_Type, "image/gif");
	if (strcmp(extname, "jpg") == 0) strcpy(content_Type, "image/jpeg");
	if (strcmp(extname, "jpeg") == 0) strcpy(content_Type, "image/jpeg");
	if (strcmp(extname, "png") == 0) strcpy(content_Type, "image/png");

	char sendContent_Type[40] = "Content-Type: ";
	strcat(sendContent_Type, content_Type);
	strcat(sendContent_Type, "\r\n");

	const char* ok_find = "HTTP/1.1 200 OK\r\n"; //200 OK
	const char* not_find = "HTTP/1.1 404 NOT FOUND\r\n"; //404 Not Found
	const char* forbidden = "HTTP/1.1 403 FORBIDDEN\r\n"; //404 Forbidden

	if (strcmp(filename, "C:/index/private.png") == 0) {  //403 ��ֹ����
		if (send(s, forbidden, strlen(forbidden), 0) == SOCKET_ERROR) {
			printf("����ʧ�ܣ�\n");
			return;
		}
		if (send(s, sendContent_Type, strlen(sendContent_Type), 0) == SOCKET_ERROR) {
			printf("����ʧ�ܣ�\n");
			return;
		}
		printf("403 �ͻ��������ֹ�����ļ���\n");
		return;
	}

	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {                    //404 �޷����ҵ��ļ�
		if (send(s, not_find, strlen(not_find), 0) == SOCKET_ERROR) {
			printf("����ʧ�ܣ�\n");
			return;
		}
		printf("404 �ͻ��������ļ�����ʧ�ܣ�\n");
	}
	else {                                  //200 �ļ����ҳɹ�
		if (send(s, ok_find, strlen(ok_find), 0) == SOCKET_ERROR) {
			printf("����ʧ�ܣ�\n");
			return;
		}
		printf("200 �ͻ��������ļ����ҳɹ���\n");
	}
	if (content_Type) {
		if (send(s, sendContent_Type, strlen(sendContent_Type), 0) == SOCKET_ERROR) {
			printf("����ʧ�ܣ�\n");
			return;
		}
	}
	if (send(s, "\r\n", 2, 0) == SOCKET_ERROR) {
		printf("����ʧ�ܣ�\n");
		return;
	}
	return;
}



void sendData(char* filename, SOCKET s) {
	FILE* fp_Data = fopen(filename, "rb");
	if (fp_Data == NULL) return;
	fseek(fp_Data, 0L, SEEK_END);
	int dataLen = ftell(fp_Data);  //�����ļ��ֽ���
	char* p = (char*)malloc(dataLen + 1);
	fseek(fp_Data, 0L, SEEK_SET);
	fread(p, dataLen, 1, fp_Data);
	if (send(s, p, dataLen, 0) == SOCKET_ERROR) {
		printf("����ʧ�ܣ�\n");
	}
	return;
}