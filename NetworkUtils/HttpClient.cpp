#include "stdafx.h"
#include "HttpClient.h"
#include <Winhttp.h>
#include <assert.h>
#include "..\CommonUtils\StringUtils.h"


#define HTTP_READBUF_LEN	1024*1024		//1M的接收缓冲区

inline void CloseInternetHandle(HINTERNET* hInternet)
{
	if ( *hInternet )
	{
		WinHttpCloseHandle(*hInternet);
		*hInternet = NULL;
	}
}


CWinHttp::CWinHttp(void)
	:m_hInternet(NULL)
	,m_hConnect(NULL)
	,m_hRequest(NULL)
	,m_nConnTimeout(5000)
	,m_nSendTimeout(5000)
	,m_nRecvTimeout(5000)
{
	//Init();
}

CWinHttp::~CWinHttp(void)
{
	Release();
}

bool CWinHttp::Init()
{
	if (m_hInternet == NULL)
	{
		m_hInternet = ::WinHttpOpen(
			L"Microsoft Internet Explorer",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);
		if (NULL == m_hInternet)
		{
			m_error = Hir_InitErr;
			return false;
		}
		::WinHttpSetTimeouts(m_hInternet, 0, m_nConnTimeout, m_nSendTimeout, m_nRecvTimeout);
	}

	return true;
}

void CWinHttp::Release()
{
	CloseInternetHandle(&m_hRequest);
	CloseInternetHandle(&m_hConnect);
	CloseInternetHandle(&m_hInternet);
}

bool CWinHttp::ConnectHttpServer(LPCWSTR lpIP, WORD wPort)
{
	m_hConnect = ::WinHttpConnect(m_hInternet, lpIP, wPort, 0);
	return m_hConnect != NULL;
}

bool CWinHttp::CreateHttpRequest(LPCWSTR lpPage, HttpRequest type, DWORD dwFlag/*=0*/)
{
	wchar_t* pVerb = (type == Hr_Get)?L"GET":L"POST";
	m_hRequest = ::WinHttpOpenRequest(
		m_hConnect,
		pVerb,
		lpPage,
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 
		dwFlag);
	return m_hRequest != NULL;
}

void CWinHttp::SetTimeOut( int dwConnectTime, int dwSendTime, int dwRecvTime )
{
	m_nConnTimeout = dwConnectTime;
	m_nSendTimeout = dwSendTime;
	m_nRecvTimeout = dwRecvTime;
}

bool CWinHttp::DownloadFile( LPCWSTR lpUrl, LPCWSTR lpFilePath )
{
	Release();
	if ( !Init() )
		return false;
	bool bRet = false;
	DWORD dwBytesToRead = 0, dwFileSize = 0, dwReadSize=0, dwRecvSize =0;
	if ( !InitConnect(lpUrl, Hr_Get) )
		return false;
	if ( !QueryContentLength(dwFileSize) )
	{
		m_error = Hir_QueryErr;
		return false; 
	}
	wstring strHeaders;
	bool bQuery = QueryRawHeaders(strHeaders);
	if ( bQuery && (strHeaders.find(L"404")!=wstring::npos) )
	{
		m_error = Hir_404;
		return false;
	}
	HANDLE hFile = CreateFile(lpFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == hFile )
	{
		m_error = Hir_CreateFileErr;
		return false;
	}
	SetFilePointer(hFile, dwFileSize, 0, FILE_BEGIN);
	SetEndOfFile(hFile);
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
	{
		CloseHandle(hFile);
		DeleteFile(lpFilePath);
		return false;
	}
	void* lpBuff = malloc(HTTP_READBUF_LEN);
	while( true )
	{
		if ( dwBytesToRead>HTTP_READBUF_LEN )
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
			break;
		DWORD dwWriteByte;
		if ( !WriteFile(hFile, lpBuff, dwReadSize, &dwWriteByte, NULL) || (dwReadSize != dwWriteByte) )
			break;
		dwRecvSize += dwReadSize;
// 		if( m_pCallback )
// 			m_pCallback->OnDownloadCallback(m_lpParam, DS_Loading, dwFileSize, dwRecvSize);
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			break;
		if ( dwBytesToRead<= 0 )
		{
			bRet = true;
			break;
		}
	}
	free(lpBuff);
	CloseHandle(hFile);
	if ( !bRet )
	{//下载失败，删除文件
		DeleteFile(lpFilePath);
	}
	return bRet;
}

bool CWinHttp::RequestAndDownload(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData, LPCSTR lpHeader, LPCWSTR lpFilePath)
{
	Release();
	if (!Init())
		return false;
	bool bRet = false;
	DWORD dwBytesToRead = 0, dwFileSize = 0, dwReadSize = 0, dwRecvSize = 0;
	if (!InitConnect(lpUrl, type, lpPostData, (lpHeader == NULL) ? NULL : StringUtils::MultiByteToUnicode(string(lpHeader), CP_UTF8).c_str()))
		return false;

	HANDLE hFile = CreateFile(lpFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD dwLastErr = GetLastError();
		m_error = Hir_CreateFileErr;
		return false;
	}

	SetFilePointer(hFile, dwFileSize, 0, FILE_BEGIN);
	SetEndOfFile(hFile);
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
	{
		CloseHandle(hFile);
		DeleteFile(lpFilePath);
		return false;
	}

	void* lpBuff = malloc(HTTP_READBUF_LEN);
	while (true)
	{
		if (dwBytesToRead > HTTP_READBUF_LEN)
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
			break;
		DWORD dwWriteByte;
		if (!WriteFile(hFile, lpBuff, dwReadSize, &dwWriteByte, NULL) || (dwReadSize != dwWriteByte))
			break;
		dwRecvSize += dwReadSize;
		if (m_pCallback)
			m_pCallback->OnDownloadCallback(m_lpParam, DS_Loading, dwFileSize, dwRecvSize);
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			break;
		if (dwBytesToRead <= 0)
		{
			bRet = true;
			break;
		}
	}
	free(lpBuff);
	CloseHandle(hFile);
	if (!bRet)
	{//下载失败，删除文件
		DeleteFile(lpFilePath);
	}
	return bRet;
}

bool CWinHttp::DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)
{
	bool bResult = false;
	BYTE* lpFileMem = NULL;
	void* lpBuff = NULL;
	DWORD dwLength = 0, dwBytesToRead = 0,  dwReadSize = 0, dwRecvSize = 0;
	try
	{
		if ( !InitConnect(lpUrl, Hr_Get) )
			throw Hir_InitErr;
		if ( !QueryContentLength(dwLength) )
			throw Hir_QueryErr;
		wstring strHeaders;
		bool bQuery = QueryRawHeaders(strHeaders);
		if ( bQuery && (strHeaders.find(L"404")!=wstring::npos) )
			throw Hir_404;
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			throw Hir_QueryErr;
		lpFileMem = (BYTE*)malloc(dwLength);
		lpBuff = malloc(HTTP_READBUF_LEN);
		while( true )
		{
			if ( dwBytesToRead>HTTP_READBUF_LEN )
			{
				free(lpBuff);
				lpBuff = malloc(dwBytesToRead);
			}
			if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
				throw Hir_DownloadErr;
			memcpy(lpFileMem+dwRecvSize, lpBuff, dwReadSize);
			dwRecvSize += dwReadSize;
			if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
				throw Hir_DownloadErr;
			if ( dwBytesToRead<= 0 )
			{
				bResult = true;
				break;
			}
		}
	}
	catch( HttpInterfaceError error )
	{
		m_error = error;
	}
	if ( lpBuff )
		free(lpBuff);
	if ( bResult )
	{
		*ppBuffer = lpFileMem;
		*nSize = dwRecvSize;
	}
	else
		free(lpFileMem);
	return bResult;
}

bool CWinHttp::SetCookie(LPCWSTR lpCookie)
{
	m_strCookie.Format(L"%s", lpCookie);
	return true;
}

wstring CWinHttp::GetRawHeaders()
{
	return m_strRawHeaders.GetString();
}

string CWinHttp::Request( LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData /*= NULL*/, LPCSTR lpHeader/*=NULL*/ )
{
	string strRet;
	wstring strUrl = StringUtils::MultiByteToUnicode(string(lpUrl), CP_UTF8);
	if ( !InitConnect(strUrl.c_str(), type, lpPostData, (lpHeader==NULL) ? NULL : StringUtils::MultiByteToUnicode(string(lpHeader), CP_UTF8).c_str()) )
		return strRet;
	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(HTTP_READBUF_LEN);
	bool bFinish = false;
	while ( true )
	{
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			break;
		if ( dwBytesToRead<= 0 )
		{
			bFinish = true;
			break;
		}
		if ( dwBytesToRead>HTTP_READBUF_LEN )
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
			break;
		strRet.append((const char*)lpBuff, dwReadSize);
	}
	free(lpBuff);
	if ( !bFinish )
		strRet.clear();
	return strRet;
}

string CWinHttp::Request( LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData /*= NULL*/, LPCWSTR lpHeader/*=NULL*/ )
{
	string strRet;
	if ( !InitConnect(lpUrl, type, lpPostData, lpHeader) )
		return strRet;
	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(HTTP_READBUF_LEN);
	bool bFinish = false;
	while ( true )
	{
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			break;
		if ( dwBytesToRead<= 0 )
		{
			bFinish = true;
			break;
		}
		if ( dwBytesToRead>HTTP_READBUF_LEN )
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
			break;
		strRet.append((const char*)lpBuff, dwReadSize);
	}
	free(lpBuff);
	if ( !bFinish )
		strRet.clear();
	return strRet;
}

bool CWinHttp::QueryRawHeaders( OUT wstring& strHeaders )
{
	bool bRet = false;
	DWORD dwSize = 0;
	BOOL bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
	if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() && dwSize > 0 )
	{
		wchar_t* lpData = (wchar_t*)malloc(dwSize);
		if (lpData)
		{
			bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, lpData, &dwSize, WINHTTP_NO_HEADER_INDEX);
			if (bResult)
			{
				strHeaders = lpData;
				bRet = true;
			}
			free(lpData);
			lpData = nullptr;
		}
	}
	return bRet;
}

bool CWinHttp::QueryContentLength( OUT DWORD& dwLength )
{
	bool bRet = false;
	wchar_t szBuffer[30] = {0};
	DWORD dwSize = 30*sizeof(wchar_t);
	if ( ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, szBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX) )
	{
		TCHAR *p = NULL;
		dwLength = wcstoul(szBuffer, &p, 10);
		bRet = true;
	}
	return bRet;
}

bool CWinHttp::AddCookie(wstring cookie)
{
	if (!cookie.empty())
	{
		wstring strCookieString = L"Cookie:" + cookie;
		BOOL bRet = WinHttpAddRequestHeaders(m_hRequest,
			/*L"Cookie:"*/strCookieString.c_str(),
			-1,
			WINHTTP_ADDREQ_FLAG_ADD);
		DWORD dwError = GetLastError();

		return true;
	}

	return false;
}

bool CWinHttp::InitConnect( LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData/*=NULL*/, LPCWSTR lpHeader/*=NULL*/ )
{
	Release();
	if ( !Init() )
		return false;

    wstring strHostName, strPage;
    WORD wPort;

    URL_COMPONENTS URI = { 0 };
    URI.dwStructSize = sizeof(URL_COMPONENTS);
    URI.dwHostNameLength = (DWORD)-1;
    URI.dwUrlPathLength = (DWORD)-1;
    if (!WinHttpCrackUrl(lpUrl, 0, 0, &URI))
    {
        m_error = Hir_IllegalUrl;
        return false;
    }

    strHostName = std::wstring(URI.lpszHostName, URI.dwHostNameLength);
    strPage = std::wstring(URI.lpszUrlPath, URI.dwUrlPathLength);
    wPort = URI.nPort;

	if ( !ConnectHttpServer(strHostName.c_str(), wPort) )
	{
		m_error = Hir_ConnectErr;
		return false;
	}
	DWORD flag = WINHTTP_FLAG_REFRESH;
	if (wPort == 443 || wPort == 7004)		// 7004为移动版本测试端口 
	{
		flag |= WINHTTP_FLAG_SECURE;
	}

	if ( !CreateHttpRequest(strPage.c_str(), type, flag) )
	{
		m_error = Hir_InitErr;
		return false;
	}

	AddCookie(m_strCookie.GetString());

	if ( !SendHttpRequest(lpPostData, lpHeader) )
	{
		m_error = Hir_SendErr;
		return false;
	}
	if ( !WinHttpReceiveResponse(m_hRequest, NULL) )
	{
		m_error = Hir_InitErr;;
		return false;
	}

	wstring strRawHeaders;
	if (!QueryRawHeaders(strRawHeaders))
	{
		m_strRawHeaders.Empty();
	}
	else
	{
		m_strRawHeaders = strRawHeaders.c_str();
	}
	return true;
}

bool CWinHttp::SendHttpRequest( LPCSTR lpPostData/*=NULL*/, LPCWSTR lpHeader/*=NULL*/ )
{
	DWORD dwSize = (NULL==lpPostData) ? 0 : strlen(lpPostData);
	if ( lpHeader == NULL )
		return ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)lpPostData, dwSize, dwSize, NULL) == TRUE;
	return ::WinHttpSendRequest(m_hRequest, lpHeader, -1L, (LPVOID)lpPostData, dwSize, dwSize, NULL) == TRUE;
}