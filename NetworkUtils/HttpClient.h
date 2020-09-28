#pragma once
#include "IHttpInterface.h"
#include <atlstr.h>

#ifndef HINTERNET
#define HINTERNET LPVOID
#endif

class IHttp
{
public:
	IHttp()
		: m_lpParam(NULL)
		, m_error(Hir_Success)
		, m_pCallback(NULL)
	{

	}
protected:
	void*	m_lpParam;
	HttpInterfaceError	m_error;
	IHttpCallback*	m_pCallback;
};

class CWinHttp
	: public IWinHttp
	, public IHttp
{
public:
	CWinHttp(void);
	virtual ~CWinHttp(void);
	
	virtual void	SetTimeOut(int dwConnectTime,  int dwSendTime, int dwRecvTime);
	virtual string	Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader=NULL);
	virtual string	Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCWSTR lpHeader=NULL);
	virtual void	FreeInstance()													{ delete this;		}
	virtual HttpInterfaceError GetErrorCode()										{ return m_error;	}
	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam)		{ m_pCallback = pCallback; m_lpParam = pParam; }
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath);
	virtual bool	RequestAndDownload(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData, LPCSTR lpHeader, LPCWSTR lpFilePath);
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);
	virtual bool	SetCookie(LPCWSTR lpCookie);
	virtual wstring	GetRawHeaders();

protected:
	bool	Init();
	void	Release();
	//init
	bool	InitConnect(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData=NULL, LPCWSTR lpHeader=NULL);
	bool	ConnectHttpServer(LPCWSTR lpIP, WORD wPort);
	bool	CreateHttpRequest(LPCWSTR lpPage, HttpRequest type, DWORD dwFlag=0);
	bool	SendHttpRequest(LPCSTR lpPostData=NULL, LPCWSTR lpHeader=NULL);
	//query 
	bool	QueryRawHeaders(OUT wstring& strHeaders);
	bool	QueryContentLength(OUT DWORD& dwLength);
	bool	AddCookie(wstring cookie);

private:
	HINTERNET	m_hInternet;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	int			m_nConnTimeout;
	int			m_nSendTimeout;
	int			m_nRecvTimeout;
	CAtlString	m_strCookie;
	CAtlString	m_strRawHeaders;
};

