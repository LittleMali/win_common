#include "stdafx.h"

int WSPAPI WSPShutdown(
    SOCKET s,
    int how,
    LPINT lpErrno
)
{
    return g_nextProcTable.lpWSPShutdown(s, how, lpErrno);
}