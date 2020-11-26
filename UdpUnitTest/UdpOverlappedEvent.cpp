#include "stdafx.h"
#include "UdpOverlappedEvent.h"

bool UdpOverlappedEventTest()
{
    int i = 0;

    SOCKET s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET) {
        return false;
    }

    DWORD dwMode = 1;
    int nErr = ioctlsocket(s, FIONBIO, &dwMode);
    if (nErr == SOCKET_ERROR) {
        return false;
    }

    sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    inet_pton(AF_INET, UDP_SRV_ADDR, &addrSrv.sin_addr.s_addr);
    addrSrv.sin_port = htons(UDP_SRV_PORT);

    WSAOVERLAPPED op;
    ZeroMemory(&op, sizeof(op));

    op.hEvent = WSACreateEvent();
    if (op.hEvent = WSA_INVALID_EVENT) {
        return false;
    }

    char szSendBuf[MAX_PATH] = { 0 };
    sprintf_s(szSendBuf, MAX_PATH, "udp overlapped event test, i=%d", i++);

    WSABUF wsaSendBuf;
    wsaSendBuf.buf = szSendBuf;
    wsaSendBuf.len = strlen(szSendBuf);




    
    
    return true;
}
