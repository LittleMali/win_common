// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>


// TODO: reference additional headers your program requires here

#include "..\CommonUtils\Log.h"

//#define UDP_SRV_ADDR    "127.0.0.1"
#define UDP_SRV_ADDR    "148.70.209.112"
#define UDP_SRV_PORT    12345
//#define UDP_SRV_PORT    1207

#define BUF_LEN 2048