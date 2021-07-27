#include "stdafx.h"
#include "SystemUtils.h"

BOOL SystemUtils::IsSystem64b()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL SystemUtils::IsWow64()
{
    typedef BOOL(WINAPI *lpfnIsWow64Process)(HANDLE, PBOOL);
    lpfnIsWow64Process pfn = nullptr;

    BOOL bIsWow64 = FALSE;
    pfn = (lpfnIsWow64Process)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
    if (pfn)
    {
        if (!pfn(GetCurrentProcess(), &bIsWow64))
        {
            // handle error
        }
    }

    return bIsWow64;
}

// GetVersionEx is deprecated
#pragma warning(disable : 4996)

SystemUtils::WinSysVer SystemUtils::GetWinSysVersion()
{
    WinSysVer enSysVer = WinSysVer::WinUnknown;

    OSVERSIONINFOEX os = { 0 };
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((OSVERSIONINFO*)&os))
    {
        return enSysVer;
    }

    if (os.dwMajorVersion <= 4)
    {
        enSysVer = WinSysVer::WinNt;
    }
    else if (os.dwMajorVersion == 5)
    {
        if (os.dwMinorVersion == 0)
        {
            enSysVer = WinSysVer::Win2000;
        }
        else if (os.dwMinorVersion == 1)
        {
            enSysVer = WinSysVer::Win2000;
        }
        else if (os.dwMinorVersion == 2)
        {
            enSysVer = WinSysVer::WinServer2003;
        }
    }
    else if (os.dwMajorVersion == 6)
    {
        if (os.dwMinorVersion == 0)
        {
            enSysVer = (os.wProductType == VER_NT_WORKSTATION) ? WinSysVer::WinVista : WinSysVer::WinServer2008;
        }
        else if (os.dwMinorVersion == 1)
        {
            enSysVer = (os.wProductType == VER_NT_WORKSTATION) ? WinSysVer::Win7 : WinSysVer::WinServer2008R2;
        }
        else if (os.dwMinorVersion == 2)
        {
            enSysVer = (os.wProductType == VER_NT_WORKSTATION) ? WinSysVer::Win8 : WinSysVer::WinServer2012;
        }
        else if (os.dwMinorVersion == 3)
        {
            enSysVer = (os.wProductType == VER_NT_WORKSTATION) ? WinSysVer::Win81 : WinSysVer::WinServer2012R2;
        }
    }
    else if (os.dwMajorVersion == 10)
    {
        enSysVer = (os.wProductType == VER_NT_WORKSTATION) ? WinSysVer::Win10 : WinSysVer::WinServer2016;
    }

    return enSysVer;
}