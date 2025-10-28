#include "pch.h"
#include "CCameraSetupDlg.h"
#include "resource.h"
#include "CCameraManager.h"
#include "MainFrm.h"

BEGIN_MESSAGE_MAP(CCameraSetupDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CCameraSetupDlg::OnBnClickedSearch)
    ON_BN_CLICKED(IDOK, &CCameraSetupDlg::OnBnClickedSave)
END_MESSAGE_MAP()

CCameraSetupDlg::CCameraSetupDlg(CWnd* p) : CDialogEx(IDD_CAMERA_SETUP, p) {}

void CCameraSetupDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BOOL CCameraSetupDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // List (ID: IDC_LIST1)
    m_list.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT, CRect(10, 10, 600, 220), this, 1001);
    m_list.InsertColumn(0, _T("Index"), LVCFMT_LEFT, 60);
    m_list.InsertColumn(1, _T("Serial"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(2, _T("IP"), LVCFMT_LEFT, 120);
    m_list.InsertColumn(3, _T("Port"), LVCFMT_LEFT, 80);
    m_list.InsertColumn(4, _T("Motion"), LVCFMT_LEFT, 70);
    m_list.InsertColumn(5, _T("Threshold"), LVCFMT_LEFT, 90);

    RefreshList();
    return TRUE;
}

void CCameraSetupDlg::LoadFromIni(std::vector<CameraConfig>& out) {
    out.clear();
    TCHAR path[MAX_PATH] = { 0 }; GetModuleFileName(nullptr, path, MAX_PATH);
    PathRemoveFileSpec(path); PathAppend(path, _T("config.ini"));
    CString ini = path;

    TCHAR buf[256];
    for (int i = 0;i < MAX_CAMERAS;++i) {
        CString sec; sec.Format(_T("CAM%d"), i + 1);
        GetPrivateProfileString(sec, _T("Serial"), _T(""), buf, 256, ini);
        CString serial = buf;
        if (serial.IsEmpty()) continue;
        CameraConfig c; c.nIndex = i; c.sSerial = serial;
        GetPrivateProfileString(sec, _T("IP"), _T("127.0.0.1"), buf, 256, ini); c.sIp = buf;
        c.nPort = GetPrivateProfileInt(sec, _T("Port"), 9000 + i, ini);
        c.nMotionThreshold = GetPrivateProfileInt(sec, _T("Threshold"), 5000, ini);
        c.bMotionEnabled = GetPrivateProfileInt(sec, _T("MotionEnable"), 1, ini);
        c.sFriendlyName.Format(_T("CAM %d"), i + 1);
        out.push_back(c);
    }
}
void CCameraSetupDlg::SaveToIni(const std::vector<CameraConfig>& in) {
    TCHAR path[MAX_PATH] = { 0 }; GetModuleFileName(nullptr, path, MAX_PATH);
    PathRemoveFileSpec(path); PathAppend(path, _T("config.ini"));
    CString ini = path;
    WritePrivateProfileString(nullptr, nullptr, nullptr, ini); // reset

    for (auto& c : in) {
        CString sec; sec.Format(_T("CAM%d"), c.nIndex + 1);
        WritePrivateProfileString(sec, _T("Serial"), c.sSerial, ini);
        WritePrivateProfileString(sec, _T("IP"), c.sIp, ini);
        CString v;
        v.Format(_T("%d"), c.nPort); WritePrivateProfileString(sec, _T("Port"), v, ini);
        v.Format(_T("%d"), c.nMotionThreshold); WritePrivateProfileString(sec, _T("Threshold"), v, ini);
        v.Format(_T("%d"), (int)c.bMotionEnabled); WritePrivateProfileString(sec, _T("MotionEnable"), v, ini);
    }
}

void CCameraSetupDlg::RefreshList() {
    std::vector<CameraConfig> cfgs; LoadFromIni(cfgs);
    m_list.DeleteAllItems();
    for (auto& c : cfgs) {
        int row = m_list.InsertItem(m_list.GetItemCount(), std::to_wstring(c.nIndex).c_str());
        m_list.SetItemText(row, 1, c.sSerial);
        m_list.SetItemText(row, 2, c.sIp);
        m_list.SetItemText(row, 3, std::to_wstring(c.nPort).c_str());
        m_list.SetItemText(row, 4, c.bMotionEnabled ? _T("ON") : _T("OFF"));
        m_list.SetItemText(row, 5, std::to_wstring(c.nMotionThreshold).c_str());
    }
}

// 검색 버튼: 시스템에 연결된 Basler 카메라 시리얼 자동 채움
void CCameraSetupDlg::OnBnClickedSearch() {
    auto* mf = dynamic_cast<class CMainFrame*>(AfxGetMainWnd());
    auto* mgr = mf ? mf->GetCameraManager() : nullptr;
    if (!mgr) { AfxMessageBox(_T("Camera manager not ready.")); return; }

    std::vector<Pylon::CDeviceInfo> devs;
    mgr->FindDevices(devs);

    // 기존 ini 불러오고, 빈 슬롯에 자동 채움
    std::vector<CameraConfig> cfgs; LoadFromIni(cfgs);
    std::array<bool, MAX_CAMERAS> used{}; for (auto& c : cfgs) used[c.nIndex] = true;

    for (auto& dv : devs) {
        CString serial(dv.GetSerialNumber());
        // 이미 등록돼 있으면 skip
        bool exists = false; for (auto& c : cfgs) { if (c.sSerial == serial) { exists = true; break; } }
        if (exists) continue;
        // 빈 index 할당
        for (int i = 0;i < MAX_CAMERAS;++i) {
            if (!used[i]) { used[i] = true; CameraConfig c; c.nIndex = i; c.sSerial = serial; c.sIp = _T("127.0.0.1"); c.nPort = 9000 + i; c.sFriendlyName.Format(_T("CAM %d"), i + 1); cfgs.push_back(c); break; }
        }
    }
    SaveToIni(cfgs);
    RefreshList();
}

void CCameraSetupDlg::OnBnClickedSave() {
    EndDialog(IDOK);
}
