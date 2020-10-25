// LittleWin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "..\CommonUtils\Log.h"
#include "..\CommonUtils\Singleton.h"
#include "..\LspOperate\LspOperate.h"


int main()
{
    std::cout << "this is little's win32 project" << std::endl;

    DBGLOGW(L"%d", 123);

    CLspOperate::Instance()->EnumLspInfo();
    BOOL bExits = CLspOperate::Instance()->IsLspExist(LSP_NAME);
    BOOL bInstall = CLspOperate::Instance()->InstallLsp();

    getchar();
    return 0;
}

