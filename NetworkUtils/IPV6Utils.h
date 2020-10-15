#pragma once

namespace IPV6Utils {
    // https://docs.microsoft.com/en-us/troubleshoot/windows-server/networking/configure-ipv6-in-windows
    enum class IPV6Config :DWORD
    {
        // Disable IPv6
        DisableIPv6_All_Interfaces = 0xff,
        // windows default
        Default = 0x00,
        //disable IPv6 on all nontunnel interfaces
        DisableIPV6_Nontunnel_Interfaces = 0x10,
        // Prefer IPv4 over IPv6
        PreferIPv4OverIPv6 = 0x20,
        // prefer IPv6 over IPv4
        // PreferIPv6OverIPv4 = 0x00, // 见doc文档，按位清除才行
        // disable IPv6 on all nontunnel interfaces (except the loopback) and on IPv6 tunnel interface
        DisableIPV6_Nontunnel_Tunnel_Interfaces = 0x11,

    };
    BOOL IPV6DisablePrivacyState();
    BOOL IPV6EnablePrivacyState();

    DWORD GetIPV6Config();
    BOOL SetIPV6Components(const IPV6Config enConfig);
}