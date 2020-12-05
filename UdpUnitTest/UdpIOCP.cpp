#include "stdafx.h"
#include "UdpIOCP.h"
#include <process.h>
#include <memory>

static volatile long g_nTestCnt = 0;
static long g_nMaxTestCnt = 10000;

static HANDLE g_hQuitEvent = NULL;

unsigned __stdcall UdpIOCPThread(void* param)
{
    HANDLE hCompPort = (HANDLE)param;

    DWORD dwBytesTransferred = 0;
    ULONG_PTR ulCompKey = 0;
    UdpIocpOverlapped* pOverlapped = nullptr;

    while (true)
    {
        BOOL bRet = GetQueuedCompletionStatus(hCompPort, &dwBytesTransferred, &ulCompKey, (LPOVERLAPPED*)&pOverlapped, 3000);
        if (!bRet)
        {
            DWORD dwLastErr = GetLastError();
            if (dwLastErr == WAIT_TIMEOUT)
            {
                DWORD dd = dwLastErr;
                DBGLOGW(L"tid[%d] get queued timeout, last err=%d", ::GetCurrentThreadId(), dd);
                continue;
            }
            DBGLOGW(L"tid[%d] get queued failed, last err=%d", ::GetCurrentThreadId(), dwLastErr);
            break;
        }

        if (InterlockedCompareExchange(&g_nTestCnt, g_nMaxTestCnt, g_nMaxTestCnt) >= g_nMaxTestCnt)
        {
            DBGLOGW(L"tid [%d], test cnt reached max.", ::GetCurrentThreadId());
            SetEvent(g_hQuitEvent);
            break;
        }

        switch (pOverlapped->enCompKey)
        {
        case UdpIocpOverlapped::CompKey::ToRecv:
        {
            pOverlapped->enCompKey = UdpIocpOverlapped::CompKey::ToSend;
            pOverlapped->ClearSendBuf();
            pOverlapped->InitOverlapped();

            DWORD dwRecvBytes = 0;
            DWORD dwFlags = 0;
            sockaddr_in addrFrom = { 0 };
            int nAddrFromLen = sizeof(addrFrom);

            int nRet = WSARecvFrom(pOverlapped->sock, &pOverlapped->wsaRecvBuf, 1, &dwRecvBytes, &dwFlags, 
                (sockaddr*)&addrFrom, &nAddrFromLen,
                (LPOVERLAPPED)pOverlapped, nullptr);
            if (nRet == SOCKET_ERROR)
            {
                DWORD dwLastErr = GetLastError();
                if (dwLastErr != WSA_IO_PENDING)
                {
                    DBGLOGW(L"recvfrom failed, last err=%d", ::GetLastError());
                    continue;
                }
                DBGLOGW(L"recv wait for io pendding");
            }
            else if (nRet == 0)
            {
                DBGLOGW(L"recv finished directly");
            }
        }
        break;
        
        case UdpIocpOverlapped::CompKey::ToSend:
        {
            pOverlapped->enCompKey = UdpIocpOverlapped::CompKey::ToRecv;

            char* pRecvData = pOverlapped->ReadRecvBuf();
            DBGLOGA("recv data: %s", pRecvData);
            pOverlapped->ClearRecvBuf();
            pOverlapped->InitOverlapped();

            DWORD dwSendBytes = 0;
            DWORD dwFlags = 0;
            pOverlapped->WriteSendBuf("udp iocp send", g_nTestCnt);

            InterlockedExchangeAdd(&g_nTestCnt, 1);

            int nRet = WSASendTo(pOverlapped->sock, &pOverlapped->wsaSendBuf, 1, &dwSendBytes, dwFlags,
                (const sockaddr*)(&pOverlapped->addrSrv), sizeof(pOverlapped->addrSrv),
                (LPOVERLAPPED)pOverlapped, nullptr);
            if (nRet == SOCKET_ERROR)
            {
                DWORD dwLastErr = GetLastError();
                if (dwLastErr != WSA_IO_PENDING)
                {
                    DBGLOGW(L"sendto failed, last err=%d", ::GetLastError());
                    continue;
                }
                DBGLOGW(L"send wait for io pendding");
            }
            else if (nRet == 0)
            {
                DBGLOGW(L"send finished directly");
            }
        }
        break;

        case UdpIocpOverlapped::CompKey::ToClose:
        {
            DBGLOGW(L"tid [%d], notify quit.", ::GetCurrentThread());
            break;
        }
        break;

        default:
            break;
        }
    }
    
    return 0;
}

bool UdpIOCPTest()
{
    // create iocp
    HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hCompletionPort == NULL)
    {
        return false;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD dwThreadCnt = sysInfo.dwNumberOfProcessors * 2;
    dwThreadCnt = 6;

    std::vector<HANDLE> vecThreadHandle;
    for (size_t i = 0; i < dwThreadCnt; ++i)
    {
        HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, UdpIOCPThread, hCompletionPort, 0, nullptr);
        if (hThread)
        {
            vecThreadHandle.push_back(hThread);
        }
    }

    // create socket
    SOCKET s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (s == SOCKET_ERROR)
    {
        return false;
    }
    
    DWORD dwMode = 1;
    int nErr = ioctlsocket(s, FIONBIO, &dwMode);
    if (nErr == SOCKET_ERROR)
    {
        closesocket(s);
        return false;
    }

    // associate iocp and socket
    HANDLE hRet = CreateIoCompletionPort((HANDLE)s, hCompletionPort, 0, 0);
    if (hRet == NULL)
    {
        closesocket(s);
        CloseHandle(hCompletionPort);
        return false;
    }

    int nValidThreadCnt = vecThreadHandle.size();
    //std::shared_ptr<UdpIocpOverlapped> spIocpOverlappedArr(new (std::nothrow) UdpIocpOverlapped[nValidThreadCnt], 
    //        [](UdpIocpOverlapped* pArr) { delete[] pArr; });

    std::vector<std::shared_ptr<UdpIocpOverlapped>> vecIocpOverlapped;

    for (int i = 0; i < nValidThreadCnt; ++i)
    {
        std::shared_ptr<UdpIocpOverlapped> sp = std::make_shared<UdpIocpOverlapped>();
        if (sp)
        {
            vecIocpOverlapped.push_back(sp);

            sp->sock = s;
            sp->enCompKey = UdpIocpOverlapped::CompKey::ToSend;
            sp->addrSrv.sin_family = AF_INET;
            inet_pton(AF_INET, UDP_SRV_ADDR, &sp->addrSrv.sin_addr.s_addr);
            sp->addrSrv.sin_port = htons(UDP_SRV_PORT);

            PostQueuedCompletionStatus(hCompletionPort, 0, static_cast<ULONG_PTR>(UdpIocpOverlapped::CompKey::ToSend), (LPOVERLAPPED)(sp.get()));
        }
    }

    DBGLOGW(L"start udp test and wait for quit singled");

    // wait test finish
    g_hQuitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    DWORD dwRet = WaitForSingleObject(g_hQuitEvent, INFINITE);
    if (dwRet == WAIT_OBJECT_0)
    {
        DBGLOGW(L"test cnt is reached max");
    }
    else 
    {
        DBGLOGW(L"test wait error");
    }

    // notify quit
    for (int i = 0; i < nValidThreadCnt; ++i)
    {
        std::shared_ptr<UdpIocpOverlapped> sp = vecIocpOverlapped[i];
        sp->InitOverlapped();
        sp->enCompKey = UdpIocpOverlapped::CompKey::ToClose;

        PostQueuedCompletionStatus(hCompletionPort, 0, static_cast<ULONG_PTR>(UdpIocpOverlapped::CompKey::ToClose), (LPOVERLAPPED)(sp.get()));
    }
    
    // wait all work thread quit
    dwRet = WaitForMultipleObjects(vecThreadHandle.size(), &vecThreadHandle[0], TRUE, INFINITE);
    if (dwRet == WAIT_FAILED)
    {
        DWORD dwLastErr = GetLastError();
        DBGLOGW(L"wait for quit failed, last err=%d", dwLastErr);
    }
    else
    {

    }

    for (int i = 0; i < nValidThreadCnt; ++i)
    {
        CloseHandle(vecThreadHandle[i]);
    }

    closesocket(s);
    CloseHandle(hCompletionPort);

    return true;
}