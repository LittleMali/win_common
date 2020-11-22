#include "stdafx.h"

enum class SocketState {
    SocketNone = 0,
    SocketToRecv = 1,
};

#pragma warning (push)
#pragma warning (disable:4996)

bool UdpEventSelectNonBlockTest()
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
    {
        return false;
    }

    int nErr = 0;
    DWORD dwMode = 1;
    nErr = ioctlsocket(s, FIONBIO, &dwMode);
    if (nErr == SOCKET_ERROR)
    {
        return false;
    }

    WSAEVENT hEvent = ::WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        return false;
    }

    nErr = WSAEventSelect(s, hEvent, FD_READ | FD_WRITE | FD_CLOSE);
    if (nErr == SOCKET_ERROR)
    {
        return false;
    }

    sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(UDP_SRV_PORT);
    inet_pton(AF_INET, UDP_SRV_ADDR, &srvAddr.sin_addr.s_addr);

    int nSendCnt = 0;
    SocketState enSockState = SocketState::SocketNone;

    char szSendBuf[MAX_PATH] = { 0 };
    _snprintf_s(szSendBuf, MAX_PATH, "udp event select non block, send i=%d", nSendCnt++);

    for (int i = 0; i < 1000; ++i)
    {
        nErr = sendto(s, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&srvAddr, sizeof(srvAddr));
    }
    
    
    enSockState = SocketState::SocketToRecv;

    while (true)
    {
        // 这里可以等待多个hEvent，每个hEvent对应一个socket
        DWORD dwWait = WSAWaitForMultipleEvents(1, &hEvent, TRUE, WSA_INFINITE, FALSE);
        if (dwWait == WSA_WAIT_FAILED)
        {
            DBGLOGW(L"wait event failed");
            break;
        }
        else if (dwWait == WSA_WAIT_EVENT_0)
        {
            WSANETWORKEVENTS networkEvents;
            // enum的时候有传入s，也就意味着可以遍历处理多个socket请求。
            int nEnumError = WSAEnumNetworkEvents(s, hEvent, &networkEvents);
            if (SOCKET_ERROR == nEnumError)
            {
                DBGLOGW(L"enum network events failed");
                break;
            }

            if (FD_CLOSE & networkEvents.lNetworkEvents)
            {
                if (networkEvents.iErrorCode[FD_CLOSE_BIT] != ERROR_SUCCESS)
                {
                    DBGLOGW(L"socket CLOSED, but has errors");
                    break;
                }
            }


            if (FD_WRITE & networkEvents.lNetworkEvents)
            {
                if (networkEvents.iErrorCode[FD_WRITE_BIT] != ERROR_SUCCESS)
                {
                    DBGLOGW(L"socket can read, but has errors");
                    break;
                }

                enSockState = SocketState::SocketToRecv;
            }

            if (FD_READ & networkEvents.lNetworkEvents)
            {
                if (networkEvents.iErrorCode[FD_READ_BIT] != ERROR_SUCCESS)
                {
                    DBGLOGW(L"socket can read, but has errors");
                    break;
                }

                if (enSockState != SocketState::SocketToRecv)
                {
                    DBGLOGW(L"socket state is wrong");
                    break;
                }

                char szRecvBuf[MAX_PATH] = { 0 };
                sockaddr_in fromAddr;
                int fromLen = sizeof(fromAddr);
                nErr = recvfrom(s, szRecvBuf, sizeof(szRecvBuf), 0, (sockaddr*)&fromAddr, &fromLen);
                if (nErr == SOCKET_ERROR)
                {
                    DBGLOGW(L"recvfrom error: %d", ::WSAGetLastError());
                    break;
                }

                DBGLOGA("recv from client: %s", szRecvBuf);

                if (nSendCnt == 100)
                {
                    DBGLOGA("send test is finished");
                    break;
                }

                // send again
                char szSendBuf[MAX_PATH] = { 0 };
                _snprintf_s(szSendBuf, MAX_PATH, "udp event select non block, send i = %d", nSendCnt++);
                nErr = sendto(s, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&srvAddr, sizeof(srvAddr));

                for (int i = 0; i < 1000; ++i)
                {
                    nErr = sendto(s, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&srvAddr, sizeof(srvAddr));
                }

                if (nErr == SOCKET_ERROR)
                {
                    DBGLOGW(L"sendto failed: %d", ::WSAGetLastError());
                    break;
                }
            }
        }
        else 
        {
            break;
        }
    }

    WSACloseEvent(hEvent);
    closesocket(s);

    return nSendCnt != 100;
}

#pragma warning (pop)