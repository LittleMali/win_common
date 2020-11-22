#pragma once
#include "MsgWnd.h"

// 异步select
// 核心API： WSAAsyncSelect
// 基于窗口的网络通知，需要创建一个msg wnd并接收网络消息。

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