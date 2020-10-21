#pragma once
#include "..\CommonUtils\Singleton.h"
#include <memory>
#include <vector>
#include <Ws2spi.h>
#include <algorithm>

#define LSP_NAME    L"QMProxyAccLsp"
#define LSP_MODULE  L"QMProxyAccLsp.dll"

class CLspOperate
    : public LittleUtils::CSingleton<CLspOperate>
{
public:
    CLspOperate();
    virtual ~CLspOperate() { }

    void EnumLspInfo();
    std::vector<WSAPROTOCOL_INFOW> GetProtocols();
    BOOL IsLspExist(const std::wstring& strLspName);


private:
    std::vector<WSAPROTOCOL_INFOW> GetProtocolsByApi();
    std::vector<WSAPROTOCOL_INFOW> GetProtocolsByRegistry();
};