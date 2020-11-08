#include "stdafx.h"

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