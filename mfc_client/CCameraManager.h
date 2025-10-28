#pragma once
// #include <pylon/PylonIncludes.h> // Already in pch.h
// #include <pylon/gige/BaslerGigEInstantCamera.h> // Already in pch.h
// #include <opencv2/opencv.hpp> // Already in pch.h
// #include "SharedData.h" // Already in pch.h
#include "CTcpCommunicator.h" // Needs definition before use

// Forward declaration if needed, but include preferred if definition required
namespace Pylon {
    class CDeviceInfo; // Forward declare if full definition not needed here
}

class CCameraManager {
public:
    CCameraManager();
    ~CCameraManager();

    void FindDevices(std::vector<Pylon::CDeviceInfo>& out);
    BOOL ConnectCamera(const CameraConfig& cfg, HWND hNotifyWnd);
    void DisconnectCamera(int idx);
    void DisconnectAll();
    BOOL IsCameraConnected(int idx);
    void TriggerManualCapture(int idx);
    void UpdateMotionSettings(int idx, BOOL enable, int threshold);

private:
    Pylon::PylonAutoInitTerm m_pylonAutoInitTerm; // Semicolon was likely okay here
    Pylon::CInstantCamera m_Cameras[MAX_CAMERAS]; // Semicolon was likely okay here
    CTcpCommunicator m_Comms[MAX_CAMERAS]; // Semicolon was likely okay here

    // Make sure GrabThreadParams is fully defined before this point (moved to SharedData.h / pch.h)
public: // GrabThread needs access
    HANDLE m_hGrabThreads[MAX_CAMERAS]; // Semicolon was likely okay here
    BOOL   m_bThreadStopFlags[MAX_CAMERAS]; // Semicolon was likely okay here
    GrabThreadParams m_ThreadParams[MAX_CAMERAS]; // Semicolon was likely okay here
    cv::Ptr<cv::BackgroundSubtractorMOG2> m_Detectors[MAX_CAMERAS]; // Semicolon was likely okay here
    BOOL   m_bManualTrigger[MAX_CAMERAS]; // Semicolon was likely okay here

    static UINT __cdecl GrabThread(LPVOID pParam); // Semicolon was likely okay here
}; // Ensure this semicolon is present
