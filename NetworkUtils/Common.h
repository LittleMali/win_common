#pragma once
#include <string>
using std::wstring;
using std::string;


inline void MyParseUrlW( LPCWSTR lpUrl, wstring& strHostName, wstring& strPage, WORD& sPort )
{ 
	wstring strTemp(lpUrl);

	int nPos = strTemp.find(_T("http://"));
	if (std::string::npos != nPos)
	{
		strTemp = strTemp.substr(nPos + 7, strTemp.size() - nPos - 7);
		sPort = 80; 
	}
	else
	{
		nPos = strTemp.find(_T("https://"));
		strTemp = strTemp.substr(nPos + 8, strTemp.size() - nPos - 8);
		sPort = 443;
	} 

	nPos=strTemp.find('/');
	if ( wstring::npos == nPos )//没有找到 /
		strHostName=strTemp;
	else
		strHostName = strTemp.substr(0, nPos);
	int nPos1 = strHostName.find(':');
	if ( nPos1 != wstring::npos )
	{
		wstring strPort = strTemp.substr(nPos1+1, strHostName.size()-nPos1-1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (WORD)_wtoi(strPort.c_str());
	}
	if ( wstring::npos == nPos )
		return ;
	strPage=strTemp.substr(nPos, strTemp.size()-nPos);
} 

inline wstring Utf2U(const string& strUtf8)  
{  
	wstring wstrRet(L"");
	int nLen=MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);  
	if (nLen == ERROR_NO_UNICODE_TRANSLATION)  
		throw "Utf8ToUnicode出错：无效的UTF-8字符串。";  
	wstrRet.resize(nLen+1,'\0');
	MultiByteToWideChar(CP_UTF8,0,strUtf8.c_str(), -1,(LPWSTR)wstrRet.c_str(), nLen);
	return wstrRet;  
}
 

inline wstring A2U(const string& str)
{
	wstring strDes;
	if ( str.empty() )
		goto __end;
	int nLen=::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if ( 0==nLen )
		goto __end;
	wchar_t* pBuffer=new wchar_t[nLen+1];
	memset(pBuffer, 0, nLen+1);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen);
	pBuffer[nLen]='\0';
	strDes.append(pBuffer);
	delete[] pBuffer;
__end:
	return strDes;
}
 