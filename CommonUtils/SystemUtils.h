#pragma once
namespace SystemUtils
{
    // 是否64b系统
    BOOL IsSystem64b();

    // 是否wow64
    BOOL IsWow64();

    enum class WinSysVer
    {
        WinUnknown = 0,
        WinNt = 40,
        Win2000 = 50,
        WinXp = 51,
        WinServer2003 = 52,
        WinVista = 601,
        WinServer2008 = 603,
        Win7 = 611,
        WinServer2008R2 = 613,
        Win8 = 621,
        WinServer2012 = 623,
        Win81 = 631,
        WinServer2012R2 = 633,
        Win10 = 1001,
        WinServer2016 = 1003,
    };
    // Applications not manifested for Windows 8.1 or Windows 10 will return the Windows 8 OS version value (6.2). 
    // To manifest your applications for Windows 8.1 or Windows 10, refer to Targeting your application for Windows.
    WinSysVer GetWinSysVersion();
}