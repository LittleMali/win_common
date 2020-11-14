#include "stdafx.h"
#include "LspUtils.h"

std::string LspUtils::lsp_inet_ntoa(ULONG uIp)
{
    char szIp[32] = { 0 };
    std::string strIp;
    unsigned char* bytes = (unsigned char*)&uIp;
    sprintf_s(szIp, sizeof(szIp), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[4]);
    strIp = szIp;
    return strIp;
}

