#include "stdafx.h"
#include "LnkUtils.h"
#include <shlobj.h>
#include <winbase.h>
#include <shlwapi.h>
#include "..\CommonUtils\Log.h"


BOOL CLnkUtils::ReplaceLnk(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, const int nWalkInDirDeepth /*= 0*/)
{
    if (!lpDstDir || !::PathFileExists(lpDstDir))
        return FALSE;

    return ReplaceLnkImpl(lpDstDir, lpOldFilePath, lpNewFilePath, nWalkInDirDeepth, 0);
}

BOOL CLnkUtils::IsLnkExists(LPCTSTR lpCheckDir, LPCTSTR lpFileName)
{
	if (!lpCheckDir || !lpFileName)
		return FALSE;

	TCHAR szFindFile[MAX_PATH + 2] = {0};
	_sntprintf_s(szFindFile, MAX_PATH, MAX_PATH, L"%s\\*.*", lpCheckDir);


	WIN32_FIND_DATA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = ::FindFirstFile(szFindFile, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BOOL bExists = FALSE;
	do 
	{
		if (StrCmpI(findData.cFileName, L".") == 0 || StrCmpI(findData.cFileName, L"..") == 0)
		{
			continue;
		}

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		LPTSTR lpExt = ::PathFindExtension(findData.cFileName);
		if (_tcsicmp(findData.cFileName, lpFileName) == 0)
		{
			bExists = TRUE;
			break;
		}

	} while (::FindNextFile(hFind, &findData));

	::FindClose(hFind);
	hFind = INVALID_HANDLE_VALUE;

	return bExists;
}

BOOL CLnkUtils::ReplaceLnkImpl(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, const int nWalkInDirDeepth, int nCurDeepth)
{
    if (nCurDeepth > nWalkInDirDeepth)
        return TRUE;

    nCurDeepth++;

    TCHAR szFindFile[MAX_PATH + 2] = {0};
    _sntprintf_s(szFindFile, MAX_PATH, MAX_PATH, L"%s\\*.*", lpDstDir);

    WIN32_FIND_DATA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = ::FindFirstFile(szFindFile, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    do 
    {
        if (StrCmpI(findData.cFileName, L".") == 0 || StrCmpI(findData.cFileName, L"..") == 0)
        {
            continue;
        }

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (nWalkInDirDeepth != 0))
        {
            //if (nCurDeepth < nWalkInDirDeepth)
            {                
                TCHAR szChildDir[MAX_PATH + 2] = {0};
                ::PathCombine(szChildDir, lpDstDir, findData.cFileName);

                ReplaceLnkImpl(szChildDir, lpOldFilePath, lpNewFilePath, nWalkInDirDeepth, nCurDeepth);
            }
        }
        else
        {
            LPTSTR lpExt = ::PathFindExtension(findData.cFileName);
            if (lpExt && StrCmpI(lpExt, L".lnk") == 0)
            {
                TCHAR szLnkFile[MAX_PATH + 2] = {0};
                ::PathCombine(szLnkFile, lpDstDir, findData.cFileName);

                CheckLnk(szLnkFile, lpOldFilePath, lpNewFilePath);
            }
        }

    } while (::FindNextFile(hFind, &findData));

    ::FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    return TRUE;
}

BOOL CLnkUtils::CheckLnk(LPCTSTR lpLnk, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath)
{
    if (!lpLnk)
        return FALSE;

    HRESULT hres = 0;
    IShellLink* psl = NULL;
    IPersistFile* ppf = NULL;

    TCHAR szGetRawPath[MAX_PATH + 2] = {0}, szExpandPath[MAX_PATH + 2] = {0};
    WIN32_FIND_DATA wfd;

    TCHAR szIconLoc[MAX_PATH] = {0};
    int nIconIndex = 0;

    // Get a pointer to the IShellLink interface.
    hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
    if (FAILED(hres) || !psl)
    {
        DBGLOGW(L"CoCreateInstance failed, hres=%08x.", hres);
        goto Exit0;
    }

    // Get a pointer to the IPersistFile interface. 
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hres) || !ppf)
    {
        DBGLOGW(L"QueryInterface failed, hres=%08x.", hres);
        goto Exit0;
    }

    // Load the shortcut. 
    hres = ppf->Load(lpLnk, STGM_READWRITE);
    if (FAILED(hres))
    {
        DBGLOGW(L"Load failed, hres=%08x.", hres);
        goto Exit0;
    }

    // Get the path to the link target. 
    hres = psl->GetPath(szGetRawPath, MAX_PATH, &wfd, SLGP_RAWPATH);
    if (FAILED(hres))
    {
        DBGLOGW(L"GetPath failed, hres=%08x.", hres);
        goto Exit0;
    }

    DBGLOGW(L"szGetRawPath = %s", szGetRawPath);

    if (0 == ::ExpandEnvironmentStrings(szGetRawPath, szExpandPath, MAX_PATH))
    {
        DBGLOGW(L"ExpandEnvironmentStrings failed, last err=%d", GetLastError());
        goto Exit0;
    }

    DBGLOGW(L"szExpandPath = %s", szExpandPath);
    DBGLOGW(L"lpOldFilePath = %s", lpOldFilePath);

    if (StrCmpI(szExpandPath, lpOldFilePath) != 0)
    {
        DBGLOGW(L"StrCmpI failed, last err=%d", GetLastError());
        goto Exit0;
    }

    DBGLOGW(L"find old lnk file, lnk: %s", lpLnk);

    hres = psl->GetIconLocation(szIconLoc, MAX_PATH, &nIconIndex);
    if (FAILED(hres))
    {
        DBGLOGW(L"get icon location failed, hres=%08x.", hres);
        goto Exit0;
    }

    DBGLOGW(L"get icon location suc, nIconIndex=%d.", nIconIndex);

    hres = psl->SetIconLocation(lpNewFilePath, nIconIndex);
    if (FAILED(hres))
    {
        //
        DBGLOGW(L"set icon location failed, hres=%08x.", hres);
    }

    hres = psl->SetPath(lpNewFilePath);
    if (FAILED(hres))
    {
        DBGLOGW(L"set path failed, hres=%08x.", hres);
        goto Exit0;
    }

    hres = ppf->Save(lpLnk, TRUE);
    if (FAILED(hres))
    {
        DBGLOGW(L"save data failed, hres=%08x.", hres);
        goto Exit0;
    }
    
    DBGLOGW(L"find target lnk and save suc");

Exit0:
    if (ppf)
    {
        ppf->Release();
        ppf = NULL;
    }

    if (psl)
    {
        psl->Release();
        psl = NULL;
    }

    return TRUE;
}
