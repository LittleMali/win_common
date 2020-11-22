#pragma once

// 事件select
// 核心API： WSAEventSelect, WSAWaitForMultipleEvents, WSAEnumNetworkEvents
// 创建一个event并等待网络通知。
// 在socket创建完之后，一次性注册所有感兴趣的网络通知。

bool UdpEventSelectNonBlockTest();