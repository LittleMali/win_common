#include "stdafx.h"
#include <WS2tcpip.h>

bool UdpBlockTest()
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
    {
        DBGLOGW(L"invalid socket");
        return false;
    }

    u_long uMode = 0;
    int nErr = ioctlsocket(s, FIONBIO, &uMode);
    if (SOCKET_ERROR == nErr)
    {
        DBGLOGW(L"set block socket failed, nErr=%d, last err=%d", nErr, WSAGetLastError());
        closesocket(s);
        return false;
    }

    sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    // srvAddr.sin_addr.s_addr = inet_addr(UDP_SRV_ADDR);
    inet_pton(AF_INET, UDP_SRV_ADDR, &srvAddr.sin_addr.s_addr);
    srvAddr.sin_port = htons(UDP_SRV_PORT);

    

    int nSendFailCnt = 0, nRecvFailCnt = 0;
    for (int i = 0; i < 100; ++i)
    {
        char szSendBuf[MAX_PATH] = { 0 };
        sprintf_s(szSendBuf, MAX_PATH, "udp block test, i=%d", i);
        int nRet = sendto(s, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&srvAddr, sizeof(srvAddr));
        DBGLOGW(L"sendto i=%d, buf len=%d, ret len=%d", i, strlen(szSendBuf), nRet);
        if (nRet == SOCKET_ERROR)
        {
            DBGLOGW(L"sendto failed, i=%d, ret=%d, last err=%d", i, nRet, ::WSAGetLastError());
            nSendFailCnt++;
            continue;
        }

        char szRecvBuf[MAX_PATH] = { 0 };
        sockaddr_in fromAddr = { 0 };
        int nFromLen = sizeof(fromAddr);
        nRet = recvfrom(s, szRecvBuf, sizeof(szRecvBuf), 0, (sockaddr*)&fromAddr, &nFromLen);
        DBGLOGA("recvfrom i=%d, ret len=%d, recv data=%s", i, nRet, szRecvBuf);
        if (nRet == SOCKET_ERROR)
        {
            DBGLOGW(L"recvfrom failed, i=%d, ret=%d, last err=%d", i, nRet, ::WSAGetLastError());
            nRecvFailCnt++;
            continue;
        }
    }

    closesocket(s);
    return true;
}
