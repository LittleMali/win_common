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

void CALLBACK SendCompletionROUTINE(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags)
{
    DBGLOGW(L"send completion routine, dwErr=%d, transferred bytes=%d", dwError, cbTransferred);
}

void CALLBACK RecvCompletionROUTINE(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags)
{
    // MyOverlapped* pMyOp = (MyOverlapped*)lpOverlapped;
    // 要扩展op结构保存上下文信息，否则，虽然recv成功了，但是在这里我们拿不到回包内容，也不知道这对应哪一次发包。
    DBGLOGW(L"recv completion routine, dwErr=%d, transferred bytes=%d", dwError, cbTransferred);
    if (lpOverlapped)
    {
        DBGLOGW(L"recv completion routine, lpOverlapped->Internal=%d, lpOverlapped->InternalHigh=%d, lpOverlapped->Offset=%d, lpOverlapped->OffsetHigh=%d", 
            lpOverlapped->Internal, lpOverlapped->InternalHigh,
            lpOverlapped->Offset, lpOverlapped->OffsetHigh);
    }
}

bool UdpOverlappedCompletionRoutineTest()
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

    HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (hEvent == NULL)
    {
        closesocket(s);
        return false;
    }

    DWORD dwLastErr = 0;

    for (int i = 0; i < 100; ++i)
    {
        // start sendto
        DBGLOGW(L"start sendto, i=%d", i);
        char szSendBuf[BUF_LEN] = { 0 };
        sprintf_s(szSendBuf, BUF_LEN, "udp overlapped completion routine test, udp overlapped completion routine test, udp overlapped completion routine test, i=%d", i);

        WSABUF wsaSendBuf;
        wsaSendBuf.buf = szSendBuf;
        wsaSendBuf.len = strlen(szSendBuf);

        DWORD dwSendCnt = 0;
        nErr = WSASendTo(s, &wsaSendBuf, 1,
            &dwSendCnt, 0, (sockaddr*)&addrSrv, sizeof(addrSrv),
            &op, SendCompletionROUTINE);
        if (nErr == SOCKET_ERROR) {
            dwLastErr = WSAGetLastError();
            if (dwLastErr != WSA_IO_PENDING) {
                DBGLOGW(L"sendto error, ret=%d, last err=%d", nErr, dwLastErr);
                CloseHandle(hEvent);
                closesocket(s);
                return false;
            }
            DBGLOGW(L"sendto is waiting for io oper");
        }
        else {
            DBGLOGW(L"sendto directly finished, all data has been sent, dwSendCnt=%d", dwSendCnt);
        }

        // wait sendto finish
        nErr = WaitForMultipleObjectsEx(1, &hEvent, TRUE, INFINITE, TRUE);  // ex系列，set alertable
        if (nErr == WAIT_FAILED) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"waitfor sendto error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            CloseHandle(hEvent);
            return false;
        }
        else if (nErr == WAIT_OBJECT_0) {
            DBGLOGW(L"waitfor sendto suc");
        }
        else if (nErr == WAIT_IO_COMPLETION)
        {
            DBGLOGW(L"ioc ompletion finished");
        }
        else {
            // we wait infinite, this could not happend
        }

        // we can recvfrom now
        sockaddr_in addrFrom = { 0 };
        int nAddrFromLen = sizeof(addrFrom);

        ZeroMemory(&op, sizeof(op));

        char szRecvBuf[BUF_LEN] = { 0 };
        WSABUF wsaRecvBuf;
        wsaRecvBuf.buf = szRecvBuf;
        wsaRecvBuf.len = sizeof(szRecvBuf);
        DWORD dwRecvLen = 0;
        DWORD dwFlags = 0;
        nErr = WSARecvFrom(s, &wsaRecvBuf, 1,
            &dwRecvLen, &dwFlags, (sockaddr*)&addrFrom, &nAddrFromLen,
            &op, RecvCompletionROUTINE);

        if (nErr == SOCKET_ERROR) {
            dwLastErr = WSAGetLastError();
            if (dwLastErr != WSA_IO_PENDING) {
                DBGLOGW(L"recvfrom error, ret=%d, last err=%d", nErr, dwLastErr);
                closesocket(s);
                CloseHandle(hEvent);
                return false;
            }
            DBGLOGW(L"recvfrom is waiting for io oper");
            DBGLOGW(L"check event singled: %d", IsEvnetSingled(hEvent));
        }
        else {
            //SetEvent(hEvent);
            DBGLOGW(L"recvfrom directly finished, all data has been recved, dwRecvLen=%d", dwRecvLen);
        }

        // waitfor recvfrom finished
        nErr = WaitForMultipleObjectsEx(1, &hEvent, TRUE, INFINITE, TRUE);
        DBGLOGW(L"check event singled: %d", IsEvnetSingled(hEvent));

        if (nErr == WAIT_FAILED) {
            dwLastErr = WSAGetLastError();
            DBGLOGW(L"waitfor recvfrom error, ret=%d, last err=%d", nErr, dwLastErr);
            closesocket(s);
            CloseHandle(hEvent);
            return false;
        }
        else if (nErr == WAIT_OBJECT_0) {
            DBGLOGW(L"waitfor recvfrom suc");
        }
        else if (nErr == WAIT_IO_COMPLETION)
        {
            DBGLOGW(L"ioc ompletion finished");
        }
        else {
            // we wait infinite, this could not happend
        }
    }

    // clear
    closesocket(s);
    CloseHandle(hEvent);

    return true;
}
