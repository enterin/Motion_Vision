#pragma once
#include <afxcmn.h>
#include <vector>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "SharedData.h"

class CPreviewWnd : public CWnd
{
public:
    int CamIndex{ -1 };
    CCriticalSection Lock;
    cv::Mat LastFrame;
    ULONGLONG LastDrawTick{ 0 };

    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint()
    {
        CPaintDC dc(this);
        CRect rc; GetClientRect(&rc);
        CDC mem; mem.CreateCompatibleDC(&dc);
        CBitmap bmp; bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
        auto old = mem.SelectObject(&bmp);
        mem.FillSolidRect(&rc, RGB(20, 20, 22));

        cv::Mat img;
        { CSingleLock lk(&Lock, TRUE); if (!LastFrame.empty()) img = LastFrame.clone(); }
        if (!img.empty() && img.type() == CV_8UC3)
        {
            // 간단한 StretchDIBits 렌더 (BGR)
            BITMAPINFO bi{}; bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = img.cols;
            bi.bmiHeader.biHeight = -img.rows; // top-down
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biCompression = BI_RGB;

            CRect rd = rc;
            // AR 유지
            double rcRatio = (double)rc.Width() / rc.Height();
            double imRatio = (double)img.cols / img.rows;
            if (rcRatio > imRatio) {
                int w = (int)(rc.Height() * imRatio);
                rd.left = rc.left + (rc.Width() - w) / 2;
                rd.right = rd.left + w;
            }
            else {
                int h = (int)(rc.Width() / imRatio);
                rd.top = rc.top + (rc.Height() - h) / 2;
                rd.bottom = rd.top + h;
            }
            StretchDIBits(mem.GetSafeHdc(), rd.left, rd.top, rd.Width(), rd.Height(),
                0, 0, img.cols, img.rows, img.data, &bi, DIB_RGB_COLORS, SRCCOPY);
        }
        dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mem, 0, 0, SRCCOPY);
        mem.SelectObject(old);
    }
};
BEGIN_MESSAGE_MAP(CPreviewWnd, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

// 탭형 라이브뷰 패널 (모델리스 대화상자)
class CLivePanel : public CDialogEx
{
public:
    enum DockState { DockLeft, DockRight, Floating, Hidden };

    CLivePanel() : CDialogEx(IDD_LIVE_PANEL) {}
    virtual ~CLivePanel() {}

    // 상태값 (config.ini 저장/복원 대상)
    DockState m_state{ DockLeft };
    int       m_dockWidth{ 380 };
    CRect     m_floatRect{ 100,100,460,620 };
    int       m_previewFpsCap{ 12 };     // FPS 상한
    int       m_jpegQuality{ 92 };       // 스냅샷 JPEG 품질

    // 생성/빌드/업데이트
    BOOL CreateModeless(CWnd* pParent)
    {
        BOOL ok = Create(IDD_LIVE_PANEL, pParent);
        if (ok) { ShowWindow(SW_SHOW); UpdateLayout(); }
        return ok;
    }
    void BuildTabs(const std::vector<CameraConfig>& cfgs);
    void UpdateLayout();
    void OnNewFrame(int camIndex, const cv::Mat& frame);

    // 도킹/플로팅 제어
    void DockLeftPane();
    void DockRightPane();
    void FloatPane();
    void HidePane();
    void TogglePane();

    // 컨텍스트 메뉴
    void ShowTabContextMenu(CPoint ptScreen);
    BOOL SaveSnapshotSelected(CString* outPath = nullptr);
    void OpenCaptureFolder();

    // 구성값 적용 헬퍼
    void ApplyConfig(int dockWidth, DockState st, const CRect& fr, int fpsCap, int jpgQ)
    {
        if (dockWidth > 120) m_dockWidth = dockWidth;
        m_state = st; m_floatRect = fr;
        if (fpsCap > 0) m_previewFpsCap = fpsCap;
        if (jpgQ >= 1 && jpgQ <= 100) m_jpegQuality = jpgQ;
    }

    // 리사이저(폭 조절)
    bool m_resizing{ false };
    int  m_resizeGrabX{ 0 };
    int  m_resizeStartW{ 0 };
    const int RESIZE_GRIP = 6; // 가장자리 6px

    // 탭/프리뷰
    CTabCtrl m_Tab;
    std::vector<CPreviewWnd*> m_Previews;

    // 플로팅 생성 여부 캐시
    bool m_floatingCreated{ false };

    // 직전 저장용 (필요 시 확장)
    CString m_snapDir;

    // 메시지
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual BOOL OnInitDialog()
    {
        CDialogEx::OnInitDialog();
        m_Tab.SubclassDlgItem(IDC_TAB_LIVE, this);
        return TRUE;
    }
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    // 선택된 탭의 CAM index
    int GetSelectedCamIndex() const;
};
