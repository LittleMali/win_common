#ifndef _LNK_UTILS_H_
#define _LNK_UTILS_H_

#include "shobjidl.h"
#include "shlguid.h"
#include <string>

class CLnkUtils
{
public:
    BOOL ReplaceLnk(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, const int nWalkInDirDeepth = 0);
	BOOL IsLnkExists(LPCTSTR lpCheckDir, LPCTSTR lpFileName);

    BOOL ReplaceLnkImpl(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, const int nWalkInDirDeepth, int nCurDeepth);
    BOOL CheckLnk(LPCTSTR lpLnk, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath);

};

#endif
