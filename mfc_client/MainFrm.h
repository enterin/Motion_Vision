#pragma once
#include "FactoryVisionClientView.h"
#include "CCameraSetupDlg.h"
#include "CCameraManager.h"
#include "SharedData.h"
#include "CLivePanel.h"
#include <vector>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define ID_CAM_BTN_BASE         (WM_USER + 200)
#define ID_CAM_BTN_1            (ID_CAM_BTN_BASE + 0)
#define ID_CAM_BTN_2            (ID_CAM_BTN_BASE + 1)
#define ID_CAM_BTN_3            (ID_CAM_BTN_BASE + 2)
#define ID_CAM_BTN_4            (ID_CAM_BTN_BASE + 3)
#define ID_MANUAL_CAPTURE_BTN   (WM_USER + 210)
#define ID_SETTINGS_BTN         (WM_USER + 211)
#define ID_VIEW_TOGGLE_LIVEPANEL (WM_USER + 212)

class CMainFrame : public CFrameWnd
{
public:
    CMainFrame() noexcept;
protected:
    DECLARE_DYNCREATE(CMainFrame)

public:
    CCameraManager* GetCameraManager() { return &m_CameraManager; }
    std::vector<CameraConfig>& GetCamConfigs() { return m_CamConfigs; }

    void LoadConfigs();
    void SaveConfigs();
    void UpdateCameraButtons();

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CStatusBar        m_wndStatusBar;
    CFont             m_FontUI;
    CButton           m_btnSettings;
    CButton           m_btnManualCapture;
    CButton           m_btnTogglePanel;
    std::vector<CButton*> m_vecCamButtons;

    CFactoryVisionClientView* m_pView{};

    CCameraManager    m_CameraManager;
    std::vector<CameraConfig> m_CamConfigs;

    CLivePanel        m_LivePanel;

    void CreateDynamicButtonsLayout();
    void UpdateLayout(int cx, int cy);

    // ini 경로
    CString GetIniPath() const
    {
        TCHAR path[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, path, MAX_PATH);
        PathRemoveFileSpec(path);
        PathAppend(path, _T("config.ini"));
        return path;
    }

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSettingsClick();
    afx_msg void OnManualCaptureClick();
    afx_msg void OnCameraViewClick(UINT nID);
    afx_msg LRESULT OnTcpStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCameraDisconnected(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCameraStatus(WPARAM wParam, LPARAM lParam);

    // 패널 제어
    afx_msg void OnTogglePanel();
    afx_msg void OnDockLeft();
    afx_msg void OnDockRight();
    afx_msg void OnFloatPanel();
    afx_msg void OnHidePanel();

    DECLARE_MESSAGE_MAP()
};
