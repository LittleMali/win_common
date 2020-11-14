#include "stdafx.h"
#include "LspUtils/LspUtils.h"

int WSPAPI WSPSendToImpl(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
)
{
    if (lpTo == nullptr || iTolen < sizeof(sockaddr_in) || lpBuffers == nullptr || dwBufferCount == 0)
    {
        DBGLOGW(L"invalid params");
        return g_nextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
            , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
    
    sockaddr_in addrNew = { 0 };
    addrNew = *(sockaddr_in*)lpTo;

    std::string strIp = LspUtils::lsp_inet_ntoa(addrNew.sin_addr.s_addr);
    USHORT uPort = addrNew.sin_port;
    return 0;
}

int WSPAPI WSPSendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
)
{
    __try
    {
        return WSPSendToImpl(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
            , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;
    }
}