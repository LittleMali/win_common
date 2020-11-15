#pragma once
#include "atlbase.h"
#include "atlapp.h"
#include "atlwin.h"
#include "atlframe.h"
#include "atlctrls.h"
#include "atldlgs.h"
#include "atlctrlw.h"
#include "atlcrack.h"

typedef CWinTraits<WS_POPUP | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0> CMsgWndTraits;

#define WM_SOCKET   (WM_USER + 101)

class CMsgWnd:
    public ATL::CWindowImpl<CMsgWnd, CWindow, CMsgWndTraits>
{
    typedef ATL::CWindowImpl<CMsgWnd, CWindow, CMsgWndTraits> SuperWnd;
public:
    struct ISocketCallback
    {
        virtual void OnSocket(WPARAM wParam, LPARAM lParam) = 0;
    };

    DECLARE_WND_CLASS(L"Little_MsgWnd");
    static LPCTSTR GetWndCaption()
    {
        return L"Little_MsgWnd";
    }

    CMsgWnd(ISocketCallback* pCallback);
    virtual ~CMsgWnd();

    bool Init();
    void UnInit();

    BEGIN_MSG_MAP_EX(CMsgWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER_EX(WM_SOCKET, OnWmSocket)
    END_MSG_MAP()

protected:
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
    LRESULT OnWmSocket(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ISocketCallback* m_pCallback;
};