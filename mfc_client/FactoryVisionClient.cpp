// FactoryVisionClient.cpp: 애플리케이션에 대한 클래스 동작 정의

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "FactoryVisionClient.h"
#include "MainFrm.h"
#include "FactoryVisionClientDoc.h"
#include "FactoryVisionClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CFactoryVisionClientApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CFactoryVisionClientApp::OnAppAbout)
    ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
    ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

CFactoryVisionClientApp theApp;

CFactoryVisionClientApp::CFactoryVisionClientApp() noexcept
    : m_gdiplusToken(0)
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
    SetAppID(_T("MyCompany.FactoryVisionClient.VisionClient.1.0"));
}

BOOL CFactoryVisionClientApp::InitInstance()
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);
    CWinApp::InitInstance();

    if (!AfxOleInit()) { AfxMessageBox(IDP_OLE_INIT_FAILED); return FALSE; }
    AfxEnableControlContainer();
    EnableTaskbarInteraction(FALSE);

    Gdiplus::GdiplusStartupInput gsi;
    if (Gdiplus::GdiplusStartup(&m_gdiplusToken, &gsi, NULL) != Gdiplus::Ok)
    {
        AfxMessageBox(_T("GDI+ 초기화 실패")); return FALSE;
    }

    if (!InitializeWinsock())
    {
        AfxMessageBox(_T("Winsock 초기화 실패")); Gdiplus::GdiplusShutdown(m_gdiplusToken); return FALSE;
    }

    SetRegistryKey(_T("MyCompany Vision Systems"));
    LoadStdProfileSettings(4);

    CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CFactoryVisionClientDoc),
        RUNTIME_CLASS(CMainFrame),
        RUNTIME_CLASS(CFactoryVisionClientView));
    if (!pDocTemplate) return FALSE;
    AddDocTemplate(pDocTemplate);

    CCommandLineInfo cmdInfo; ParseCommandLine(cmdInfo);
    if (!ProcessShellCommand(cmdInfo)) return FALSE;

    m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
    m_pMainWnd->UpdateWindow();
    return TRUE;
}

int CFactoryVisionClientApp::ExitInstance()
{
    AfxOleTerm(FALSE);
    if (m_gdiplusToken) Gdiplus::GdiplusShutdown(m_gdiplusToken);
    WSACleanup();
    return CWinApp::ExitInstance();
}

BOOL CFactoryVisionClientApp::InitializeWinsock()
{
    WSADATA wsa; return (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
}

class CAboutDlg : public CDialogEx
{
public: CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX) {}
protected:
    virtual void DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
    DECLARE_MESSAGE_MAP()
};
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx) END_MESSAGE_MAP()

    void CFactoryVisionClientApp::OnAppAbout()
{
    CAboutDlg dlg; dlg.DoModal();
}
