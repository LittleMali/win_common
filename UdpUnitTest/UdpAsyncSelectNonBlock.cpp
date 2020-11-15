#include "stdafx.h"
#include "UdpAsyncSelectNonBlock.h"

#pragma warning (push)
#pragma warning (disable:4996)

CUdpAsyncSelectNonBlock::CUdpAsyncSelectNonBlock()
    : m_socketMsgWnd(this)
{
    m_socket = INVALID_SOCKET;
    memset(&m_srvAddr, 0, sizeof(m_srvAddr));
    m_bInitSuc = m_socketMsgWnd.Init();
}

CUdpAsyncSelectNonBlock::~CUdpAsyncSelectNonBlock()
{
    m_socketMsgWnd.UnInit();
    m_bInitSuc = false;
    m_socket = INVALID_SOCKET;
    memset(&m_srvAddr, 0, sizeof(m_srvAddr));
}

void CUdpAsyncSelectNonBlock::OnSocket(WPARAM wParam, LPARAM lParam)
{
    static int nReadCnt = 0, nWriteCnt = 0, nCloseCnt = 0;

    if (WSAGETSELECTERROR(lParam))
    {
        DBGLOGW(L"wm_socket is error: %d", WSAGETSELECTERROR(lParam));
        PostQuitMessage(0);
    }
    else
    {
        DWORD dwEvent = WSAGETSELECTEVENT(lParam);
        switch (dwEvent)
        {
        case FD_CONNECT:
        {
            DBGLOGW(L"FD_CONNECT");
            break;
        }

        case FD_READ:
        {
            DBGLOGW(L"FD_READ: nReadCnt=%d", nReadCnt++);

            // 有数据到了，可以接收。
            char szRecvBuf[MAX_PATH] = { 0 };
            sockaddr_in fromAddr = { 0 };
            int nFromLen = sizeof(fromAddr);
            int nRet = recvfrom(m_socket, szRecvBuf, sizeof(szRecvBuf), 0, (sockaddr*)&fromAddr, &nFromLen);
            DBGLOGA("recvfrom i=%d, ret len=%d, recv data=%s", nReadCnt, nRet, szRecvBuf);
            if (nRet == SOCKET_ERROR)
            {
                DBGLOGW(L"recvfrom failed, i=%d, ret=%d, last err=%d", nReadCnt, nRet, ::WSAGetLastError());
                PostQuitMessage(0);
                break;
            }

            // 数据接收完了，接着干嘛呢，这里我们继续发。
            // send again
            char szSendBuf[MAX_PATH] = { 0 };
            sprintf_s(szSendBuf, MAX_PATH, "udp async select non block test, i=%d", nWriteCnt++);
            nRet = sendto(m_socket, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&m_srvAddr, sizeof(m_srvAddr));
            DBGLOGW(L"sendto i=%d, buf len=%d, ret len=%d", nWriteCnt, strlen(szSendBuf), nRet);
            if (nRet == SOCKET_ERROR)
            {
                DBGLOGW(L"sendto failed, i=%d, ret=%d, last err=%d", nWriteCnt, nRet, ::WSAGetLastError());
                PostQuitMessage(0);
                break;
            }

            break;
        }

        case FD_WRITE:
        {
            DBGLOGW(L"FD_WRITE: nWriteCnt=%d", nWriteCnt);

            // ready to send.
            // WSAAsyncSelect的时候，如果有缓存就绪，那就会收到FD_WRITE，表示可以send了。
            char szSendBuf[MAX_PATH] = { 0 };
            sprintf_s(szSendBuf, MAX_PATH, "udp async select non block test, i=%d", nWriteCnt++);
            int nRet = sendto(m_socket, szSendBuf, strlen(szSendBuf), 0, (const sockaddr*)&m_srvAddr, sizeof(m_srvAddr));
            DBGLOGW(L"sendto i=%d, buf len=%d, ret len=%d", nWriteCnt, strlen(szSendBuf), nRet);
            if (nRet == SOCKET_ERROR)
            {
                DBGLOGW(L"sendto failed, i=%d, ret=%d, last err=%d", nWriteCnt, nRet, ::WSAGetLastError());
                PostQuitMessage(0);
                break;
            }

            break;
        }

        case FD_CLOSE:
        {
            DBGLOGW(L"FD_CLOSE: nCloseCnt=%d", nCloseCnt++);
            break;
        }
        default:
        {
            DBGLOGW(L"unknown=%d", dwEvent);
            break;
        }

        }

        if (nWriteCnt > 100)
        {
            PostQuitMessage(0);
        }
    }
}

bool CUdpAsyncSelectNonBlock::Run()
{
    if (!m_bInitSuc)
    {
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET)
    {
        DBGLOGW(L"invalid socket");
        return false;
    }

    u_long uMode = 1;
    int nErr = ioctlsocket(m_socket, FIONBIO, &uMode);
    if (SOCKET_ERROR == nErr)
    {
        DBGLOGW(L"set block socket failed, nErr=%d, last err=%d", nErr, WSAGetLastError());
        closesocket(m_socket);
        return false;
    }

    m_srvAddr;
    m_srvAddr.sin_family = AF_INET;
    // srvAddr.sin_addr.s_addr = inet_addr(UDP_SRV_ADDR);
    inet_pton(AF_INET, UDP_SRV_ADDR, &m_srvAddr.sin_addr.s_addr);
    m_srvAddr.sin_port = htons(UDP_SRV_PORT);

    nErr = WSAAsyncSelect(m_socket, m_socketMsgWnd.m_hWnd,
        WM_SOCKET,
        FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

    DBGLOGW(L"async select for. nErr=%d, last err=%d", nErr, WSAGetLastError());
    if (nErr == SOCKET_ERROR)
    {
        closesocket(m_socket);
        return false;
    }

    while (true)
    {
        MSG msg;
        int ret = ::GetMessage(&msg, NULL, 0, 0);
        if (0 == ret || -1 == ret)
        {
            break;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    closesocket(m_socket);

    return true;
}

#pragma warning (pop)
