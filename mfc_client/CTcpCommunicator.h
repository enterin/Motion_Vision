#pragma once
#include "pch.h"
#include <queue>
#include <mutex>
#include <nlohmann/json.hpp>

class CTcpCommunicator {
public:
    CTcpCommunicator();
    ~CTcpCommunicator();

    BOOL Connect(const CString& ip, int port, HWND hNotify, int camIdx);
    void Disconnect();
    BOOL IsConnected() const { return m_sock != INVALID_SOCKET; }

    // UI/GrabThread���� ȣ��
    CCriticalSection* GetLock() { return &m_csSend; }

    // IMGE ��Ŷ ���� (��� + �ٵ�)
    bool SendImage(const void* pData, int size, DWORD clientTs);

    // �������� �� JSON ����� pop
    bool TryPopResult(nlohmann::json& out);

private:
    SOCKET m_sock{ INVALID_SOCKET };
    HANDLE m_hRecvThread{ nullptr };
    BOOL   m_bStop{ FALSE };
    HWND   m_hNotify{ nullptr };
    int    m_camIdx{ -1 };

    CCriticalSection m_csSend;

    // ��� ť
    std::mutex m_mq;
    std::queue<nlohmann::json> m_qResults;

    static UINT __cdecl RecvThread(LPVOID pParam);

    // ���� ��ƿ
    bool SendAll(const char* buf, int len);
    int  RecvAll(char* buf, int len);
};
