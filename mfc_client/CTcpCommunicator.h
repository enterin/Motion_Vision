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

    // UI/GrabThread에서 호출
    CCriticalSection* GetLock() { return &m_csSend; }

    // IMGE 패킷 전송 (헤더 + 바디)
    bool SendImage(const void* pData, int size, DWORD clientTs);

    // 서버에서 온 JSON 결과를 pop
    bool TryPopResult(nlohmann::json& out);

private:
    SOCKET m_sock{ INVALID_SOCKET };
    HANDLE m_hRecvThread{ nullptr };
    BOOL   m_bStop{ FALSE };
    HWND   m_hNotify{ nullptr };
    int    m_camIdx{ -1 };

    CCriticalSection m_csSend;

    // 결과 큐
    std::mutex m_mq;
    std::queue<nlohmann::json> m_qResults;

    static UINT __cdecl RecvThread(LPVOID pParam);

    // 내부 유틸
    bool SendAll(const char* buf, int len);
    int  RecvAll(char* buf, int len);
};
