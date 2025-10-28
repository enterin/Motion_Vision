#include "pch.h"
#include "CTcpCommunicator.h"
#include "SharedData.h"

CTcpCommunicator::CTcpCommunicator() {}
CTcpCommunicator::~CTcpCommunicator() { Disconnect(); }

BOOL CTcpCommunicator::Connect(const CString& ip, int port, HWND hNotify, int camIdx) {
    Disconnect();
    m_hNotify = hNotify; m_camIdx = camIdx;

    addrinfo hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM; hints.ai_protocol = IPPROTO_TCP;
    addrinfo* res = nullptr;
    char portStr[16]; sprintf_s(portStr, "%d", port);
    if (getaddrinfo(CT2A(ip), portStr, &hints, &res) != 0) return FALSE;

    SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == INVALID_SOCKET) { freeaddrinfo(res); return FALSE; }

    if (::connect(s, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        closesocket(s); freeaddrinfo(res); return FALSE;
    }
    freeaddrinfo(res);

    m_sock = s;
    m_bStop = FALSE;

    // 수신 스레드
    unsigned tid;
    m_hRecvThread = (HANDLE)_beginthreadex(nullptr, 0, RecvThread, this, 0, &tid);

    if (m_hNotify) ::PostMessage(m_hNotify, WM_TCP_STATUS, m_camIdx, TRUE);
    return TRUE;
}

void CTcpCommunicator::Disconnect() {
    m_bStop = TRUE;
    if (m_hRecvThread) {
        WaitForSingleObject(m_hRecvThread, 2000);
        CloseHandle(m_hRecvThread);
        m_hRecvThread = nullptr;
    }
    if (m_sock != INVALID_SOCKET) { closesocket(m_sock); m_sock = INVALID_SOCKET; }
    if (m_hNotify) ::PostMessage(m_hNotify, WM_TCP_STATUS, m_camIdx, FALSE);
}

bool CTcpCommunicator::SendAll(const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int r = send(m_sock, buf + sent, len - sent, 0);
        if (r <= 0) return false;
        sent += r;
    }
    return true;
}
int CTcpCommunicator::RecvAll(char* buf, int len) {
    int got = 0;
    while (got < len) {
        int r = recv(m_sock, buf + got, len - got, 0);
        if (r <= 0) return r;
        got += r;
    }
    return got;
}

// 패킷: 4바이트 magic "IMGE", 4바이트 bodyLen(LE), 4바이트 timestamp, body(JPEG bytes)
bool CTcpCommunicator::SendImage(const void* pData, int size, DWORD clientTs) {
    if (!IsConnected() || size <= 0) return false;
    const char magic[4] = { 'I','M','G','E' };
    int bodyLen = size + 4; // include timestamp
    std::string buf; buf.resize(4 + 4 + bodyLen);
    memcpy(buf.data(), magic, 4);
    memcpy(buf.data() + 4, &bodyLen, 4);
    memcpy(buf.data() + 8, &clientTs, 4);
    memcpy(buf.data() + 12, pData, size);

    return SendAll(buf.data(), (int)buf.size());
}

// 수신 스레드: 서버가 보낸 "RSLT" 패킷(JSON) 읽어 큐에 push
UINT __cdecl CTcpCommunicator::RecvThread(LPVOID p) {
    auto* self = (CTcpCommunicator*)p;
    SOCKET s = self->m_sock;
    while (!self->m_bStop && s != INVALID_SOCKET) {
        char hdr[8];
        int r = recv(s, hdr, 8, MSG_WAITALL);
        if (r <= 0) break;

        if (memcmp(hdr, "RSLT", 4) != 0) { break; }
        int len = 0; memcpy(&len, hdr + 4, 4);
        if (len <= 0 || len > 10 * 1024 * 1024) break;

        std::string body; body.resize(len);
        int got = self->RecvAll(body.data(), len);
        if (got != len) break;

        try {
            auto js = nlohmann::json::parse(body);
            std::lock_guard<std::mutex> g(self->m_mq);
            self->m_qResults.push(std::move(js));
        }
        catch (...) {
            // ignore parse error
        }
    }
    self->Disconnect(); // notify
    return 0;
}

bool CTcpCommunicator::TryPopResult(nlohmann::json& out) {
    std::lock_guard<std::mutex> g(m_mq);
    if (m_qResults.empty()) return false;
    out = std::move(m_qResults.front());
    m_qResults.pop();
    return true;
}
