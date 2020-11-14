// UdpUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UpdBlock.h"


int main()
{
    WSAData wsData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsData);

    bool bret = false;
    
    bret = UdpBlockTest();

    WSACleanup();
    getchar();
    return 0;
}

