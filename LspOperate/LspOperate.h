#pragma once
#include "..\CommonUtils\Singleton.h"
#include <memory>
#include <vector>
#include <Ws2spi.h>
#include <algorithm>

#define LSP_NAME    L"LittleLsp"
#define LSP_MODULE  L"LittleLsp.dll"



// {45C78E7A-E24F-4690-A0C5-115CF1A26C4A}
#define LSP_GUID { 0x45c78e7a, 0xe24f, 0x4690,{ 0xa0, 0xc5, 0x11, 0x5c, 0xf1, 0xa2, 0x6c, 0x4a } }



class CLspOperate
    : public LittleUtils::CSingleton<CLspOperate>
{
public:
    CLspOperate();
    virtual ~CLspOperate() { }

    GUID GetLspGuid();

    void EnumLspInfo();
    std::vector<WSAPROTOCOL_INFOW> GetProtocols();
    BOOL IsLspExist(const std::wstring& strLspName);
    BOOL InstallLsp();
    BOOL UnInstallLsp();

private:
    std::vector<WSAPROTOCOL_INFOW> GetProtocolsByApi();
    std::vector<WSAPROTOCOL_INFOW> GetProtocolsByRegistry();
};