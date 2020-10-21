#include "stdafx.h"
#include "LspOperate.h"
#include "..\CommonUtils\Log.h"
#include <combaseapi.h>
#include "..\CommonUtils\StringUtils.h"

#pragma comment (lib, "Ws2_32.lib")

CLspOperate::CLspOperate()
{

}

void CLspOperate::EnumLspInfo()
{
    std::vector<WSAPROTOCOL_INFOW> vecProtocols = GetProtocolsByRegistry();

    for each (auto elem in vecProtocols)
    {
        DBGLOGW(L"ProtocolChain.ChainLen = %d", elem.ProtocolChain.ChainLen);
        DBGLOGW(L"dwCatalogEntryId = %d", elem.dwCatalogEntryId);
        DBGLOGW(L"szProtocol = %s", elem.szProtocol);

        TCHAR szGuidString[40] = { 0 };
        StringFromGUID2(elem.ProviderId, (LPOLESTR)&szGuidString, 39);
        DBGLOGW(L"ProviderId = %s", szGuidString);
    }

    for_each(vecProtocols.begin(), vecProtocols.end(), 
        [=](WSAPROTOCOL_INFOW elem)
        {
            DBGLOGW(L"ProtocolChain.ChainLen = %d", elem.ProtocolChain.ChainLen);
            DBGLOGW(L"dwCatalogEntryId = %d", elem.dwCatalogEntryId);
            DBGLOGW(L"szProtocol = %s", elem.szProtocol);

            TCHAR szGuidString[40] = { 0 };
            StringFromGUID2(elem.ProviderId, (LPOLESTR)&szGuidString, 39);
            DBGLOGW(L"ProviderId = %s", szGuidString);
        }
    );
}

std::vector<WSAPROTOCOL_INFOW> CLspOperate::GetProtocols()
{
    auto protocols = GetProtocolsByApi();
    if (protocols.empty())
    {
        protocols = GetProtocolsByRegistry();
    }

    return protocols;
}

BOOL CLspOperate::IsLspExist(const std::wstring& strLspName)
{

    return TRUE;
}

std::vector<WSAPROTOCOL_INFOW> CLspOperate::GetProtocolsByApi()
{
    std::vector<WSAPROTOCOL_INFOW> vecProtocols;
    DWORD dwBufferSize = 0;
    int nError = 0;
    int nRet = 0;

    TCHAR szGuidString[40] = { 0 };
    std::shared_ptr<WSAPROTOCOL_INFOW> spProtocols;

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

    for (int i = 0; i < nRet; ++i)
    {
        vecProtocols.push_back(spProtocols.get()[i]);
    }

Exit0:
    return vecProtocols;
}

std::vector<WSAPROTOCOL_INFOW> CLspOperate::GetProtocolsByRegistry()
{
    std::vector<WSAPROTOCOL_INFOW> vecProtocols;

    HKEY hKey = NULL;
    LPCWSTR lpLspSub = nullptr;
    if (SystemUtils::IsWow64())
    {
        lpLspSub = L"SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries";
    }
    else
    {
        lpLspSub = L"SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries64";
    }

    LSTATUS lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, lpLspSub, 0, KEY_READ, &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        return vecProtocols;
    }

    int nIndex = 0;
    while (TRUE)
    {
        wchar_t szSubName[MAX_PATH] = { 0 };
        DWORD dwKeyNameCch = _countof(szSubName);
        lRet = RegEnumKeyExW(hKey, nIndex, szSubName, &dwKeyNameCch, nullptr, nullptr, nullptr, nullptr);
        if (ERROR_SUCCESS != lRet)
        {
            break;
        }
        ++nIndex;

        if (ERROR_NO_MORE_ITEMS == lRet)
        {
            break;
        }

        wchar_t szSubPath[MAX_PATH + 2] = { 0 };
        _snwprintf_s(szSubPath, MAX_PATH, MAX_PATH, L"%s\\%s", lpLspSub, szSubName);

        HKEY hSubKey = NULL;
        lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szSubPath, 0, KEY_READ, &hSubKey);
        if (lRet == ERROR_SUCCESS)
        {
            DWORD dwType = 0;
            char szProtocolItem[1024 + 2] = { 0 };
            DWORD dwSize = sizeof(szProtocolItem);
            lRet = RegQueryValueExW(hSubKey, L"PackedCatalogItem", nullptr, &dwType, (LPBYTE)szProtocolItem, &dwSize);
            if (lRet == ERROR_SUCCESS)
            {
                if (REG_BINARY == dwType)
                {
                    WSAPROTOCOL_INFOW protocol = {0};
                    memcpy_s(&protocol, sizeof(protocol), (void*)&szProtocolItem[MAX_PATH], sizeof(WSAPROTOCOL_INFOW));
                    
                    vecProtocols.push_back(protocol);
                }
            }

            RegCloseKey(hSubKey);
            hSubKey = NULL;
        }
    }

    RegCloseKey(hKey);
    hKey = NULL;

    return vecProtocols;
}
