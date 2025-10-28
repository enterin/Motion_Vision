#include "pch.h"
#include "CLivePanel.h"
#include "resource.h"
#include "MainFrm.h"
#include "FactoryVisionClientView.h" // CFactoryVisionClientView 사용 위해 추가
#include <Shellapi.h>
#include <Shlwapi.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include <opencv2/imgcodecs.hpp>

#pragma comment(lib, "Shlwapi.lib")

#if _MSVC_LANG >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1914)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <direct.h>
#endif

// 플로팅 프레임 클래스 정의
class CFloatLiveFrame : public CFrameWnd
{
public:
    CLivePanel* m_pPanel = nullptr;
    virtual void PostNcDestroy() { delete this; }
    DECLARE_MESSAGE_MAP()
    afx_msg void OnClose();
    afx_msg void OnMove(int x, int y);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

BEGIN_MESSAGE_MAP(CFloatLiveFrame, CFrameWnd)
    ON_WM_CLOSE()
    ON_WM_MOVE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

void CFloatLiveFrame::OnClose()
{
    if (m_pPanel)
    {
        m_pPanel->ShowWindow(SW_HIDE);
        m_pPanel->m_state = CLivePanel::Hidden;
        if (CWnd* pMain = AfxGetMainWnd())
        {
            pMain->SendMessage(WM_SIZE);
        }
    }
    // CFrameWnd::OnClose(); // Prevent actual destruction
}

void CFloatLiveFrame::OnMove(int x, int y)
{
    CFrameWnd::OnMove(x, y);
    if (m_pPanel && ::IsWindow(m_pPanel->GetSafeHwnd())) {
        WINDOWPLACEMENT wp;
        GetWindowPlacement(&wp);
        if (wp.showCmd != SW_SHOWMINIMIZED && wp.showCmd != SW_SHOWMAXIMIZED)
            m_pPanel->m_floatRect = wp.rcNormalPosition;
    }
}

void CFloatLiveFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    if (m_pPanel && ::IsWindow(m_pPanel->GetSafeHwnd())) {
        m_pPanel->MoveWindow(0, 0, cx, cy);
        m_pPanel->UpdateLayout();
        if (nType != SIZE_MINIMIZED && nType != SIZE_MAXIMIZED) {
            WINDOWPLACEMENT wp;
            GetWindowPlacement(&wp);
            m_pPanel->m_floatRect = wp.rcNormalPosition;
        }
    }
}


BEGIN_MESSAGE_MAP(CLivePanel, CDialogEx)
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_LIVE, &CLivePanel::OnTcnSelChange) // IDC_TAB_LIVE 확인 필요
END_MESSAGE_MAP()

CLivePanel::CLivePanel(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_LIVE_PANEL, pParent), m_pFloatFrame(nullptr), m_state(Hidden), m_dockWidth(380), m_resizing(false), m_resizeGrabX(0), m_resizeStartW(0), m_previewFpsCap(12), m_jpegQuality(92)
{
    m_floatRect = CRect(100, 100, 460, 620);
}

CLivePanel::~CLivePanel()
{
    for (auto* p : m_Previews) delete p;
    m_Previews.clear();
    if (m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
    {
        m_pFloatFrame->DestroyWindow();
    }
}

int CLivePanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1) return -1;
    if (!m_Tab.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | TCS_TOOLTIPS,
        CRect(0, 0, 10, 10), this, IDC_TAB_LIVE)) // IDC_TAB_LIVE 확인 필요
    {
        TRACE0("Failed to create tab control\n");
        return -1;
    }
    return 0;
}

void CLivePanel::OnDestroy()
{
    if (m_state == Floating && m_pFloatFrame && m_pFloatFrame->GetSafeHwnd()) {
        WINDOWPLACEMENT wp;
        m_pFloatFrame->GetWindowPlacement(&wp);
        if (wp.showCmd != SW_SHOWMINIMIZED && wp.showCmd != SW_SHOWMAXIMIZED)
            m_floatRect = wp.rcNormalPosition;
    }
    CDialogEx::OnDestroy();
}

// CreateModeless 함수 제거 (대신 CDialogEx::Create 사용)

void CLivePanel::BuildTabs(const std::vector<CameraConfig>& cfgs)
{
    if (!m_Tab.GetSafeHwnd())
        return;

    for (auto* p : m_Previews) { if (p && p->GetSafeHwnd()) p->DestroyWindow(); delete p; }
    m_Previews.clear(); m_Tab.DeleteAllItems();

    for (size_t i = 0; i < cfgs.size(); ++i)
    {
        CString t; t.Format(_T("CAM %d"), cfgs[i].nIndex + 1);
        TCITEM ti{}; ti.mask = TCIF_TEXT; ti.pszText = t.GetBuffer();
        m_Tab.InsertItem((int)i, &ti);
        t.ReleaseBuffer();

        auto* pv = new CPreviewWnd();
        pv->CamIndex = cfgs[i].nIndex;
        CRect rc(0, 0, 10, 10);
        if (!pv->Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
            _T("Preview"), WS_CHILD | WS_CLIPSIBLINGS, rc, &m_Tab, 0))
        {
            TRACE1("Failed to create preview window for CAM %d\n", cfgs[i].nIndex + 1);
            delete pv;
            continue;
        }
        m_Previews.push_back(pv);
    }
    UpdateLayout();
    if (m_Tab.GetItemCount() > 0)
    {
        m_Tab.SetCurSel(0);
        OnTcnSelChange(nullptr, nullptr);
    }
}

void CLivePanel::UpdateLayout()
{
    if (!GetSafeHwnd() || !m_Tab.GetSafeHwnd()) return;

    CRect rcClient;
    if (m_state == Floating && m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
        m_pFloatFrame->GetClientRect(&rcClient);
    else
        GetClientRect(&rcClient);

    CRect rcTab = rcClient;
    rcTab.DeflateRect(6, 6);
    if (::IsWindow(m_Tab.GetSafeHwnd()))
        m_Tab.MoveWindow(&rcTab);

    CRect rcPane;
    if (::IsWindow(m_Tab.GetSafeHwnd())) {
        m_Tab.GetClientRect(&rcPane);
        m_Tab.AdjustRect(FALSE, &rcPane);

        int currentTab = m_Tab.GetCurSel();
        for (size_t i = 0; i < m_Previews.size(); ++i) {
            if (m_Previews[i] && m_Previews[i]->GetSafeHwnd()) {
                m_Previews[i]->MoveWindow(&rcPane);
                m_Previews[i]->ShowWindow(static_cast<int>(i) == currentTab ? SW_SHOW : SW_HIDE);
            }
        }
    }
}

void CLivePanel::OnNewFrame(int camIndex, const cv::Mat& frame)
{
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG minGap = (m_previewFpsCap > 0) ? (1000ULL / (ULONGLONG)m_previewFpsCap) : 0ULL;

    for (auto* pv : m_Previews)
    {
        if (pv && pv->CamIndex == camIndex && pv->GetSafeHwnd())
        {
            { CSingleLock lk(&pv->Lock, TRUE); pv->LastFrame = frame.clone(); }

            if (minGap == 0 || now - pv->LastDrawTick >= minGap)
            {
                pv->LastDrawTick = now;
                pv->Invalidate(FALSE);
            }
        }
    }
}

void CLivePanel::ChangeParent(CWnd* pNewParent)
{
    if (!GetSafeHwnd()) return;

    CWnd* pCurrentParent = GetParent();
    if (pNewParent == pCurrentParent) return;

    PrepareChangingParent();

    if (pNewParent)
    {
        ModifyStyle(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU, WS_CHILD);
        SetParent(pNewParent);
        ModifyStyleEx(WS_EX_TOOLWINDOW | WS_EX_APPWINDOW, 0);
        SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowWindow(SW_SHOW);
    }
    else
    {
        ModifyStyle(WS_CHILD, WS_POPUP);
        ModifyStyleEx(0, WS_EX_TOOLWINDOW);
        SetParent(nullptr);
    }
}

void CLivePanel::DockLeftPane()
{
    CWnd* pMainFrame = AfxGetMainWnd();
    if (!pMainFrame) return;

    if (m_state == Floating && m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
    {
        PrepareChangingParent();
        m_pFloatFrame->DestroyWindow();
        m_pFloatFrame = nullptr;
    }

    if (!GetSafeHwnd())
    {
        if (!Create(IDD_LIVE_PANEL, pMainFrame)) return; // Create 사용
    }
    else
    {
        ChangeParent(pMainFrame);
    }

    m_state = DockLeft;
    pMainFrame->SendMessage(WM_SIZE);
    ShowWindow(SW_SHOW);
}

void CLivePanel::DockRightPane()
{
    CWnd* pMainFrame = AfxGetMainWnd();
    if (!pMainFrame) return;

    if (m_state == Floating && m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
    {
        PrepareChangingParent();
        m_pFloatFrame->DestroyWindow();
        m_pFloatFrame = nullptr;
    }

    if (!GetSafeHwnd())
    {
        if (!Create(IDD_LIVE_PANEL, pMainFrame)) return; // Create 사용
    }
    else
    {
        ChangeParent(pMainFrame);
    }

    m_state = DockRight;
    pMainFrame->SendMessage(WM_SIZE);
    ShowWindow(SW_SHOW);
}

void CLivePanel::FloatPane()
{
    if (m_state == Floating)
    {
        if (m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
            m_pFloatFrame->BringWindowToTop();
        return;
    }

    m_pFloatFrame = new CFloatLiveFrame();
    m_pFloatFrame->m_pPanel = this;
    if (!m_pFloatFrame->Create(nullptr, _T("Live Panel"), WS_OVERLAPPEDWINDOW, m_floatRect, nullptr))
    {
        delete m_pFloatFrame;
        m_pFloatFrame = nullptr;
        AfxMessageBox(_T("Failed to create floating window"));
        return;
    }

    if (GetSafeHwnd()) {
        ChangeParent(m_pFloatFrame);
        SetWindowPos(nullptr, 0, 0, m_floatRect.Width(), m_floatRect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else {
        if (!Create(IDD_LIVE_PANEL, m_pFloatFrame)) // Create 사용
        {
            m_pFloatFrame->DestroyWindow();
            m_pFloatFrame = nullptr;
            AfxMessageBox(_T("Failed to create panel"));
            return;
        }
        SetWindowPos(nullptr, 0, 0, m_floatRect.Width(), m_floatRect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    m_pFloatFrame->ShowWindow(SW_SHOW);
    m_pFloatFrame->UpdateWindow();

    m_state = Floating;
    if (CWnd* pMain = AfxGetMainWnd()) pMain->SendMessage(WM_SIZE);
    UpdateLayout();
}

void CLivePanel::HidePane()
{
    if (m_state == Floating && m_pFloatFrame && m_pFloatFrame->GetSafeHwnd())
    {
        m_pFloatFrame->ShowWindow(SW_HIDE);
    }
    else if (GetSafeHwnd())
    {
        ShowWindow(SW_HIDE);
    }
    m_state = Hidden;
    CWnd* pParentWnd = GetParent();
    if (pParentWnd) pParentWnd->SendMessage(WM_SIZE);
}

void CLivePanel::TogglePane()
{
    if (m_state == Hidden)
    {
        DockLeftPane();
    }
    else HidePane();
}

BOOL CLivePanel::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR p = (LPNMHDR)lParam;
    if (p != nullptr && p->hwndFrom == m_Tab.m_hWnd)
    {
        if (p->code == TCN_SELCHANGE)
        {
            OnTcnSelChange(p, pResult);
        }
        else if (p->code == NM_RCLICK)
        {
            CPoint pt; GetCursorPos(&pt);
            ShowTabContextMenu(pt);
        }
    }
    return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CLivePanel::OnTcnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    int sel = m_Tab.GetCurSel();
    if (sel >= 0 && sel < (int)m_Previews.size())
    {
        for (size_t i = 0; i < m_Previews.size(); ++i) {
            if (m_Previews[i] && m_Previews[i]->GetSafeHwnd()) {
                m_Previews[i]->ShowWindow(static_cast<int>(i) == sel ? SW_SHOW : SW_HIDE);
            }
        }
        if (m_Previews[sel] && m_Previews[sel]->GetSafeHwnd()) {
            m_Previews[sel]->Invalidate(FALSE);
        }

        int cam = m_Previews[sel]->CamIndex;
        if (CWnd* pMain = AfxGetMainWnd())
        {
            auto* pFrm = dynamic_cast<CMainFrame*>(pMain);
            if (pFrm && pFrm->GetSafeHwnd() && pFrm->GetActiveView())
            {
                auto* v = dynamic_cast<CFactoryVisionClientView*>(pFrm->GetActiveView()); // C2027 해결
                if (v) v->SetActiveCamera(cam);
            }
        }
    }

    if (pResult) *pResult = 0;
}

void CLivePanel::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    ShowTabContextMenu(point);
}

void CLivePanel::ShowTabContextMenu(CPoint ptScreen)
{
    CMenu m; m.LoadMenu(IDR_MENU_LIVE_TAB);
    CMenu* popup = m.GetSubMenu(0);
    if (!popup) return;

    popup->CheckMenuItem(ID_VIEW_DOCK_LEFT, MF_BYCOMMAND | (m_state == DockLeft ? MF_CHECKED : MF_UNCHECKED));
    popup->CheckMenuItem(ID_VIEW_DOCK_RIGHT, MF_BYCOMMAND | (m_state == DockRight ? MF_CHECKED : MF_UNCHECKED));
    popup->CheckMenuItem(ID_VIEW_FLOAT, MF_BYCOMMAND | (m_state == Floating ? MF_CHECKED : MF_UNCHECKED));
    popup->EnableMenuItem(ID_VIEW_HIDE, MF_BYCOMMAND | (m_state != Hidden ? MF_ENABLED : MF_GRAYED));

    UINT id = (UINT)popup->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, ptScreen.x, ptScreen.y, this);
    switch (id)
    {
    case ID_LIVE_SNAPSHOT:           SaveSnapshotSelected(); break;
    case ID_LIVE_OPEN_CAPTUREFOLDER: OpenCaptureFolder();    break;
    case ID_VIEW_DOCK_LEFT:          DockLeftPane();         break;
    case ID_VIEW_DOCK_RIGHT:         DockRightPane();        break;
    case ID_VIEW_FLOAT:              FloatPane();            break;
    case ID_VIEW_HIDE:               HidePane();             break;
    default: break;
    }
}

int CLivePanel::GetSelectedCamIndex() const
{
    int sel = m_Tab.GetCurSel();
    if (sel < 0 || sel >= (int)m_Previews.size() || !m_Previews[sel]) return -1;
    return m_Previews[sel]->CamIndex;
}

BOOL CLivePanel::SaveSnapshotSelected(CString* outPath)
{
    int cam = GetSelectedCamIndex();
    if (cam < 0) return FALSE;

    CPreviewWnd* pv = nullptr;
    for (auto* p : m_Previews) if (p && p->CamIndex == cam) { pv = p; break; }
    if (!pv) return FALSE;

    cv::Mat img;
    { CSingleLock lk(&pv->Lock, TRUE); if (!pv->LastFrame.empty()) img = pv->LastFrame.clone(); }
    if (img.empty()) return FALSE;

    TCHAR exe[MAX_PATH]{};
    GetModuleFileName(NULL, exe, MAX_PATH);
    PathRemoveFileSpec(exe);
    PathAppend(exe, _T("captures"));
    CString capDir = exe;

#if _MSVC_LANG >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1914)
    if (!fs::exists((LPCTSTR)capDir)) { fs::create_directories((LPCTSTR)capDir); }
#else
    if (GetFileAttributes(capDir) == INVALID_FILE_ATTRIBUTES) { _tmkdir(capDir); }
#endif

    SYSTEMTIME st; GetLocalTime(&st);
    CString name; name.Format(_T("CAM%d_%04d%02d%02d_%02d%02d%02d.jpg"),
        cam + 1, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    CString full = capDir + _T("\\") + name;

    std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, m_jpegQuality };
    std::string path_std = static_cast<std::string>(ATL::CW2A(full));
    bool ok = cv::imwrite(path_std, img, params);

    if (ok)
    {
        if (outPath) *outPath = full;
        MessageBeep(MB_ICONINFORMATION);
        return TRUE;
    }
    MessageBeep(MB_ICONHAND);
    return FALSE;
}

void CLivePanel::OpenCaptureFolder()
{
    TCHAR exe[MAX_PATH]{};
    GetModuleFileName(NULL, exe, MAX_PATH);
    PathRemoveFileSpec(exe);
    PathAppend(exe, _T("captures"));
    ShellExecute(NULL, _T("open"), exe, NULL, NULL, SW_SHOWNORMAL);
}

void CLivePanel::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    UpdateLayout();
}

// ==== 폭 리사이저(도킹 상태에서만) ====
void CLivePanel::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if ((m_state == DockLeft || m_state == DockRight) && !m_resizing)
    {
        CRect rc; GetClientRect(&rc);
        bool onEdge = false;
        if (m_state == DockLeft)  onEdge = (rc.right - pt.x) <= RESIZE_GRIP;
        if (m_state == DockRight) onEdge = (pt.x - rc.left) <= RESIZE_GRIP;

        if (onEdge)
        {
            m_resizing = true;
            SetCapture();
            m_resizeGrabX = pt.x;
            m_resizeStartW = m_dockWidth;
        }
    }
    if (m_state != Floating)
        CDialogEx::OnLButtonDown(nFlags, pt);
}

void CLivePanel::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_resizing)
    {
        m_resizing = false;
        ReleaseCapture();
        if (CWnd* pMain = AfxGetMainWnd()) pMain->SendMessage(WM_SIZE);
    }
    if (m_state != Floating)
        CDialogEx::OnLButtonUp(nFlags, pt);
}

void CLivePanel::OnMouseMove(UINT nFlags, CPoint pt)
{
    if (m_resizing && (nFlags & MK_LBUTTON))
    {
        int delta = pt.x - m_resizeGrabX;
        if (m_state == DockLeft)  m_dockWidth = std::max(160, m_resizeStartW + delta);
        if (m_state == DockRight) m_dockWidth = std::max(160, m_resizeStartW - delta);
        if (CWnd* pMain = AfxGetMainWnd()) pMain->SendMessage(WM_SIZE);
    }
    if (m_state != Floating)
        CDialogEx::OnMouseMove(nFlags, pt);
}

BOOL CLivePanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if ((m_state == DockLeft || m_state == DockRight) && !m_resizing)
    {
        CPoint pt; GetCursorPos(&pt); ScreenToClient(&pt);
        CRect rc; GetClientRect(&rc);
        bool onEdge = false;
        if (m_state == DockLeft)  onEdge = (rc.right - pt.x) <= RESIZE_GRIP;
        if (m_state == DockRight) onEdge = (pt.x - rc.left) <= RESIZE_GRIP;
        if (onEdge) { ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE)); return TRUE; }
    }
    if (m_state != Floating)
        return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
    else
        return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CLivePanel::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_LIVE, m_Tab); // IDC_TAB_LIVE 확인 필요
}

void CLivePanel::PrepareChangingParent()
{
    if (m_Tab.GetSafeHwnd()) m_Tab.SetParent(nullptr);
    for (auto* pv : m_Previews) {
        if (pv && pv->GetSafeHwnd()) pv->SetParent(nullptr);
    }
}

void CLivePanel::ApplyConfig(int dockWidth, DockState state, CRect floatRect, int fpsCap, int jpegQuality)
{
    m_dockWidth = dockWidth;
    m_state = state;
    m_floatRect = floatRect;
    m_previewFpsCap = fpsCap;
    m_jpegQuality = jpegQuality;
}