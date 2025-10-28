#pragma once
#include "CCameraManager.h"
#include "CLivePanel.h" // CLivePanel 헤더 포함
#include "SharedData.h" // CameraConfig 사용 위해 추가
#include <vector> // std::vector 사용 위해 추가

class CMainFrame : public CFrameWnd
{
    DECLARE_DYNCREATE(CMainFrame)
protected:
    CMainFrame() noexcept;

public:
    virtual ~CMainFrame();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // --- 수정된 부분 (멤버 함수 추가) ---
    CCameraManager* GetCameraManager() { return &m_CameraManager; }
    CLivePanel* GetLivePanel() { return &m_LivePanel; } // GetLivePanel 함수 추가
    BOOL LoadCameraConfigs(std::vector<CameraConfig>& out); // 추가
    BOOL SaveCameraConfigs(const std::vector<CameraConfig>& cfgs); // 추가
    // --- 수정 끝 ---

protected:
    CStatusBar m_wndStatusBar;
    class CFactoryVisionClientView* m_pView = nullptr;
    CCameraManager m_CameraManager;
    std::vector<CameraConfig> m_CamConfigs;
    CLivePanel m_LivePanel; // m_LivePanel 멤버 변수
    CFont m_FontUI;
    CButton m_btnTogglePanel, m_btnManualCapture, m_btnSettings;
    std::vector<CButton*> m_vecCamButtons;

    void UpdateLayout(int cx, int cy);
    void CreateDynamicButtonsLayout();
    void UpdateCameraButtons();
    void LoadConfigs();
    void SaveConfigs();
    CString GetIniPath() const;

    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnTcpStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCameraDisconnected(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCameraStatus(WPARAM wParam, LPARAM lParam);
    afx_msg void OnCameraViewClick(UINT nID);
    afx_msg void OnManualCaptureClick();
    afx_msg void OnSettingsClick();
    afx_msg void OnTogglePanel();
    afx_msg void OnDockLeft();
    afx_msg void OnDockRight();
    afx_msg void OnFloatPanel();
    afx_msg void OnHidePanel();
};