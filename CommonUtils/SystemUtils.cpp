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

