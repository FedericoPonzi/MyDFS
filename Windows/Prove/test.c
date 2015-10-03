#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#define DEBUG 1


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9001"

void CreateSocket();
int main(int argc, char* argv[])
{
	CreateSocket();

	return 0;
}

void CreateSocket()
{
	WSADATA wsaData;
	SOCKET ListenSocket, ClientSocket = INVALID_SOCKET;
	int iResult = 0;
	struct sockaddr_in service;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);	
	char recvbuf[100];
		
	service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(0); //DEFAULT_PORT

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	printf("socket: %d\n", ListenSocket);
	
	iResult = bind( ListenSocket, (SOCKADDR *) &service, sizeof (service));
	printf("bind: %d\n", iResult);
	
	
	iResult = listen(ListenSocket, 4);
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	
    getsockname(ListenSocket, (struct sockaddr *) &sin, &len);
	printf("Sto richiedendo la connessione sulla porta numero: %d\n", ntohs(sin.sin_port));

	ClientSocket = accept(ListenSocket, NULL, NULL);
	printf("Ciao\n");
	
	closesocket(ListenSocket);
	printf("Ciao\n");
	
	iResult = recv(ClientSocket, recvbuf, sizeof(recvbuf), 0);
	
	closesocket(ClientSocket);
	WSACleanup();
	
}

