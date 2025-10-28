#pragma once
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <opencv2/opencv.hpp>
#include "SharedData.h"
#include "CTcpCommunicator.h"

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
    Pylon::PylonAutoInitTerm m_pylonAutoInitTerm;
    Pylon::CInstantCamera m_Cameras[MAX_CAMERAS];
    CTcpCommunicator m_Comms[MAX_CAMERAS];

public: // GrabThread에서 접근하므로 public
    HANDLE m_hGrabThreads[MAX_CAMERAS];
    BOOL   m_bThreadStopFlags[MAX_CAMERAS];
    GrabThreadParams m_ThreadParams[MAX_CAMERAS];
    cv::Ptr<cv::BackgroundSubtractorMOG2> m_Detectors[MAX_CAMERAS];
    BOOL   m_bManualTrigger[MAX_CAMERAS];

    static UINT __cdecl GrabThread(LPVOID pParam);
};
