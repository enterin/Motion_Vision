#pragma once
#include "pch.h" // Should include afxcmn.h, opencv.hpp, SharedData.h
// #include <afxcmn.h> // Included via framework.h -> pch.h
#include <vector> // Already in pch.h
#include <filesystem> // Requires C++17 or later
// #include <opencv2/opencv.hpp> // Already in pch.h
// #include "SharedData.h" // Already in pch.h

// Forward declaration if CMainFrame definition isn't needed here
class CMainFrame;

class CPreviewWnd : public CWnd
{
public:
    int CamIndex{ -1 };
    CCriticalSection Lock; // Included via pch.h
    cv::Mat LastFrame; // Included via pch.h
    ULONGLONG LastDrawTick{ 0 }; // ULONGLONG from windows.h (via pch.h)

    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint(); // afx_msg defined in afxwin.h (via pch.h)
};
// Removed BEGIN/END_MESSAGE_MAP from header, should be in .cpp

// CDialogEx defined in afxdialogex.h (via pch.h)
class CLivePanel : public CDialogEx
{
public:
    enum DockState { DockLeft, DockRight, Floating, Hidden };

    CLivePanel(); // Use default constructor IDD_LIVE_PANEL
    virtual ~CLivePanel();

    // State values (config.ini save/restore targets)
    DockState m_state{ DockLeft };
    int       m_dockWidth{ 380 };
    CRect     m_floatRect{ 100,100,460,620 }; // CRect from afxwin.h
    int       m_previewFpsCap{ 12 };     // FPS cap
    int       m_jpegQuality{ 92 };       // Snapshot JPEG quality

    // Creation/Build/Update
    // CWnd defined in afxwin.h, CameraConfig in SharedData.h (via pch.h)
    BOOL CreateModeless(CWnd* pParent);
    void BuildTabs(const std::vector<CameraConfig>& cfgs);
    void UpdateLayout();
    // cv::Mat defined in opencv.hpp (via pch.h)
    void OnNewFrame(int camIndex, const cv::Mat& frame);

    // Docking/Floating control
    void DockLeftPane();
    void DockRightPane();
    void FloatPane();
    void HidePane();
    void TogglePane();

    // Context menu
    // CPoint defined in afxwin.h
    void ShowTabContextMenu(CPoint ptScreen);
    // CString defined in afxwin.h
    BOOL SaveSnapshotSelected(CString* outPath = nullptr);
    void OpenCaptureFolder();

    // Config helper
    void ApplyConfig(int dockWidth, DockState st, const CRect& fr, int fpsCap, int jpgQ);

    // Resizer (width adjustment)
    bool m_resizing{ false };
    int  m_resizeGrabX{ 0 };
    int  m_resizeStartW{ 0 };
    const int RESIZE_GRIP = 6; // Edge 6px

    // Tab/Preview
    CTabCtrl m_Tab; // afxcmn.h (via pch.h)
    std::vector<CPreviewWnd*> m_Previews;

    // Floating creation cache
    bool m_floatingCreated{ false };

    // Previous save dir (expand if needed)
    CString m_snapDir;

    // Messages
    DECLARE_MESSAGE_MAP()
    // LPCREATESTRUCT from windows.h
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual BOOL OnInitDialog();
    // UINT from windows.h
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    // WPARAM, LPARAM, LRESULT from windows.h
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    // Selected tab's CAM index
    int GetSelectedCamIndex() const;
};
