#pragma once
#include "afxdialogex.h"
#include "afxcmn.h" // CTabCtrl 사용 위해 추가
#include "SharedData.h" // CameraConfig 사용 위해 추가
#include <vector>
#include <opencv2/opencv.hpp>
#include <afxmt.h> // CCriticalSection, CSingleLock 사용

// 프리뷰 윈도우 클래스 (헤더로 이동 또는 별도 파일 분리)
class CPreviewWnd : public CWnd
{
public:
    int CamIndex = -1;
    cv::Mat LastFrame;
    CCriticalSection Lock;
    ULONGLONG LastDrawTick = 0;

    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
};


// 플로팅 프레임 클래스 전방 선언
class CFloatLiveFrame;

class CLivePanel : public CDialogEx
{
    DECLARE_DYNAMIC(CLivePanel)

public:
    // --- 수정된 부분 (생성자 시그니처 변경) ---
    CLivePanel(CWnd* pParent = nullptr);   // 표준 생성자입니다.
    // --- 수정 끝 ---
    virtual ~CLivePanel();

    // 도킹 상태
    enum DockState { Hidden, DockLeft, DockRight, Floating };

    // --- 수정된 부분 (멤버 변수 및 함수 선언 추가/수정) ---
    DockState m_state = Hidden;
    int m_dockWidth = 380;
    CRect m_floatRect = CRect(100, 100, 460, 620);
    int m_previewFpsCap = 12;
    int m_jpegQuality = 92;
    void ApplyConfig(int dockWidth, DockState state, CRect floatRect, int fpsCap, int jpegQuality); // 추가
    // --- 수정 끝 ---

    void BuildTabs(const std::vector<CameraConfig>& cfgs);
    void UpdateLayout();
    void OnNewFrame(int camIndex, const cv::Mat& frame);

    void DockLeftPane();
    void DockRightPane();
    void FloatPane();
    void HidePane();
    void TogglePane();

    int GetSelectedCamIndex() const;
    BOOL SaveSnapshotSelected(CString* outPath = nullptr);
    void OpenCaptureFolder();


#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_LIVE_PANEL }; // IDD_LIVE_PANEL 확인 필요
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다. (추가)
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult); // 추가
    virtual int OnCreate(LPCREATESTRUCT lpCreateStruct); // 추가
    virtual void OnDestroy(); // 추가

    // --- 수정된 부분 (멤버 변수 및 함수 선언 추가) ---
    CTabCtrl m_Tab;
    std::vector<CPreviewWnd*> m_Previews;
    CFloatLiveFrame* m_pFloatFrame = nullptr; // 플로팅 프레임 포인터

    bool m_resizing = false;
    int m_resizeGrabX = 0;
    int m_resizeStartW = 0;
    static const int RESIZE_GRIP = 5;

    void ShowTabContextMenu(CPoint ptScreen);
    void ChangeParent(CWnd* pNewParent); // 추가
    void PrepareChangingParent(); // 추가
    // --- 수정 끝 ---


    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    // --- 추가된 부분 (탭 변경 메시지 핸들러 선언) ---
    afx_msg void OnTcnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
    // --- 추가 끝 ---
};