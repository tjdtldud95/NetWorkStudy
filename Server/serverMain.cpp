#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <process.h>

const int bufSize = 1024;

unsigned WINAPI clientConn(void* arg);
void SendMsg(char* Message, int len);
void ErrorHandling(const char* message);

int cLntNumber = 0;
SOCKET clntSocks[10];
HANDLE hMutex;

int main(int argc,char** argv)
{
	WSADATA wsaData;
	SOCKET servSock;
	SOCKET clntSock;

	SOCKADDR_IN servAddr;
	SOCKADDR_IN ClntAddr;
	int ClntAddrSize;

	HANDLE hThread;
	DWORD dwThreadID;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup Error!");
	
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == hMutex)
		ErrorHandling("뮤텍스 생성 오류!");

	servSock = socket(PF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == servSock)
		ErrorHandling("소켓 생성 오류!");

	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if(bind(servSock,(SOCKADDR*)&servAddr,sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("바인딩 오류!");

	if (listen(servSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen Error");

	while (true)
	{
		ClntAddrSize = sizeof ClntAddr;
		clntSock = accept(servSock, (SOCKADDR*)&ClntAddr, &ClntAddrSize);

		if (clntSock == INVALID_SOCKET)
			ErrorHandling("Accept() error");
		
		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[cLntNumber++] = clntSock;
		cLntNumber %= 10;
		ReleaseMutex(hMutex);

		printf("새로운 연결, 클라이언트 IP : %s \n", inet_ntoa(ClntAddr.sin_addr));
		printf("현재 인원 : %d \n", cLntNumber);
		hThread = (HANDLE)_beginthreadex(NULL, 0, clientConn, (void*)clntSock, 0, (unsigned*)&dwThreadID);

		if (hThread == 0)
			ErrorHandling("쓰레드 생성 오류!");
	}

	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

unsigned WINAPI clientConn(void* arg)
{
	SOCKET clntSock = SOCKET(arg);
	int strLen = 0;
	char Message[bufSize] = { 0, };
	while ((strLen = recv(clntSock, Message, bufSize, 0)) != 0)
	{
		SendMsg(Message, strLen);	
	}

	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cLntNumber; ++i) // 클라 종료시
	{
		if (clntSock == clntSocks[i])
		{
			for (; i < cLntNumber - 1; ++i)
				clntSocks[i] = clntSocks[i + 1];

			cLntNumber--;
			printf("클라이언트 연결 종료,  현재 인원 : %d \n", cLntNumber);
			break;
		}
	}
	ReleaseMutex(hMutex);
	closesocket(clntSock);
	return 0;
}

void SendMsg(char* Message, int len)
{
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cLntNumber; ++i)
	{
		send(clntSocks[i], Message, len, 0);
	}
	ReleaseMutex(hMutex);
}