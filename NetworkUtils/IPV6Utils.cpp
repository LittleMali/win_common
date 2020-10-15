#include "stdafx.h"
#include "IPV6Utils.h"
#include "..\CommonUtils\ProcessUtils.h"


BOOL IPV6Utils::IPV6DisablePrivacyState()
{
    wchar_t szSysPath[MAX_PATH + 2] = { 0 };
    UINT uRet = ::GetSystemDirectoryW(szSysPath, MAX_PATH);
    if (uRet != 0)
    {
        ::PathAppend(szSysPath, L"netsh.exe");
    }
    else
    {
        wcscpy_s(szSysPath, MAX_PATH, L"netsh.exe");
    }
    
    const std::wstring strCmdline = L"interface ipv6 set privacy state=enable";
    BOOL bRet = ProcessUtils::RunExe(szSysPath, strCmdline);
    return bRet ? true : false;
}

BOOL IPV6Utils::IPV6EnablePrivacyState()
{
    wchar_t szSysPath[MAX_PATH + 2] = { 0 };
    UINT uRet = ::GetSystemDirectoryW(szSysPath, MAX_PATH);
    if (uRet != 0)
    {
        ::PathAppend(szSysPath, L"netsh.exe");
    }
    else
    {
        wcscpy_s(szSysPath, MAX_PATH, L"netsh.exe");
    }

    const std::wstring strCmdline = L"interface ipv6 set privacy state=disable";
    BOOL bRet = ProcessUtils::RunExe(szSysPath, strCmdline);
    return bRet;
}

DWORD IPV6Utils::GetIPV6Config()
{
    DWORD dwConfig = 0;

    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    LSTATUS lRet = ::SHGetValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\TCPIP6\\Parameters", L"DisabledComponents", &dwType, &dwConfig, &dwSize);

    if (lRet == ERROR_FILE_NOT_FOUND)
    {
        dwConfig = 0; // windows default config
    }

    return dwConfig;
}

BOOL IPV6Utils::SetIPV6Components(const IPV6Config enConfig)
{
    DWORD dwConfig = static_cast<DWORD>(enConfig);

    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    LSTATUS lRet = ::SHSetValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\TCPIP6\\Parameters", L"DisabledComponents", dwType, &dwConfig, dwSize);

    return lRet == ERROR_SUCCESS;
}


