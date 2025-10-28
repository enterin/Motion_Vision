#pragma once
#include <afxmt.h> // For CCriticalSection, CString, etc. (should be included via pch.h -> framework.h)
// #include <opencv2/core.hpp> // REMOVE - Should be included via pch.h
// #include <nlohmann/json.hpp> // REMOVE - Should be included via pch.h
#include <vector> // Keep standard headers if needed specifically here, though likely in pch.h

// Forward declare Pylon types if full definition not needed, otherwise ensure pch.h includes them
namespace Pylon {
    class CInstantCamera;
}
// Forward declare OpenCV types
namespace cv {
    class Mat;
    class BackgroundSubtractorMOG2; // Forward declare if Ptr<> is used
    template<typename _Tp> class Ptr; // Forward declare Ptr template
}

#define MAX_CAMERAS 4

// App <-> View <-> Threads �޽���
#define WM_UPDATE_FRAME         (WM_USER + 101) // (WPARAM) camIndex, (LPARAM) cv::Mat*
#define WM_INSPECTION_RESULT    (WM_USER + 102) // (WPARAM) 0, (LPARAM) InspectionResult*
#define WM_CAMERA_STATUS        (WM_USER + 103) // (WPARAM) camIndex, (LPARAM) CString*
#define WM_CAMERA_DISCONNECTED  (WM_USER + 104) // (WPARAM) camIndex
#define WM_TCP_STATUS           (WM_USER + 105) // (WPARAM) camIndex, (LPARAM) BOOL

// DB ��Ű���� ���εǴ� JSON ��� �׸�
struct DefectItem {
    CString sCameraType;    // items[].cam    -> 'top' | 'side'
    CString sDefectType;    // items[].type   -> '�Ѳ�����' ��
    CString sDefectValue;   // items[].value  -> '����' | '�ҷ�'
    float   fConfidence;    // items[].conf
    CString sAdditionalInfo;// items[].info
};

struct InspectionResult {
    int     nCameraIndex{};     // Ŭ���̾�Ʈ ����
    CString sProduct;            // product        (ǰ���ȣ ��)
    CString sFinalResult;        // final          ('����'|'�ҷ�')
    CString sTime;               // time           (ISO string)
    CString sDefectSummary;      // summary
    DWORD   dwTimestamp{};      // ���� ��Ī��
    std::vector<DefectItem> vecDefects; // items[]
};

// ī�޶� ����
struct CameraConfig {
    int nIndex{};
    CString sFriendlyName;
    CString sSerial;
    CString sIp;
    int nPort{};
    int nMotionThreshold{ 5000 };
    BOOL bMotionEnabled{ TRUE };
};

// ������ �Ķ����
struct GrabThreadParams {
    int nCameraIndex{};
    HWND hNotifyWnd{};
    CCriticalSection* pCommLock{};
    Pylon::CInstantCamera* pCamera{}; // Pointer, so forward declaration is okay
    void* pCommunicator{}; // CTcpCommunicator* (Forward declare or include CTcpCommunicator.h *after* this struct if needed by value)
    // Use void* or forward declare cv::BackgroundSubtractorMOG2 if definition isn't required here
    // cv::Ptr<cv::BackgroundSubtractorMOG2> needs the full definition usually.
    // Let's assume void* is sufficient for the struct definition itself.
    void* pDetector{};     // cv::BackgroundSubtractorMOG2* or cv::Ptr<cv::BackgroundSubtractorMOG2>*
    CameraConfig config;
};

// �ҷ� ����
struct DefectData {
    cv::Mat matImage; // Needs full cv::Mat definition (via pch.h)
    InspectionResult result;
};
