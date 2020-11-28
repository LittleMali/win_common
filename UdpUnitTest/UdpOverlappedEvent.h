#pragma once

// 使用基于event的overlapped socket模型。
// 1、overlapped socket这一套模型是使用WSASocket，WSASendto等系列函数。
// 2、收发数据可能直接成功，也有可能进入IO_PENDING等待收发操作完成，若进入
// io pending则需要wait overlapped.hEvent事件，系统会在操作完成后set event
// 通知我们，在收到通知后再使用WSAGetOverlappedResult获取操作结果。
// 3、这一套API确实比较复杂，收发--wait--get result，每一步都要判断成功/失败/阻塞。
// 4、注意到，我们在代码中有几处地方调用了 WSASetEvent， WSAResetEvent 置位op.hEvent。
// 这点也很好理解，举个例子，若sendto直接完成了，后面的wait for等待hEvent时，hEvent应该
// 是singled状态，所以，应该手动set event一下。不过，手动Set和Reset event不是必须的操作，
// 因为我们调用WSASendto和WSARecvfrom的时候，系统会帮我们设置正确的状态。
bool UdpOverlappedEventTest();