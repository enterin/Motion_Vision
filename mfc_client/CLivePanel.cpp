#include "pch.h"
#include "CLivePanel.h"
#include "resource.h"
#include "MainFrm.h"
#include <Shellapi.h>
#include <Shlwapi.h>
#include <chrono>

#pragma comment(lib, "Shlwapi.lib")

using std::filesystem::create_directories;
using std::filesystem::exists;

BEGIN_MESSAGE_MAP(CLivePanel, CDialogEx)
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_CREATE()
END_MESSAGE_MAP()

int CLivePanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1) return -1;
    return 0;
}

void CLivePanel::BuildTabs(const std::vector<CameraConfig>& cfgs)
{
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
        pv->Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
            _T("Preview"), WS_CHILD | WS_VISIBLE, rc, this, 0);
        m_Previews.push_back(pv);
    }
    UpdateLayout();
    if (m_Tab.GetItemCount() > 0) m_Tab.SetCurSel(0);
}

void CLivePanel::UpdateLayout()
{
    if (!GetSafeHwnd()) return;

    // 도킹/플로팅 공통 탭 배치
    CRect rc; GetClientRect(&rc);
    CRect tabRc(6, 6, rc.Width() - 6, rc.Height() - 6);
    m_Tab.MoveWindow(&tabRc);
    CRect pane; m_Tab.GetClientRect(&pane);
    m_Tab.AdjustRect(FALSE, &pane);
    pane.OffsetRect(6, 26);
    for (auto* p : m_Previews) if (p && p->GetSafeHwnd()) p->MoveWindow(&pane);
}

void CLivePanel::OnNewFrame(int camIndex, const cv::Mat& frame)
{
    // FPS cap: 미리보기용 Invalidate 간격 제한
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG minGap = (m_previewFpsCap > 0) ? (1000ULL / (ULONGLONG)m_previewFpsCap) : 0ULL;

    for (auto* pv : m_Previews)
    {
        if (pv && pv->CamIndex == camIndex)
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

void CLivePanel::DockLeftPane()
{
    if (m_state == Floating)
    {
        DestroyWindow();
        Create(IDD_LIVE_PANEL, GetParent()); ShowWindow(SW_SHOW);
    }
    m_state = DockLeft;
    if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
}

void CLivePanel::DockRightPane()
{
    if (m_state == Floating)
    {
        DestroyWindow();
        Create(IDD_LIVE_PANEL, GetParent()); ShowWindow(SW_SHOW);
    }
    m_state = DockRight;
    if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
}

void CLivePanel::FloatPane()
{
    if (m_state != Floating)
    {
        if (m_floatRect.IsRectEmpty()) m_floatRect = CRect(100, 100, 460, 620);
        DestroyWindow();

        CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW),
            (HBRUSH)(COLOR_WINDOW + 1), NULL);
        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        BOOL ok = CWnd::CreateEx(WS_EX_APPWINDOW, cls, _T("Live Panel"), style, m_floatRect, NULL, 0);
        if (ok)
        {
            m_Tab.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | TCS_TOOLTIPS,
                CRect(4, 4, m_floatRect.Width() - 4, m_floatRect.Height() - 4), this, IDC_TAB_LIVE);
            for (auto* p : m_Previews) { if (p && p->GetSafeHwnd()) p->DestroyWindow(); }
            for (auto* p : m_Previews)
            {
                if (p) {
                    CRect rc(0, 0, 10, 10);
                    p->Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
                        _T("Preview"), WS_CHILD | WS_VISIBLE, rc, this, 0);
                }
            }
            UpdateLayout();
        }
        m_state = Floating;
        m_floatingCreated = ok;
    }
    else
    {
        SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }
}

void CLivePanel::HidePane()
{
    m_state = Hidden;
    ShowWindow(SW_HIDE);
    if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
}

void CLivePanel::TogglePane()
{
    if (m_state == Hidden)
    {
        if (!GetSafeHwnd()) { Create(IDD_LIVE_PANEL, GetParent()); ShowWindow(SW_SHOW); }
        m_state = DockLeft;
        ShowWindow(SW_SHOW);
        if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
    }
    else HidePane();
}

BOOL CLivePanel::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    LPNMHDR p = (LPNMHDR)lParam;
    if (p && p->hwndFrom == m_Tab.m_hWnd)
    {
        if (p->code == TCN_SELCHANGE)
        {
            int sel = m_Tab.GetCurSel();
            if (sel >= 0 && sel < (int)m_Previews.size())
            {
                int cam = m_Previews[sel]->CamIndex;
                if (CWnd* pMain = AfxGetMainWnd())
                {
                    auto* pFrm = dynamic_cast<CMainFrame*>(pMain);
                    if (pFrm && pFrm->GetSafeHwnd() && pFrm->GetActiveView())
                    {
                        auto* v = (class CFactoryVisionClientView*)pFrm->GetActiveView();
                        v->SetActiveCamera(cam);
                    }
                }
            }
        }
        else if (p->code == NM_RCLICK)
        {
            CPoint pt; GetCursorPos(&pt);
            ShowTabContextMenu(pt);
        }
    }
    return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CLivePanel::OnContextMenu(CWnd* pWnd, CPoint point)
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
    if (sel < 0 || sel >= (int)m_Previews.size()) return -1;
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

    // 캡처 폴더
    TCHAR exe[MAX_PATH]{};
    GetModuleFileName(NULL, exe, MAX_PATH);
    PathRemoveFileSpec(exe);
    PathAppend(exe, _T("captures"));
    CString capDir = exe;
    if (!exists((LPCTSTR)capDir)) { create_directories((LPCTSTR)capDir); }

    SYSTEMTIME st; GetLocalTime(&st);
    CString name; name.Format(_T("CAM%d_%04d%02d%02d_%02d%02d%02d.jpg"),
        cam + 1, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    CString full = capDir + _T("\\") + name;

    std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, m_jpegQuality };
    bool ok = cv::imwrite(CT2A(full), img, params);
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

void CLivePanel::OnSize(UINT, int, int)
{
    UpdateLayout();
}

// ==== 폭 리사이저(도킹 상태에서만) ====
void CLivePanel::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if (m_state == DockLeft || m_state == DockRight)
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
    CDialogEx::OnLButtonDown(nFlags, pt);
}

void CLivePanel::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_resizing)
    {
        m_resizing = false;
        ReleaseCapture();
        if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
    }
    CDialogEx::OnLButtonUp(nFlags, pt);
}

void CLivePanel::OnMouseMove(UINT nFlags, CPoint pt)
{
    if (m_resizing && (nFlags & MK_LBUTTON))
    {
        int delta = pt.x - m_resizeGrabX;
        if (m_state == DockLeft)  m_dockWidth = max(160, m_resizeStartW + delta);
        if (m_state == DockRight) m_dockWidth = max(160, m_resizeStartW - delta);
        if (CWnd* p = GetParent()) p->SendMessage(WM_SIZE);
    }
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
    return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}
