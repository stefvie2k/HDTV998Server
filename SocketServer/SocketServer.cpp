// SocketServer.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SocketServer.h"

typedef int (CALLBACK* PFN_ONRECEIVESETUP)(int a1, int a2, const void* a3);

SOCKET g_listen = INVALID_SOCKET;
HANDLE g_hAbortEvent = NULL;
HANDLE g_hThreadProc = INVALID_HANDLE_VALUE;
PFN_ONRECEIVESETUP g_pfnOnReceive = NULL;

#define API_BUFFER_SIZE	0x80000
#define READ_BUFFER_SIZE 0x02000

void log(const char* format, ...)
{
	va_list argptr;
	time_t ltime = time(NULL);
	const char* timestamp = asctime(localtime(&ltime));
	printf("%.*s ", strlen(timestamp) - 1, timestamp); 
	va_start(argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
}

DWORD WINAPI SocketThread(LPVOID lpParameter)
{
	log("SocketThread started...\n");

	while (WaitForSingleObject(g_hAbortEvent, 0) == WAIT_TIMEOUT)
	{
		CHAR szSpinner[] = "-\\|/";
		CHAR cbBuffer[API_BUFFER_SIZE];
		int nOffset = 0;

		log("Waiting for client...\n");

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		SOCKET client = accept(g_listen, (struct sockaddr*)&addr, &addrlen);
		if (client == INVALID_SOCKET)
			continue;

		log("Client %s:%d connected!\n", inet_ntoa(addr.sin_addr), (int) ntohs(addr.sin_port));

		int nSpinner = 0;
		while (WaitForSingleObject(g_hAbortEvent, 0) == WAIT_TIMEOUT)
		{
			int nLen = sizeof(cbBuffer) - nOffset;
			if (nLen > READ_BUFFER_SIZE)
				nLen = READ_BUFFER_SIZE;
			nLen = recv(client, cbBuffer + nOffset, nLen, 0);
			if (nLen == SOCKET_ERROR || nLen == 0)
			{
				log("recv() failed (%d)!\n", WSAGetLastError());
				break;
			}

			nOffset += nLen;

			if (nOffset >= sizeof(cbBuffer))
			{
				g_pfnOnReceive(0, 0, (void*)cbBuffer);
				nOffset = 0;

				printf("%c\r", szSpinner[nSpinner++ % 4]);
			}
		}

		closesocket(client);
		client = INVALID_SOCKET;
	}

	log("SocketThread ended.\n");

	return 0;
}

extern "C" long WINAPI SockServer_OnReceiveSetup(PFN_ONRECEIVESETUP pfnOnReceive)
{
	log("SockServer_OnReceiveSetup  : pfnOnReceive = 0x%08X\n", pfnOnReceive);

	g_pfnOnReceive = pfnOnReceive;

	return 0;
}

extern "C" long WINAPI SockServer_Initialize(int nPort)
{
	log("SockServer_Initialize : nPort = %d\n", nPort);

	WSADATA wsaData;

	// The socket address to be passed to bind
	sockaddr_in service;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		log("Error at WSAStartup()\n");
		return 1;
	}

	// Create a SOCKET for listening for 
	// incoming connection requests
	if ((g_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		log("socket function failed with error: %u\n", WSAGetLastError());

		WSACleanup();

		return 1;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(nPort);

	// Bind the socket.
	if (bind(g_listen, (SOCKADDR *)&service, sizeof(service)) == SOCKET_ERROR)
	{
		log("bind failed with error %u\n", WSAGetLastError());
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	if (listen(g_listen, 1) == SOCKET_ERROR)
	{
		log("listen failed with error %u\n", WSAGetLastError());
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	g_hAbortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_hAbortEvent == NULL)
	{
		log("CreateEvent failed with error %u\n", GetLastError());
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	g_hThreadProc = CreateThread(0, 0, SocketThread, NULL, 0, NULL);
	if (g_hThreadProc == INVALID_HANDLE_VALUE)
	{
		log("CreateThread failed with error %u\n", GetLastError());
		CloseHandle(g_hAbortEvent);
		g_hAbortEvent = NULL;
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	return 0;
}

extern "C" long WINAPI SockServer_Uninitialize(void)
{
	log("SockServer_Uninitialize\n");

	if (g_hThreadProc != INVALID_HANDLE_VALUE)
	{
		SetEvent(g_hAbortEvent);
		WaitForSingleObject(g_hThreadProc, INFINITE);

		CloseHandle(g_hThreadProc);
		g_hThreadProc = INVALID_HANDLE_VALUE;
		CloseHandle(g_hAbortEvent);
		g_hAbortEvent = NULL;
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		WSACleanup();
	}

	return 0;
}
