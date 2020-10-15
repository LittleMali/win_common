#include "stdafx.h"
#include "ProcessUtils.h"
#include "Log.h"

BOOL ProcessUtils::RunExe(const std::wstring& strExeFilePath, const std::wstring& strParams, DWORD* pdwLastError /* = nullptr */, HANDLE* phProcess /* = nullptr */)
{
    if (strExeFilePath.empty())
    {
        return FALSE;
    }

    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);

    wchar_t szCmdline[2048] = { 0 };
    DWORD dwLastErr = 0;

    _snwprintf_s(szCmdline, _TRUNCATE, L"\"%s\" %s", strExeFilePath.c_str(), strParams.c_str());
    BOOL bRet = ::CreateProcessW(nullptr, szCmdline, nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    if (!bRet)
    {
        dwLastErr = ::GetLastError();
        DBGLOGW(L"create process failed, last err=%d", dwLastErr);
        if (pdwLastError)
        {
            *pdwLastError = dwLastErr;
        }
        return FALSE;
    }

    if (pi.hProcess)
    {
        if (phProcess)
        {
            *phProcess = pi.hProcess;
        }
        else
        {
            CloseHandle(pi.hProcess);
        }
    }
    
    if (pi.hThread)
    {
        CloseHandle(pi.hThread);
    }
    return TRUE;
}

