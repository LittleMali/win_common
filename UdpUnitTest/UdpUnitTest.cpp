// UdpUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UpdBlock.h"
#include "UdpSelectBlock.h"
#include "UdpSelectNonBlock.h"


int main()
{
    WSAData wsData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsData);

    bool bret = false;
    
    //bret = UdpBlockTest();
    //bret = UdpSelectBlockTest();
    bret = UdpSelectNonBlockTest();

    WSACleanup();
    getchar();
    return 0;
}

