#include "stdafx.h"
#include "WS2spi.h"

extern WSPPROC_TABLE g_nextProcTable;
extern WSPPROC_TABLE g_detourProcTable;
extern BOOL g_bCallFunctionDll;

SOCKET WSPAPI WSPSocket(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPSocket(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
}
SOCKET WSPAPI WSPSocketDetour(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags,
    LPINT lpErrno
)
{
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPSocket(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
    }
    else
    {
        return WSPSocket(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
    }
}

int WSPAPI WSPCloseSocket(
    SOCKET s,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPCloseSocket(s, lpErrno);
}

int WSPAPI WSPCloseSocketDetour(
    SOCKET s,
    LPINT lpErrno
)
{
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPCloseSocket(s, lpErrno);
    }
    else
    {
        return WSPCloseSocket(s, lpErrno);
    }
}

int WSPAPI WSPShutdown(
    SOCKET s,
    int how,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPShutdown(s, how, lpErrno);
}

int WSPAPI WSPShutdownDetour(
    SOCKET s,
    int how,
    LPINT lpErrno
)
{
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPShutdown(s, how, lpErrno);
    }
    else
    {
        return WSPShutdown(s, how, lpErrno);
    }
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
    return g_nextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
        , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSendToDetour(
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
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
            , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
    else
    {
        return WSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
            , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
}

int
WSPAPI
WSPRecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}
int WSPAPI WSPRecvFromDetour(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
)
{
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
    else
    {
        return WSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }
}

int WSPAPI WSPConnect(
    _In_ SOCKET s,
    _In_reads_bytes_(namelen) const struct sockaddr FAR * name,
    _In_ int namelen,
    _In_opt_ LPWSABUF lpCallerData,
    _Out_opt_ LPWSABUF lpCalleeData,
    _In_opt_ LPQOS lpSQOS,
    _In_opt_ LPQOS lpGQOS,
    _Out_ LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}

int WSPAPI WSPConnectDetour(
    _In_ SOCKET s,
    _In_reads_bytes_(namelen) const struct sockaddr FAR * name,
    _In_ int namelen,
    _In_opt_ LPWSABUF lpCallerData,
    _Out_opt_ LPWSABUF lpCalleeData,
    _In_opt_ LPQOS lpSQOS,
    _In_opt_ LPQOS lpGQOS,
    _Out_ LPINT lpErrno
)
{
    if (g_bCallFunctionDll)
    {
        return g_detourProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
    }
    else
    {
        return WSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
    }
}