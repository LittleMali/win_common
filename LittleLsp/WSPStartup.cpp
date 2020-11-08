#include "stdafx.h"
#include <WS2spi.h>
#include "..\LspOperate\LspOperate.h"
#include "..\CommonUtils\Log.h"
#include "LspWrapper.h"

#pragma comment(lib, "LspOperate.lib")
#pragma comment(lib, "CommonUtils.lib")

extern HANDLE g_hModule;
WSPPROC_TABLE g_nextProcTable;
WSPPROC_TABLE g_detourProcTable;
BOOL g_bCallFunctionDll = FALSE;

int WSPAPI WSPStartup( WORD wVersionRequested,
    _In_ LPWSPDATA lpWSPData,
    _In_ LPWSAPROTOCOL_INFOW lpProtocolInfo,
    _In_ WSPUPCALLTABLE UpcallTable,
    _Out_ LPWSPPROC_TABLE lpProcTable)
{
    if (!lpProtocolInfo || lpProtocolInfo->ProtocolChain.ChainLen <= 1
        || !lpProcTable)
    {
        return WSAEPROVIDERFAILEDINIT;
    }

    // 找到下层的基础协议，并调用其初始化WSPStartup
    // step0: 严格来说，ChainEntries[1]不一定是基础协议，它可能是下一个分层协议，所以不能像下面这样写。
    //DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];

    // step1: 找到我们自己的位置，我们的下一层就是 基础协议
    wchar_t szDllPath[MAX_PATH] = { 0 };
    ::GetModuleFileNameW((HMODULE)g_hModule, szDllPath, MAX_PATH);

    CLspOperate lspOper;
    auto protocols = lspOper.GetProtocols();

    DWORD dwBaseEnterId = 0;
    for (int i = 0; i < lpProtocolInfo->ProtocolChain.ChainLen; ++i)
    {
        DWORD dwEnteryId = lpProtocolInfo->ProtocolChain.ChainEntries[i];
        for (auto item : protocols)
        {
            if (item.dwCatalogEntryId == dwEnteryId)
            {
                wchar_t szProviderDllPath[MAX_PATH] = { 0 };
                int nLen = _countof(szProviderDllPath);
                int nError = 0;
                if (SOCKET_ERROR == ::WSCGetProviderPath(&item.ProviderId, szProviderDllPath, &nLen, &nError))
                {
                    DBGLOGW(L"get provider path failed.");
                    return WSAEPROVIDERFAILEDINIT;
                }

                wchar_t szExpandProvederDllPath[MAX_PATH] = { 0 };
                if (0 == ::ExpandEnvironmentStringsW(szProviderDllPath, szExpandProvederDllPath, MAX_PATH))
                {
                    DBGLOGW(L"expand provider path failed.");
                    return WSAEPROVIDERFAILEDINIT;
                }

                if (0 == _wcsicmp(szDllPath, szProviderDllPath))
                {
                    if (i + 1 < lpProtocolInfo->ProtocolChain.ChainLen)
                    {
                        dwBaseEnterId = lpProtocolInfo->ProtocolChain.ChainEntries[i + 1];
                    }
                }

                break;
            }
        }

        if (dwBaseEnterId != 0)
        {
            break;
        }
    }

    if (dwBaseEnterId == 0)
    {
        DBGLOGW(L"can not find our own dll");
        return WSAEPROVIDERFAILEDINIT;
    }

    // step2：提取下层基础协议的信息，找到下层基础协议路径
    WSAPROTOCOL_INFOW baseProtocolInfo;
    for (auto item : protocols)
    {
        if (item.dwCatalogEntryId == dwBaseEnterId)
        {
            memcpy_s(&baseProtocolInfo, sizeof(baseProtocolInfo), &item, sizeof(item));
            break;
        }
    }

    wchar_t szBaseDllPath[MAX_PATH] = { 0 };
    int nBaseDllLen = MAX_PATH;
    int nBaseDllError = 0;
    if (SOCKET_ERROR == ::WSCGetProviderPath(&baseProtocolInfo.ProviderId, szBaseDllPath, &nBaseDllLen, &nBaseDllError))
    {
        DBGLOGW(L"get base dll path failed");
        return WSAEPROVIDERFAILEDINIT;
    }

    wchar_t szExpandBaseDllPath[MAX_PATH] = { 0 };
    if (0 == ::ExpandEnvironmentStringsW(szBaseDllPath, szExpandBaseDllPath, MAX_PATH))
    {
        return WSAEPROVIDERFAILEDINIT;
    }

    // step3: 加载基础dll，并初始化
    HMODULE hBaseDllModele = ::LoadLibrary(szExpandBaseDllPath);
    if (!hBaseDllModele)
    {
        DBGLOGW(L"load base protocol dll failed");
        return WSAEPROVIDERFAILEDINIT;
    }

    LPWSPSTARTUP pfnBaseStartup = (LPWSPSTARTUP)::GetProcAddress(hBaseDllModele, "WSPStartup");
    if (!pfnBaseStartup)
    {
        return WSAEPROVIDERFAILEDINIT;
    }

    // 初始化基础协议时，要传入基础协议的 WSAPROTOCOL_INFOW。
    // 如果是初始化下层的分层协议（layered），则需要透传lpProtocolInfo。
    LPWSAPROTOCOL_INFOW pBaseProtocolInfo = &baseProtocolInfo;
    int nRet = pfnBaseStartup(wVersionRequested, lpWSPData, pBaseProtocolInfo, UpcallTable, lpProcTable);
    if (ERROR_SUCCESS != nRet)
    {
        DBGLOGW(L"call base WSPStartup failed");
        return WSAEPROVIDERFAILEDINIT;
    }

    // 下层的基础协议初始化工作结束了。
    // 接下来，HOOK函数替换。
    // step1：保存下层的函数表
    g_nextProcTable = *lpProcTable;

    // step2: 替换下层函数表
    g_detourProcTable.lpWSPSocket = WSPSocket;
    g_detourProcTable.lpWSPCloseSocket = WSPCloseSocket;
    g_detourProcTable.lpWSPShutdown = WSPShutdown;
    g_detourProcTable.lpWSPSendTo = WSPSendTo;
    g_detourProcTable.lpWSPRecvFrom = WSPRecvFrom;
    g_detourProcTable.lpWSPConnect = WSPConnect;
    

    lpProcTable->lpWSPSocket = WSPSocketDetour;
    lpProcTable->lpWSPCloseSocket = WSPCloseSocketDetour;
    lpProcTable->lpWSPShutdown = WSPShutdownDetour;
    lpProcTable->lpWSPSendTo = WSPSendToDetour;
    lpProcTable->lpWSPRecvFrom = WSPRecvFromDetour;
    lpProcTable->lpWSPConnect = WSPConnectDetour;


    return 0;
}