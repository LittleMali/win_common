#include "stdafx.h"
#include "MsgWnd.h"

CMsgWnd::CMsgWnd(ISocketCallback* pCallback)
    : m_pCallback(pCallback)
{

}

CMsgWnd::~CMsgWnd()
{
    m_pCallback = nullptr;
}

bool CMsgWnd::Init()
{
    // RECT rect = { 0, 0, 300, 400 };
    if (!SuperWnd::Create(HWND_MESSAGE))
    {
        DBGLOGW(L"create msg wnd failed");
        return false;
    }

    //ShowWindow(SW_SHOW);

    return true;
}

void CMsgWnd::UnInit()
{
    if (m_hWnd)
    {
        SuperWnd::DestroyWindow();
        m_hWnd = NULL;
    }
}

int CMsgWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    return 0;
}

void CMsgWnd::OnDestroy()
{

}

LRESULT CMsgWnd::OnWmSocket(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_pCallback)
    {
        m_pCallback->OnSocket(wParam, lParam);
    }

    return 0;
}
