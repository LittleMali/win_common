// LittleLspImpl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <WS2spi.h>
#include "WSPFuncDefine.h"

WSPPROC_TABLE g_nextProcTable;

BOOL IsHookProcess()
{
    return TRUE;
}

BOOL WINAPI WSPHook(WSPPROC_TABLE *lpProcTable)
{
    if (!lpProcTable)
    {
        return FALSE;
    }

    if (!IsHookProcess())
    {
        return FALSE;
    }

    g_nextProcTable = *lpProcTable;

    lpProcTable->lpWSPSocket = WSPSocket;
    lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
    lpProcTable->lpWSPShutdown = WSPShutdown;
    lpProcTable->lpWSPSendTo = WSPSendTo;
    lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
    lpProcTable->lpWSPConnect = WSPConnect;

    return TRUE;
}