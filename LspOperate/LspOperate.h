#pragma once
#include "..\CommonUtils\Singleton.h"
#include <memory>
#include <Ws2spi.h>

#define LSP_NAME    L"QMProxyAccLsp"
#define LSP_MODULE  L"QMProxyAccLsp.dll"

class CLspOperate
    : public LittleUtils::CSingleton<CLspOperate>
{
public:
    CLspOperate();
    virtual ~CLspOperate() { }

    void EnumLspInfo();
    BOOL IsLspExist(const std::wstring& strLspName);

private:
    std::shared_ptr<WSAPROTOCOL_INFOW> GetProtocols(int* pProtocolCount);
};