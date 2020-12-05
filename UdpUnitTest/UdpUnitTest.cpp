// UdpUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UpdBlock.h"
#include "UdpSelectBlock.h"
#include "UdpSelectNonBlock.h"
#include "UdpAsyncSelectNonBlock.h"
#include "UdpEventSelectNonBlock.h"
#include "UdpOverlappedEvent.h"
#include "UdpOverlappedCompletionRoutine.h"
#include "UdpIOCP.h"

int main()
{
    WSAData wsData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsData);

    bool bret = false;
    
    //bret = UdpBlockTest();
    //bret = UdpSelectBlockTest();
    //bret = UdpSelectNonBlockTest();
    
    //CUdpAsyncSelectNonBlock udpAsyncSelectNonBlock;
    //bret = udpAsyncSelectNonBlock.Run();

    //bret = UdpEventSelectNonBlockTest();

    //bret = UdpOverlappedEventTest();

    //bret = UdpOverlappedCompletionRoutineTest();

    bret = UdpIOCPTest();

    WSACleanup();

    printf("press any key to exit!\n");

    getchar();
    return 0;
}

