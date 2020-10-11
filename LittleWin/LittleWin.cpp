// LittleWin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "..\CommonUtils\Log.h"

int main()
{
    std::cout << "this is little's win32 project" << std::endl;

    DBGLOGW(L"%d", 123);
    return 0;
}

