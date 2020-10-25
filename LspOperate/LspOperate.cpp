#include "stdafx.h"
#include "LspOperate.h"
#include "..\CommonUtils\Log.h"
#include <combaseapi.h>
#include "..\CommonUtils\StringUtils.h"
#include "..\CommonUtils\SystemUtils.h"
#include <shlwapi.h>
#include <rpcdce.h>
#include <sporder.h>

CLspOperate::CLspOperate()
{

}

void CLspOperate::EnumLspInfo()
{
    std::vector<WSAPROTOCOL_INFOW> vecProtocols = GetProtocolsByRegistry();

    for each (auto elem in vecProtocols)
    {
        DBGLOGW(L"\n\n============================================");
        DBGLOGW(L"szProtocol = %s", elem.szProtocol);
        DBGLOGW(L"dwCatalogEntryId = %d", elem.dwCatalogEntryId);

        TCHAR szGuidString[40] = { 0 };
        StringFromGUID2(elem.ProviderId, (LPOLESTR)&szGuidString, 39);
        DBGLOGW(L"ProviderId = %s", szGuidString);

        DBGLOGW(L"ProtocolChain.ChainLen = %d", elem.ProtocolChain.ChainLen);
        if (elem.ProtocolChain.ChainLen == 1)
        {
            DBGLOGW(L"ProtocolChain.ChainEntries = %d", elem.ProtocolChain.ChainEntries[0]);
        }
        else if (elem.ProtocolChain.ChainLen > 1)
        {
            DBGLOGW(L"ProtocolChain.ChainEntries = %d %d", elem.ProtocolChain.ChainEntries[0], elem.ProtocolChain.ChainEntries[1]);
        }
    }

    //for_each(vecProtocols.begin(), vecProtocols.end(), 
    //    [=](WSAPROTOCOL_INFOW elem)
    //    {
    //        DBGLOGW(L"ProtocolChain.ChainLen = %d", elem.ProtocolChain.ChainLen);
    //        DBGLOGW(L"dwCatalogEntryId = %d", elem.dwCatalogEntryId);
    //        DBGLOGW(L"szProtocol = %s", elem.szProtocol);

    //        TCHAR szGuidString[40] = { 0 };
    //        StringFromGUID2(elem.ProviderId, (LPOLESTR)&szGuidString, 39);
    //        DBGLOGW(L"ProviderId = %s", szGuidString);
    //    }
    //);
}

std::vector<WSAPROTOCOL_INFOW> CLspOperate::GetProtocols()
{
    auto protocols = GetProtocolsByApi();
    if (protocols.empty())
    {
        protocols = GetProtocolsByRegistry();
    }

    EnumLspInfo();
    return protocols;
}

BOOL CLspOperate::IsLspExist(const std::wstring& strLspName)
{
    auto protocols = GetProtocols();
    BOOL bFind = FALSE;
    for (auto item : protocols)
    {
        std::wstring strName = item.szProtocol;
        if (strName.find(strLspName) != std::wstring::npos)
        {
            bFind = TRUE;
            break;
        }
    }
    return bFind;
}

BOOL CLspOperate::InstallLsp()
{
    // step1: 判断lsp dll是否存在
    wchar_t szLspDllPath[MAX_PATH + 2] = {0};
    ::GetModuleFileName(nullptr, szLspDllPath, MAX_PATH);
    ::PathAppendW(szLspDllPath, L"..\\");
    ::PathAppendW(szLspDllPath, LSP_MODULE);
    if (!::PathFileExists(szLspDllPath))
    {
        DBGLOGW(L"miss lsp module: %s", szLspDllPath);
        return FALSE;
    }

    // step2：找到所有类型的基础协议（base protocols）
    BOOL bFindUdp = FALSE, bFindTcp = FALSE, bFindRaw = FALSE;
    BOOL bFindUdpV6 = FALSE, bFindTcpV6 = FALSE, bFindRawV6 = FALSE;
    std::vector<WSAPROTOCOL_INFOW> vecBaseProtocols;
    
    auto protocols = GetProtocols();
    for (auto item : protocols)
    {
        if (AF_INET == item.iAddressFamily)
        {
            if (!bFindUdp && item.iProtocol == IPPROTO_UDP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindUdp = TRUE;
                continue;
            }

            if (!bFindTcp && item.iProtocol == IPPROTO_TCP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindTcp = TRUE;
                continue;
            }

            if (!bFindRaw && item.iProtocol == IPPROTO_IP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindRaw = TRUE;
                continue;
            }
        }
        else if (AF_INET6 == item.iAddressFamily)
        {
            if (!bFindUdpV6 && item.iProtocol == IPPROTO_UDP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindUdpV6 = TRUE;
                continue;
            }

            if (!bFindTcpV6 && item.iProtocol == IPPROTO_TCP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindTcpV6 = TRUE;
                continue;
            }

            if (!bFindRawV6 && item.iProtocol == IPPROTO_IP && item.ProtocolChain.ChainLen == BASE_PROTOCOL)
            {
                vecBaseProtocols.push_back(item);
                bFindRawV6 = TRUE;
                continue;
            }
        }
    }

    // 清空 XP1_IFS_HANDLES 标记
    for(auto &item : vecBaseProtocols)
    {
        item.dwServiceFlags1 = item.dwServiceFlags1 & (~XP1_IFS_HANDLES);
    }

    if (vecBaseProtocols.empty())
    {
        DBGLOGW(L"find base protocol failed");
        return FALSE;
    }

    // step3: 安装我们的分层协议
    // 随便找个下层协议的结构复制过来
    WSAPROTOCOL_INFOW layeredProtocol;
    layeredProtocol = vecBaseProtocols[0];

    // 修改协议名称等
    wcsncpy_s(layeredProtocol.szProtocol, WSAPROTOCOL_LEN, LSP_NAME, _TRUNCATE);
    layeredProtocol.ProtocolChain.ChainLen = LAYERED_PROTOCOL;
    layeredProtocol.dwProviderFlags |= PFL_HIDDEN;

    // 安装分层协议
    int nError = 0;
    GUID layeredProtocolGuid = LSP_GUID;
    int nRet = ::WSCInstallProvider(&layeredProtocolGuid, szLspDllPath, &layeredProtocol, 1, &nError);
    if (nRet == SOCKET_ERROR)
    {
        DBGLOGW(L"install lsp layered protocol failed, ret=%d, error=%d", nRet, nError);
        return FALSE;
    }

    // step4：安装协议链
    // 先找到我们刚刚安装的分层协议
    protocols = GetProtocols();
    if (protocols.empty())
    {
        return FALSE;
    }

    DWORD dwLayeredProtocolCatalogEnterId = 0;
    for (auto item : protocols)
    {
        if (0 == memcmp(&item.ProviderId, &layeredProtocolGuid, sizeof(GUID)))
        {
            dwLayeredProtocolCatalogEnterId = item.dwCatalogEntryId;
            break;
        }
    }

    if (dwLayeredProtocolCatalogEnterId == 0)
    {
        DBGLOGW(L"find installed layered protocol failed.");
        return FALSE;
    }

    // 生成我们的协议链
    // 我们之前找到的全部是base协议，因此，可以直接ProtocolChain.ChainLen=2。
    // 如果我们要实现多层lsp顺序调用，那么就不能这么写。
    for (auto &item : vecBaseProtocols)
    {
        wchar_t szName[WSAPROTOCOL_LEN + 1] = { 0 };
        _snwprintf_s(szName, WSAPROTOCOL_LEN, WSAPROTOCOL_LEN, L"%s over %s", LSP_NAME, item.szProtocol);
        wcsncpy_s(item.szProtocol, WSAPROTOCOL_LEN, szName, _TRUNCATE);

        item.ProtocolChain.ChainEntries[1] = item.dwCatalogEntryId;
        item.ProtocolChain.ChainEntries[0] = dwLayeredProtocolCatalogEnterId;
        item.ProtocolChain.ChainLen = 2;
    }

    // 生成协议链guid
    GUID protocolChianGuid;
    if (RPC_S_OK != ::UuidCreate(&protocolChianGuid))
    {
        DWORD dwLastErr = ::GetLastError();
        DBGLOGW(L"create guid failed, last err=%d", dwLastErr);
        return FALSE;
    }

    nError = 0;
    nRet = ::WSCInstallProvider(&protocolChianGuid, szLspDllPath, &vecBaseProtocols[0], vecBaseProtocols.size(), &nError);
    if (nError == SOCKET_ERROR)
    {
        DBGLOGW(L"install lsp chain failed, ret=%d, error=%d", nRet, nError);
        return FALSE;
    }

    // step5: 重排winsock目录，将我们的协议链前置
    protocols = GetProtocols();
    if (protocols.empty())
    {
        return FALSE;
    }

    // 添加我们的协议链
    std::vector<DWORD> vecEnteryIds;
    for (auto item : protocols)
    {
        if (item.ProtocolChain.ChainLen > 1 && item.ProtocolChain.ChainEntries[0] == dwLayeredProtocolCatalogEnterId)
        {
            vecEnteryIds.push_back(item.dwCatalogEntryId);
        }
    }

    // 添加其他协议
    for (auto item : protocols)
    {
        if (item.ProtocolChain.ChainLen <= 1 || 
            item.ProtocolChain.ChainEntries[0] != dwLayeredProtocolCatalogEnterId)
        {
            vecEnteryIds.push_back(item.dwCatalogEntryId);
        }
    }

    if (vecEnteryIds.empty())
    {
        return FALSE;
    }

    nRet = ::WSCWriteProviderOrder(&vecEnteryIds[0], vecEnteryIds.size());
    if (nRet != ERROR_SUCCESS)
    {
        DBGLOGW(L"write provider order failed, ret=%d", nRet);
        return FALSE;
    }

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
