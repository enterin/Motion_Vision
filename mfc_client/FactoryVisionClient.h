// FactoryVisionClient.h : FactoryVisionClient 애플리케이션의 기본 헤더 파일
#pragma once

#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"

class CFactoryVisionClientApp : public CWinApp
{
public:
    CFactoryVisionClientApp() noexcept;

public:
    virtual BOOL InitInstance() override;
    virtual int  ExitInstance() override;

    afx_msg void OnAppAbout();
    DECLARE_MESSAGE_MAP()

private:
    ULONG_PTR m_gdiplusToken;   // GDI+ 토큰
    BOOL InitializeWinsock();   // Winsock 초기화
};

// 애플리케이션의 유일한 인스턴스
extern CFactoryVisionClientApp theApp;
