#pragma once
#include "pch.h" // Should include SharedData.h and MFC headers
#include "resource.h" // Include resource IDs

// CDialogEx, CWnd, CDataExchange, CListCtrl, CEdit, CButton defined in MFC (via pch.h)
// CameraConfig defined in SharedData.h (via pch.h)

class CCameraSetupDlg : public CDialogEx {
public:
    CCameraSetupDlg(CWnd* pParent = nullptr);

    // Use the ID from resource.h
    enum { IDD = IDD_CAMERA_SETUP };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSearch();
    afx_msg void OnBnClickedSave();
    DECLARE_MESSAGE_MAP()

private:
    CListCtrl m_list;
    // Removed CEdit, CButton members as they weren't in the cpp file usage shown
    // int       m_selected{ -1 }; // Removed as not used in cpp

    void LoadFromIni(std::vector<CameraConfig>& out);
    void SaveToIni(const std::vector<CameraConfig>& in);
    void RefreshList();
};
