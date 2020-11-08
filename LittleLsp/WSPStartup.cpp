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

    // �ҵ��²�Ļ���Э�飬���������ʼ��WSPStartup
    // step0: �ϸ���˵��ChainEntries[1]��һ���ǻ���Э�飬����������һ���ֲ�Э�飬���Բ�������������д��
    //DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];

    // step1: �ҵ������Լ���λ�ã����ǵ���һ����� ����Э��
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

    // step2����ȡ�²����Э�����Ϣ���ҵ��²����Э��·��
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

    // step3: ���ػ���dll������ʼ��
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

    // ��ʼ������Э��ʱ��Ҫ�������Э��� WSAPROTOCOL_INFOW��
    // ����ǳ�ʼ���²�ķֲ�Э�飨layered��������Ҫ͸��lpProtocolInfo��
    LPWSAPROTOCOL_INFOW pBaseProtocolInfo = &baseProtocolInfo;
    int nRet = pfnBaseStartup(wVersionRequested, lpWSPData, pBaseProtocolInfo, UpcallTable, lpProcTable);
    if (ERROR_SUCCESS != nRet)
    {
        DBGLOGW(L"call base WSPStartup failed");
        return WSAEPROVIDERFAILEDINIT;
    }

    // �²�Ļ���Э���ʼ�����������ˡ�
    // ��������HOOK�����滻��
    // step1�������²�ĺ�����
    g_nextProcTable = *lpProcTable;

    // step2: �滻�²㺯����
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