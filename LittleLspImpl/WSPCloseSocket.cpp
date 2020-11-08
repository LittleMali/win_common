#include "stdafx.h"

int WSPAPI WSPCloseSocket(
    SOCKET s,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPCloseSocket(s, lpErrno);
}