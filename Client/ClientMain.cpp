#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<winsock2.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <process.h>
#include "Utile.h"
#include <string>
#include <vector>
using namespace std;


const int bufSize = 1024;
const int nameSize = 20;
const int LogSize = 15;
int LogCnt = 0;
vector<string> Log(LogSize);

char message[bufSize];
char name[nameSize] = "[Defalut]";

bool Wait = true;

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);

void ErrorHandling(const char* message);

int main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET hSocket_TCP;
	SOCKADDR_IN servAddr;

	HANDLE hThread1, hThread2;
	DWORD dwThread1, dwThread2;

	if (argc != 2)
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	//Load WinSocket 2.2 DLL
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStart() Error!");

	sprintf(name, "[%s]", argv[1]);

	//서버 접속을 위한 소켓 생성
	hSocket_TCP = socket(PF_INET, SOCK_STREAM, 0); // TCP 소켓 생성 -> SOCK_STREAM
	if (hSocket_TCP == INVALID_SOCKET)
		ErrorHandling("hSocket() Error!");

	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"/*IP*/); //inet_addr -> 문자열의 값을 unsigned long(big Endian) 타입으로 리턴 
	servAddr.sin_port = htons(atoi("7777"/*Port*/));

	if (SOCKET_ERROR == connect(hSocket_TCP, (SOCKADDR*)&servAddr, sizeof servAddr))
		ErrorHandling("connet() Error!");

	system("cls");
	gotoxy(10, 5);
	printf("Welcome to SiYoung Server!!!!");
	gotoxy(10, 7);
	printf("Please press the Enter key\n\n\n\n\n\n");

	while (Wait)
	{
		if (0x8001 & GetAsyncKeyState(VK_RETURN))
		{
			//ZeroMemory(message, sizeof message);
			memset(message, ' ', sizeof message);
			Wait = false;
		}
	}

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)hSocket_TCP, 0, (unsigned*)&dwThread1);

	hThread2 = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)hSocket_TCP, 0, (unsigned*)&dwThread2);

	
	if (NULL == hThread1 || NULL == hThread2 )
		ErrorHandling("쓰래드 생성 오류");

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	closesocket(hSocket_TCP);
	return 0;
}


unsigned WINAPI SendMsg(void* arg)
{
	SOCKET sock = SOCKET(arg);
	char nameMessage[nameSize + bufSize];

	while (!Wait)
	{
		gotoxy(0, 16);
		fgets(message, bufSize, stdin);
		sprintf(nameMessage, "%s : %s", name, message);
		if (!strcmp(message, "q\n"))
		{
			closesocket(sock);
			exit(0);
		}

		send(sock, nameMessage, strlen(nameMessage), 0);
		gotoxy(0, 16);
		memset(nameMessage, ' ', sizeof nameMessage);
		nameMessage[bufSize] = 0;
		printf("%s", nameMessage);
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
	SOCKET sock = (SOCKET)arg;
	char nameMessage[nameSize + bufSize];
	int strLen = 0;
	while (!Wait)
	{
		strLen = recv(sock, nameMessage, nameSize + bufSize - 1, 0);
		if (strLen == -1) return 1;

		nameMessage[strLen] = 0;
		if (LogCnt >= LogSize)
		{
			Log.erase(Log.begin());
			Log.push_back(nameMessage);
		}

		else
		{
			Log[LogCnt++] = (nameMessage);
		}
		memset(nameMessage,' ', sizeof nameMessage);
		nameMessage[100] = 0;
		for (int i = LogSize-1; i >= 0; --i)
		{
			gotoxy(0, i);
			printf("%s", nameMessage);
			gotoxy(0, i);
			printf("%s", Log[i].c_str());
		}

		gotoxy(0, 16);
	}
	return 0;
}



void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}