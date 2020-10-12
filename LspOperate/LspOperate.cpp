#include "stdafx.h"
#include "LspOperate.h"
#include "..\CommonUtils\Log.h"
#include <combaseapi.h>

#pragma comment (lib, "Ws2_32.lib")

CLspOperate::CLspOperate()
{

}

void CLspOperate::EnumLspInfo()
{
    int nCnt = 0;
    std::shared_ptr<WSAPROTOCOL_INFOW> spProtocols = GetProtocols(&nCnt);

    if (!nCnt)
    {
        DBGLOGW(L"get protocols failed");
        return;
    }

    TCHAR szGuidString[40] = { 0 };

    for (int i = 0; i < nCnt; ++i)
    {
        DBGLOGW(L"ProtocolChain.ChainLen = %d", spProtocols.get()[i].ProtocolChain.ChainLen);
        DBGLOGW(L"dwCatalogEntryId = %d", spProtocols.get()[i].dwCatalogEntryId);
        DBGLOGW(L"szProtocol = %s", spProtocols.get()[i].szProtocol);
        StringFromGUID2(spProtocols.get()[i].ProviderId, (LPOLESTR)&szGuidString, 39);
        DBGLOGW(L"ProviderId = %s", szGuidString);
    }
}

BOOL CLspOperate::IsLspExist(const std::wstring& strLspName)
{
    return TRUE;
}

std::shared_ptr<WSAPROTOCOL_INFOW> CLspOperate::GetProtocols(int* pProtocolCount)
{
    std::shared_ptr<WSAPROTOCOL_INFOW> spProtocols;
    DWORD dwBufferSize = 0;
    int nError = 0;
    int nRet = 0;

    TCHAR szGuidString[40] = { 0 };

    nRet = WSCEnumProtocols(nullptr, nullptr, &dwBufferSize, &nError);
    if (nRet == SOCKET_ERROR && WSAENOBUFS == nError)
    {
        int nCnt = dwBufferSize / sizeof(WSAPROTOCOL_INFOW);
        spProtocols.reset(new WSAPROTOCOL_INFOW[nCnt], [](WSAPROTOCOL_INFOW* p) { delete[] p; });
        if (!spProtocols)
        {
            DBGLOGW(L"outof memory");
            goto Exit0;
        }
    }

    nRet = WSCEnumProtocols(nullptr, spProtocols.get(), &dwBufferSize, &nError);
    if (nRet == SOCKET_ERROR)
    {
        DBGLOGW(L"enum protocol failed, ret=%d, error=%d", nRet, nError);
        goto Exit0;
    }

    goto Exit1;

Exit0:
    if (pProtocolCount)
    {
        *pProtocolCount = 0;
    }
    return spProtocols;

Exit1:
    if (pProtocolCount)
    {
        *pProtocolCount = nRet;
    }
    
    return spProtocols;
}
