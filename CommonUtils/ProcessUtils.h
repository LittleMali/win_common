#pragma once

#include <string>

namespace ProcessUtils
{
    BOOL RunExe(const std::wstring& strExeFilePath, const std::wstring& strParams, DWORD* pdwLastError = nullptr, HANDLE* phProcess = nullptr);
}