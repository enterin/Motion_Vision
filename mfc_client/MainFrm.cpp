#include "pch.h"
#include "framework.h"
#include "FactoryVisionClient.h"
#include "MainFrm.h"
#include "CCameraSetupDlg.h"
#include "resource.h"
#include "FactoryVisionClientView.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_TCP_STATUS, &CMainFrame::OnTcpStatus)
    ON_MESSAGE(WM_CAMERA_DISCONNECTED, &CMainFrame::OnCameraDisconnected)
    ON_MESSAGE(WM_CAMERA_STATUS, &CMainFrame::OnCameraStatus)
    ON_COMMAND_RANGE(ID_CAM_BTN_BASE, ID_CAM_BTN_BASE + MAX_CAMERAS - 1, &CMainFrame::OnCameraViewClick)
    ON_COMMAND(ID_MANUAL_CAPTURE_BTN, &CMainFrame::OnManualCaptureClick)
    ON_COMMAND(ID_SETTINGS_BTN, &CMainFrame::OnSettingsClick)
    ON_COMMAND(ID_VIEW_TOGGLE_LIVEPANEL, &CMainFrame::OnTogglePanel)
    ON_COMMAND(ID_VIEW_DOCK_LEFT, &CMainFrame::OnDockLeft)
    ON_COMMAND(ID_VIEW_DOCK_RIGHT, &CMainFrame::OnDockRight)
    ON_COMMAND(ID_VIEW_FLOAT, &CMainFrame::OnFloatPanel)
    ON_COMMAND(ID_VIEW_HIDE, &CMainFrame::OnHidePanel)
END_MESSAGE_MAP()

static UINT indicators[] = { ID_SEPARATOR };

CMainFrame::CMainFrame() noexcept : m_pView(nullptr) {}

CMainFrame::~CMainFrame()
{
    SaveConfigs();
    for (auto pBtn : m_vecCamButtons) { if (pBtn && pBtn->GetSafeHwnd()) pBtn->DestroyWindow(); delete pBtn; }
    m_vecCamButtons.clear();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

    if (!m_wndStatusBar.Create(this)) return -1;
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
    m_wndStatusBar.SetPaneText(0, _T("준비 완료"));

    CCreateContext ctx; ctx.m_pCurrentDoc = nullptr; ctx.m_pCurrentFrame = this;
    ctx.m_pNewViewClass = RUNTIME_CLASS(CFactoryVisionClientView);
    m_pView = (CFactoryVisionClientView*)CreateView(&ctx, AFX_IDW_PANE_FIRST);
    if (!m_pView) return -1;

    m_FontUI.CreatePointFont(120, _T("맑은 고딕"));

    m_btnTogglePanel.Create(_T("📌 패널"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, CRect(0, 0, 0, 0), this, ID_VIEW_TOGGLE_LIVEPANEL);
    m_btnTogglePanel.SetFont(&m_FontUI);
    m_btnManualCapture.Create(_T("📸 촬영"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, CRect(0, 0, 0, 0), this, ID_MANUAL_CAPTURE_BTN);
    m_btnManualCapture.SetFont(&m_FontUI);
    m_btnSettings.Create(_T("⚙️ 설정"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, CRect(0, 0, 0, 0), this, ID_SETTINGS_BTN);
    m_btnSettings.SetFont(&m_FontUI);

    LoadConfigs();
    CreateDynamicButtonsLayout();
    UpdateCameraButtons();

    // --- 수정된 부분 (CreateModeless -> Create) ---
    if (!m_LivePanel.Create(IDD_LIVE_PANEL, this)) {
        TRACE0("Failed to create live panel\n");
        return -1;
    }
    // --- 수정 끝 ---

    switch (m_LivePanel.m_state)
    {
    case CLivePanel::DockLeft:  m_LivePanel.DockLeftPane();  break;
    case CLivePanel::DockRight: m_LivePanel.DockRightPane(); break;
    case CLivePanel::Floating:  m_LivePanel.FloatPane();     break;
    case CLivePanel::Hidden:    m_LivePanel.HidePane();      break;
    }
    m_LivePanel.UpdateLayout();

    if (m_pView && m_pView->GetSafeHwnd())
    {
        m_LivePanel.BuildTabs(m_CamConfigs); // 탭 빌드 위치 변경
        for (const auto& cfg : m_CamConfigs) m_CameraManager.ConnectCamera(cfg, m_pView->GetSafeHwnd());
        m_pView->SetActiveCamera(0);
    }

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow(cs)) return FALSE;
    cs.style &= ~FWS_ADDTOTITLE;
    cs.lpszName = _T("음료 캔 불량 검출 시스템 (Factory Vision Client)");
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.style |= WS_MAXIMIZE;
    return TRUE;
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const { CFrameWnd::AssertValid(); }
void CMainFrame::Dump(CDumpContext& dc) const { CFrameWnd::Dump(dc); }
#endif

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    if (GetSafeHwnd() && m_pView && m_pView->GetSafeHwnd()) UpdateLayout(cx, cy);
}

void CMainFrame::UpdateLayout(int cx, int cy)
{
    if (!m_pView || !::IsWindow(m_pView->GetSafeHwnd()) || cx <= 0 || cy <= 0) return;

    const int nTopBarH = 40;
    const int nSBH = 20;
    const int btnW = 86, btnH = nTopBarH - 10, margin = 8;

    int dockW = (m_LivePanel.m_state == CLivePanel::Hidden) ? 0 : m_LivePanel.m_dockWidth;
    dockW = std::max(160, std::min(dockW, std::max(160, cx * 3 / 5)));

    int panelX = 0;
    if (m_LivePanel.m_state == CLivePanel::DockLeft)      panelX = 0;
    else if (m_LivePanel.m_state == CLivePanel::DockRight) panelX = cx - dockW;

    if (m_LivePanel.m_state == CLivePanel::DockLeft || m_LivePanel.m_state == CLivePanel::DockRight)
    {
        if (m_LivePanel.GetSafeHwnd())
        {
            m_LivePanel.ShowWindow(SW_SHOW);
            m_LivePanel.MoveWindow(panelX, nTopBarH, dockW, cy - nTopBarH - nSBH);
        }
    }
    else
    {
        if (m_LivePanel.m_state == CLivePanel::Hidden && m_LivePanel.GetSafeHwnd()) m_LivePanel.ShowWindow(SW_HIDE);
    }

    int x = (m_LivePanel.m_state == CLivePanel::DockLeft ? dockW : 0) + margin;
    m_btnTogglePanel.MoveWindow(x, 4, btnW, btnH); x += (btnW + margin);
    m_btnManualCapture.MoveWindow(x, 4, btnW, btnH); x += (btnW + margin);
    m_btnSettings.MoveWindow(x, 4, btnW, btnH); x += (btnW + margin);

    for (auto* b : m_vecCamButtons)
        if (b && b->GetSafeHwnd()) { b->MoveWindow(x, 4, btnW, btnH); x += (btnW + 6); }

    int viewX = (m_LivePanel.m_state == CLivePanel::DockLeft ? dockW : 0);
    int viewW = cx - viewX - (m_LivePanel.m_state == CLivePanel::DockRight ? dockW : 0);
    m_pView->MoveWindow(viewX, nTopBarH, viewW, cy - nTopBarH - nSBH);

    m_wndStatusBar.MoveWindow(0, cy - nSBH, cx, nSBH);
}

void CMainFrame::CreateDynamicButtonsLayout()
{
    for (auto pBtn : m_vecCamButtons) { if (pBtn && pBtn->GetSafeHwnd()) pBtn->DestroyWindow(); delete pBtn; }
    m_vecCamButtons.clear();

    for (const auto& cfg : m_CamConfigs)
    {
        CButton* pBtn = new CButton();
        CString txt; txt.Format(_T("CAM %d"), cfg.nIndex + 1);
        if (pBtn->Create(txt, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, CRect(0, 0, 0, 0), this, ID_CAM_BTN_BASE + cfg.nIndex))
        {
            pBtn->SetFont(&m_FontUI);
            m_vecCamButtons.push_back(pBtn);
        }
        else delete pBtn;
    }

    CRect r; GetClientRect(&r);
    UpdateLayout(r.Width(), r.Height());
}

void CMainFrame::UpdateCameraButtons()
{
    for (size_t i = 0; i < m_vecCamButtons.size(); ++i)
    {
        if (m_vecCamButtons[i] && ::IsWindow(m_vecCamButtons[i]->GetSafeHwnd()))
        {
            bool bConnected = false;
            if (i < m_CamConfigs.size()) {
                int nCamIndex = m_CamConfigs[i].nIndex;
                bConnected = m_CameraManager.IsCameraConnected(nCamIndex);
            }
            m_vecCamButtons[i]->EnableWindow(bConnected);
        }
    }

    if (m_pView) {
        int activeIndex = m_pView->GetActiveCameraIndex();
        for (size_t i = 0; i < m_vecCamButtons.size(); ++i) {
            if (m_vecCamButtons[i] && ::IsWindow(m_vecCamButtons[i]->GetSafeHwnd())) {
                CString t; m_vecCamButtons[i]->GetWindowText(t);
                t.Replace(_T(" <Active>"), _T(""));
                if (i < m_CamConfigs.size() && m_CamConfigs[i].nIndex == activeIndex)
                    m_vecCamButtons[i]->SetWindowText(t + _T(" <Active>"));
                else
                    m_vecCamButtons[i]->SetWindowText(t);
            }
        }
    }
}

void CMainFrame::OnSettingsClick()
{
    CCameraSetupDlg dlg(this);
    if (dlg.DoModal() == IDOK)
    {
        m_wndStatusBar.SetPaneText(0, _T("설정 적용 중..."));
        m_CameraManager.DisconnectAll();
        LoadConfigs();
        CreateDynamicButtonsLayout();
        UpdateCameraButtons();
        if (m_pView && m_pView->GetSafeHwnd()) {
            m_LivePanel.BuildTabs(m_CamConfigs); // 탭 먼저 갱신
            for (const auto& cfg : m_CamConfigs) m_CameraManager.ConnectCamera(cfg, m_pView->GetSafeHwnd());
            m_pView->SetActiveCamera(0);
        }
        m_wndStatusBar.SetPaneText(0, _T("설정 적용 완료."));
    }
}

void CMainFrame::OnManualCaptureClick()
{
    if (!m_pView) return;
    int nActiveCamIndex = m_pView->GetActiveCameraIndex();
    if (nActiveCamIndex >= 0 && nActiveCamIndex < MAX_CAMERAS && m_CameraManager.IsCameraConnected(nActiveCamIndex))
    {
        m_CameraManager.TriggerManualCapture(nActiveCamIndex);
        CString s; s.Format(_T("수동 촬영 트리거 (CAM %d)"), nActiveCamIndex + 1);
        m_wndStatusBar.SetPaneText(0, s);
    }
    else m_wndStatusBar.SetPaneText(0, _T("활성 카메라가 연결되지 않음"));
}

void CMainFrame::OnCameraViewClick(UINT nID)
{
    if (!m_pView) return;
    int nCamIndex = nID - ID_CAM_BTN_BASE;
    if (nCamIndex >= 0 && nCamIndex < MAX_CAMERAS && m_CameraManager.IsCameraConnected(nCamIndex))
    {
        m_pView->SetActiveCamera(nCamIndex);
        UpdateCameraButtons();
        CString s; s.Format(_T("CAM %d 뷰 활성화"), nCamIndex + 1);
        m_wndStatusBar.SetPaneText(0, s);
    }
    else {
        CString s; s.Format(_T("CAM %d 미연결"), nCamIndex + 1);
        m_wndStatusBar.SetPaneText(0, s);
    }
}

LRESULT CMainFrame::OnTcpStatus(WPARAM wParam, LPARAM lParam)
{
    int nCamIndex = (int)wParam;
    BOOL bConnected = (BOOL)lParam;
    CString s; s.Format(_T("CAM %d: 서버 %s"), nCamIndex + 1, bConnected ? _T("연결") : _T("해제"));
    UpdateCameraButtons();
    m_wndStatusBar.SetPaneText(0, s);
    return 0;
}

LRESULT CMainFrame::OnCameraDisconnected(WPARAM wParam, LPARAM)
{
    int nCamIndex = (int)wParam;
    CString s; s.Format(_T("CAM %d: 카메라 연결 해제"), nCamIndex + 1);
    UpdateCameraButtons();
    m_wndStatusBar.SetPaneText(0, s);
    return 0;
}

LRESULT CMainFrame::OnCameraStatus(WPARAM wParam, LPARAM lParam)
{
    int nCamIndex = (int)wParam;
    CString* p = (CString*)lParam;
    if (p) {
        CString s; s.Format(_T("CAM %d Status: %s"), nCamIndex + 1, *p);
        m_wndStatusBar.SetPaneText(0, s);
        delete p;
    }
    return 0;
}

void CMainFrame::OnTogglePanel() { m_LivePanel.TogglePane(); }
void CMainFrame::OnDockLeft() { m_LivePanel.DockLeftPane(); }
void CMainFrame::OnDockRight() { m_LivePanel.DockRightPane(); }
void CMainFrame::OnFloatPanel() { m_LivePanel.FloatPane(); }
void CMainFrame::OnHidePanel() { m_LivePanel.HidePane(); }

// ===== 설정 저장/로드 =====
CString CMainFrame::GetIniPath() const
{
    TCHAR path[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);
    PathRemoveFileSpec(path);
    PathAppend(path, _T("config.ini"));
    return CString(path);
}

BOOL CMainFrame::LoadCameraConfigs(std::vector<CameraConfig>& out) // 헤더에도 선언 추가 필요
{
    out.clear();
    CString ini = GetIniPath();
    TCHAR b[256]{};

    for (int i = 0; i < MAX_CAMERAS; ++i)
    {
        CString sec; sec.Format(_T("CAM%d"), i + 1);
        GetPrivateProfileString(sec, _T("Serial"), _T(""), b, _countof(b), ini);
        CString ser = b;
        if (ser.IsEmpty()) continue;

        CameraConfig cfg;
        cfg.nIndex = i;
        cfg.sSerial = ser;
        cfg.sFriendlyName.Format(_T("CAM %d"), i + 1);
        GetPrivateProfileString(sec, _T("IP"), _T("127.0.0.1"), b, _countof(b), ini);
        cfg.sIp = b;
        cfg.nPort = GetPrivateProfileInt(sec, _T("Port"), 9000 + i, ini);
        cfg.nMotionThreshold = GetPrivateProfileInt(sec, _T("Threshold"), 5000, ini);
        cfg.bMotionEnabled = GetPrivateProfileInt(sec, _T("MotionEnable"), 1, ini);
        out.push_back(cfg);
    }
    return TRUE; // 성공 여부 반환 (여기서는 항상 TRUE)
}

BOOL CMainFrame::SaveCameraConfigs(const std::vector<CameraConfig>& cfgs) // 헤더에도 선언 추가 필요
{
    CString ini = GetIniPath();
    for (const auto& cfg : cfgs)
    {
        CString sec; sec.Format(_T("CAM%d"), cfg.nIndex + 1);
        WritePrivateProfileString(sec, _T("Serial"), cfg.sSerial, ini);
        WritePrivateProfileString(sec, _T("IP"), cfg.sIp, ini);
        CString s; s.Format(_T("%d"), cfg.nPort);            WritePrivateProfileString(sec, _T("Port"), s, ini);
        s.Format(_T("%d"), cfg.nMotionThreshold);            WritePrivateProfileString(sec, _T("Threshold"), s, ini);
        s.Format(_T("%d"), cfg.bMotionEnabled ? 1 : 0);      WritePrivateProfileString(sec, _T("MotionEnable"), s, ini);
    }
    return TRUE; // 성공 여부 반환 (여기서는 항상 TRUE)
}


void CMainFrame::LoadConfigs()
{
    LoadCameraConfigs(m_CamConfigs); // 분리된 함수 사용

    CString ini = GetIniPath();
    TCHAR b[256]{};

    CString dock = _T("Left");
    GetPrivateProfileString(_T("Panel"), _T("DockState"), _T("Left"), b, _countof(b), ini);
    dock = b;

    int w = GetPrivateProfileInt(_T("Panel"), _T("DockWidth"), 380, ini);
    CString fr; GetPrivateProfileString(_T("Panel"), _T("FloatRect"), _T("100,100,460,620"), b, _countof(b), ini);
    fr = b;

    int fps = GetPrivateProfileInt(_T("Panel"), _T("PreviewFpsCap"), 12, ini);
    int jq = GetPrivateProfileInt(_T("Panel"), _T("JpegQuality"), 92, ini);

    CLivePanel::DockState st = CLivePanel::DockLeft;
    if (dock.CompareNoCase(_T("Right")) == 0) st = CLivePanel::DockRight;
    else if (dock.CompareNoCase(_T("Floating")) == 0) st = CLivePanel::Floating;
    else if (dock.CompareNoCase(_T("Hidden")) == 0) st = CLivePanel::Hidden;

    int L = 100, T = 100, W = 360, H = 500;
    _stscanf_s(fr, _T("%d,%d,%d,%d"), &L, &T, &W, &H);
    CRect r(L, T, L + W, T + H);

    m_LivePanel.ApplyConfig(w, st, r, fps, jq);

    CString msg; msg.Format(_T("%d개의 카메라 설정 로드됨."), (int)m_CamConfigs.size());
    m_wndStatusBar.SetPaneText(0, msg);
}

void CMainFrame::SaveConfigs()
{
    SaveCameraConfigs(m_CamConfigs); // 분리된 함수 사용

    CString ini = GetIniPath();

    CString dock = _T("Left");
    switch (m_LivePanel.m_state)
    {
    case CLivePanel::DockLeft:  dock = _T("Left"); break;
    case CLivePanel::DockRight: dock = _T("Right"); break;
    case CLivePanel::Floating:  dock = _T("Floating"); break;
    case CLivePanel::Hidden:    dock = _T("Hidden"); break;
    }
    WritePrivateProfileString(_T("Panel"), _T("DockState"), dock, ini);

    CString s; s.Format(_T("%d"), m_LivePanel.m_dockWidth);
    WritePrivateProfileString(_T("Panel"), _T("DockWidth"), s, ini);

    CRect fr = m_LivePanel.m_floatRect;
    s.Format(_T("%d,%d,%d,%d"), fr.left, fr.top, fr.Width(), fr.Height());
    WritePrivateProfileString(_T("Panel"), _T("FloatRect"), s, ini);

    s.Format(_T("%d"), m_LivePanel.m_previewFpsCap);
    WritePrivateProfileString(_T("Panel"), _T("PreviewFpsCap"), s, ini);

    s.Format(_T("%d"), m_LivePanel.m_jpegQuality);
    WritePrivateProfileString(_T("Panel"), _T("JpegQuality"), s, ini);
}