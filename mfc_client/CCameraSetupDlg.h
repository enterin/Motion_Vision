#pragma once
#include "afxdialogex.h"
#include <vector> // std::vector ��� ���� �߰�
#include "SharedData.h" // CameraConfig ��� ���� �߰�
#include <afxcmn.h> // CListCtrl ��� ���� �߰�

class CCameraSetupDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CCameraSetupDlg)

public:
    CCameraSetupDlg(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
    virtual ~CCameraSetupDlg() = default; // ���� �Ҹ��� �߰�

    // ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CAMERA_SETUP }; // IDD_CAMERA_SETUP Ȯ�� �ʿ�
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
    virtual BOOL OnInitDialog(); // OnInitDialog ���� �߰�

    // --- ������ �κ� (��� �Լ� �� ���� ���� �߰�) ---
    CListCtrl m_list; // ����Ʈ ��Ʈ�� ��� ����
    void LoadAndDisplay(); // �Լ� ���� �߰�
    BOOL LoadFromIni(std::vector<CameraConfig>& out); // �Լ� ���� �߰�
    BOOL SaveToIni(const std::vector<CameraConfig>& cfgs); // �Լ� ���� �߰�
    afx_msg void OnBnClickedSearch(); // ��ư �ڵ鷯 ���� �߰�
    afx_msg void OnBnClickedSave(); // ��ư �ڵ鷯 ���� �߰�
    // --- ���� �� ---

    DECLARE_MESSAGE_MAP()
};