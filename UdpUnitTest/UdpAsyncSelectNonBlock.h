#pragma once
#include "MsgWnd.h"

class CUdpAsyncSelectNonBlock
    : public CMsgWnd::ISocketCallback
{
public:
    CUdpAsyncSelectNonBlock();
    virtual ~CUdpAsyncSelectNonBlock();

    // CMsgWnd::IMsgWndInterface
    virtual void OnSocket(WPARAM wParam, LPARAM lParam);

    bool Run();

private:
    bool m_bInitSuc;
    CMsgWnd m_socketMsgWnd;

    SOCKET m_socket;
    sockaddr_in m_srvAddr;
};