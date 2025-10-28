#pragma once
#include "pch.h" // Should include everything needed (winsock, queue, mutex, json)
// #include <queue> // Already in pch.h
// #include <mutex> // Already in pch.h
// #include <nlohmann/json.hpp> // Already in pch.h

class CTcpCommunicator {
public:
    CTcpCommunicator();
    ~CTcpCommunicator();

    // HWND is defined in windows.h (via framework.h -> pch.h)
    // CString is defined in afx.h (via framework.h -> pch.h)
    BOOL Connect(const CString& ip, int port, HWND hNotify, int camIdx);
    void Disconnect();

    // SOCKET, INVALID_SOCKET defined in winsock2.h (via pch.h)
    BOOL IsConnected() const { return m_sock != INVALID_SOCKET; }

    // CCriticalSection defined in afxmt.h (via pch.h)
    CCriticalSection* GetLock() { return &m_csSend; }

    // DWORD defined in windows.h (via framework.h -> pch.h)
    bool SendImage(const void* pData, int size, DWORD clientTs);

    // nlohmann::json is defined in json.hpp (via pch.h)
    bool TryPopResult(nlohmann::json& out);

private:
    SOCKET m_sock{ INVALID_SOCKET };
    HANDLE m_hRecvThread{ nullptr }; // HANDLE defined in windows.h
    BOOL   m_bStop{ FALSE }; // BOOL defined in windows.h
    HWND   m_hNotify{ nullptr };
    int    m_camIdx{ -1 };

    CCriticalSection m_csSend;

    // std::mutex, std::queue, nlohmann::json included via pch.h
    std::mutex m_mq;
    std::queue<nlohmann::json> m_qResults;

    // UINT, LPVOID defined in windows.h
    // __cdecl defined by compiler
    static UINT __cdecl RecvThread(LPVOID pParam);

    // Internal utils
    bool SendAll(const char* buf, int len);
    int  RecvAll(char* buf, int len);
};
