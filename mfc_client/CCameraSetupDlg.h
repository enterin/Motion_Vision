#pragma once
#include "SharedData.h"

class CCameraSetupDlg : public CDialogEx {
public:
    CCameraSetupDlg(CWnd* pParent = nullptr);
    enum { IDD = IDD_CAMERA_SETUP };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSearch();
    afx_msg void OnBnClickedSave();
    DECLARE_MESSAGE_MAP()

private:
    CListCtrl m_list;
    CEdit     m_ip, m_port, m_th;
    CButton   m_motion;
    int       m_selected{ -1 };

    void LoadFromIni(std::vector<CameraConfig>& out);
    void SaveToIni(const std::vector<CameraConfig>& in);
    void RefreshList();
};
