#pragma once
#include <afxmt.h>
#include <opencv2/core.hpp>
#include <nlohmann/json.hpp>

#define MAX_CAMERAS 4

// App <-> View <-> Threads 메시지
#define WM_UPDATE_FRAME         (WM_USER + 101) // (WPARAM) camIndex, (LPARAM) cv::Mat*
#define WM_INSPECTION_RESULT    (WM_USER + 102) // (WPARAM) 0, (LPARAM) InspectionResult*
#define WM_CAMERA_STATUS        (WM_USER + 103) // (WPARAM) camIndex, (LPARAM) CString*
#define WM_CAMERA_DISCONNECTED  (WM_USER + 104) // (WPARAM) camIndex
#define WM_TCP_STATUS           (WM_USER + 105) // (WPARAM) camIndex, (LPARAM) BOOL

// DB 스키마와 매핑되는 JSON 결과 항목
struct DefectItem {
    CString sCameraType;    // items[].cam    -> 'top' | 'side'
    CString sDefectType;    // items[].type   -> '뚜껑유무' 등
    CString sDefectValue;   // items[].value  -> '정상' | '불량'
    float   fConfidence;    // items[].conf
    CString sAdditionalInfo;// items[].info
};

struct InspectionResult {
    int     nCameraIndex{};     // 클라이언트 기준
    CString sProduct;            // product        (품목번호 등)
    CString sFinalResult;        // final          ('정상'|'불량')
    CString sTime;               // time           (ISO string)
    CString sDefectSummary;      // summary
    DWORD   dwTimestamp{};      // 로컬 매칭용
    std::vector<DefectItem> vecDefects; // items[]
};

// 카메라 설정
struct CameraConfig {
    int nIndex{};
    CString sFriendlyName;
    CString sSerial;
    CString sIp;
    int nPort{};
    int nMotionThreshold{ 5000 };
    BOOL bMotionEnabled{ TRUE };
};

// 스레드 파라미터
struct GrabThreadParams {
    int nCameraIndex{};
    HWND hNotifyWnd{};
    CCriticalSection* pCommLock{};
    Pylon::CInstantCamera* pCamera{};
    void* pCommunicator{}; // CTcpCommunicator*
    void* pDetector{};     // cv::BackgroundSubtractorMOG2*
    CameraConfig config;
};

// 불량 저장
struct DefectData {
    cv::Mat matImage;
    InspectionResult result;
};
