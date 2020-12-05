#include "stdafx.h"
#include "UdpOverlappedEvent.h"

static int IsEvnetSingled(HANDLE hEvnet)
{
    DWORD dwRet = WaitForSingleObject(hEvnet, 0);
    if (dwRet == WAIT_OBJECT_0) {
        return 1;
    }
    else if (dwRet == WAIT_TIMEOUT) {
        return 0;
    }
    else {
        return -1;
    }
}

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
        closesocket(s);
        return false;
    }

    sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    inet_pton(AF_INET, UDP_SRV_ADDR, &addrSrv.sin_addr.s_addr);
    addrSrv.sin_port = htons(UDP_SRV_PORT);

    WSAOVERLAPPED op;
    ZeroMemory(&op, sizeof(op));

    op.hEvent = WSACreateEvent();
    if (op.hEvent == WSA_INVALID_EVENT) {
        closesocket(s);
        return false;
    }

    DBGLOGW(L"check event singled: %d", IsEvnetSingled(op.hEvent));

    DWORD dwLastErr = 0;
    
    for (int i = 0; i < 100; ++i)
    {
        // start sendto
        DBGLOGW(L"start sendto, i=%d", i);
        char szSendBuf[BUF_LEN] = { 0 };
        sprintf_s(szSendBuf, BUF_LEN, "udp overlapped event test, udp overlapped event test, udp overlapped event test, i=%d", i);

        WSABUF wsaSendBuf;
        wsaSendBuf.buf = szSendBuf;
        wsaSendBuf.len = strlen(szSendBuf);

        DWORD dwSendCnt = 0;
        nErr = WSASendTo(s, &wsaSendBuf, 1,
            &dwSendCnt, 0, (sockaddr*)&addrSrv, sizeof(addrSrv),
            &op, nullptr);
        if (nErr == SOCKET_ERROR) {
            dwLastErr = WSAGetLastError();
            if (dwLastErr != WSA_IO_PENDING) {
                DBGLOGW(L"sendto error, ret=%d, last err=%d", nErr, dwLastErr);
                closesocket(s);
                WSACloseEvent(op.hEvent);
                return false;
            }
            DBGLOGW(L"sendto is waiting for io oper");
            DBGLOGW(L"check event singled: %d", IsEvnetSingled(op.hEvent));
        }
        else if (nErr == 0) {
            WSASetEvent(op.hEvent);  // note
            DBGLOGW(L"sendto directly finished, all data has been sent, dwSendCnt=%d", dwSendCnt);
        }

        // wait sendto finish
        nErr = WSAWaitForMultipleEvents(1, &op.hEvent, TRUE, INFINITE, FALSE);
        if (nErr == WSA_WAIT_FAILED) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"waitfor sendto error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            WSACloseEvent(op.hEvent);
            return false;
        }
        else if (nErr == WSA_WAIT_EVENT_0) {
            DBGLOGW(L"waitfor sendto suc");
        }
        else {
            // we wait infinite, this could not happend
        }

        // check sendto result
        DWORD dwDataTransfer = 0;
        DWORD dwFlags = 0;
        nErr = WSAGetOverlappedResult(s, &op, &dwDataTransfer, TRUE, &dwFlags);
        if (nErr == FALSE) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"get overlapped sendto result error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            WSACloseEvent(op.hEvent);
            return false;
        }
        else {
            DBGLOGW(L"data transferred bytes cnt=%d, sendto buf len=%d", dwDataTransfer, wsaSendBuf.len);
        }

        DBGLOGW(L"check event singled: %d", IsEvnetSingled(op.hEvent));
        WSAResetEvent(op.hEvent);

        // we can recvfrom now
        sockaddr_in addrFrom = { 0 };
        int nAddrFromLen = sizeof(addrFrom);

        char szRecvBuf[BUF_LEN] = { 0 };
        WSABUF wsaRecvBuf;
        wsaRecvBuf.buf = szRecvBuf;
        wsaRecvBuf.len = sizeof(szRecvBuf);
        DWORD dwRecvLen = 0;
        dwFlags = 0;
        nErr = WSARecvFrom(s, &wsaRecvBuf, 1,
            &dwRecvLen, &dwFlags, (sockaddr*)&addrFrom, &nAddrFromLen,
            &op, nullptr);

        if (nErr == SOCKET_ERROR) {
            dwLastErr = WSAGetLastError();
            if (dwLastErr != WSA_IO_PENDING) {
                DBGLOGW(L"recvfrom error, ret=%d, last err=%d", nErr, dwLastErr);
                closesocket(s);
                WSACloseEvent(op.hEvent);
                return false;
            }
            DBGLOGW(L"recvfrom is waiting for io oper");
            DBGLOGW(L"check event singled: %d", IsEvnetSingled(op.hEvent));
            DBGLOGW(L"check event singled: %d", IsEvnetSingled(op.hEvent));
        }
        else if (nErr == 0) {
            WSASetEvent(op.hEvent);
            DBGLOGW(L"recvfrom directly finished, all data has been recved, dwRecvLen=%d", dwRecvLen);
        }

        // waitfor recvfrom finished
        nErr = WSAWaitForMultipleEvents(1, &op.hEvent, TRUE, INFINITE, FALSE);
        if (nErr == WSA_WAIT_FAILED) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"waitfor recvfrom error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            WSACloseEvent(op.hEvent);
            return false;
        }
        else if (nErr == WSA_WAIT_EVENT_0) {
            DBGLOGW(L"waitfor recvfrom suc");
        }
        else {
            // we wait infinite, this could not happend
        }

        // get recv result
        dwDataTransfer = 0;
        dwFlags = 0;
        nErr = WSAGetOverlappedResult(s, &op, &dwDataTransfer, TRUE, &dwFlags);
        if (nErr == FALSE) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"get overlapped recvfrom result error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            WSACloseEvent(op.hEvent);
            return false;
        }
        else {
            DBGLOGW(L"data transferred bytes cnt=%d", dwDataTransfer);
        }

        WSAResetEvent(op.hEvent);
    }
    

    // clear
    closesocket(s);
    WSACloseEvent(op.hEvent);

    return true;
}
