#pragma once

// ʹ�û���event��overlapped socketģ�͡�
// 1��overlapped socket��һ��ģ����ʹ��WSASocket��WSASendto��ϵ�к�����
// 2���շ����ݿ���ֱ�ӳɹ���Ҳ�п��ܽ���IO_PENDING�ȴ��շ�������ɣ�������
// io pending����Ҫwait overlapped.hEvent�¼���ϵͳ���ڲ�����ɺ�set event
// ֪ͨ���ǣ����յ�֪ͨ����ʹ��WSAGetOverlappedResult��ȡ���������
// 3����һ��APIȷʵ�Ƚϸ��ӣ��շ�--wait--get result��ÿһ����Ҫ�жϳɹ�/ʧ��/������
// 4��ע�⵽�������ڴ������м����ط������� WSASetEvent�� WSAResetEvent ��λop.hEvent��
// ���Ҳ�ܺ���⣬�ٸ����ӣ���sendtoֱ������ˣ������wait for�ȴ�hEventʱ��hEventӦ��
// ��singled״̬�����ԣ�Ӧ���ֶ�set eventһ�¡��������ֶ�Set��Reset event���Ǳ���Ĳ�����
// ��Ϊ���ǵ���WSASendto��WSARecvfrom��ʱ��ϵͳ�������������ȷ��״̬��
bool UdpOverlappedEventTest();