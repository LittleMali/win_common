#pragma once

struct UdpIocpOverlapped : public OVERLAPPED
{
    enum class CompKey
    {
        ToNone = 0,
        ToRecv = 1,
        ToSend = 2,
        ToClose = 3,
    };
    UdpIocpOverlapped()
    {
        memset(this, 0, sizeof(UdpIocpOverlapped));
        wsaSendBuf.buf = szSendBuf;
        wsaRecvBuf.buf = szRecvBuf;
    }
    
    ~UdpIocpOverlapped() 
    {
        DBGLOGW(L"UdpIocpOverlapped de constructor");
    }

    void Init()
    {
        memset(this, 0, sizeof(UdpIocpOverlapped));
        wsaSendBuf.buf = szSendBuf;
        wsaRecvBuf.buf = szRecvBuf;
    }

    void InitOverlapped()
    {
        memset(this, 0, sizeof(OVERLAPPED));
    }

    void WriteSendBuf(char* pszWrite, int nIndex)
    {
        _snprintf_s(szSendBuf, BUF_LEN, _TRUNCATE, "%s, i=%d", pszWrite, nIndex);
        wsaSendBuf.len = strlen(szSendBuf);
    }
    void ClearSendBuf()
    {
        memset(szSendBuf, 0, sizeof(szSendBuf));
        wsaSendBuf.len = 0;
    }

    char* ReadRecvBuf()
    {
        return wsaRecvBuf.buf;
    }
    void ClearRecvBuf()
    {
        memset(szRecvBuf, 0, sizeof(szRecvBuf));
        wsaRecvBuf.len = BUF_LEN;
    }

    char szSendBuf[BUF_LEN];
    char szRecvBuf[BUF_LEN];
    WSABUF wsaSendBuf;
    WSABUF wsaRecvBuf;
    CompKey enCompKey;

    SOCKET sock;
    sockaddr_in addrSrv;
};

bool UdpIOCPTest();