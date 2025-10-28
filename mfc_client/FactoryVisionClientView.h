#pragma once
#include "SharedData.h"
#include <list>
#include <vector>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

class CFactoryVisionClientView : public CView
{
protected:
    CFactoryVisionClientView() noexcept;
    DECLARE_DYNCREATE(CFactoryVisionClientView)

public:
    CFactoryVisionClientDoc* GetDocument() const;
    int  GetActiveCameraIndex() const { return m_nActiveCameraIndex; }
    void SetActiveCamera(int nIndex);

public:
    virtual void OnDraw(CDC* pDC) override;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;

protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) override;
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) override;

public:
    virtual ~CFactoryVisionClientView();
#ifdef _DEBUG
    virtual void AssertValid() const override;
    virtual void Dump(CDumpContext& dc) const override;
#endif

protected:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnUpdateFrame(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnInspectionResult(WPARAM wParam, LPARAM lParam);
    afx_msg void OnExportCsv();
    DECLARE_MESSAGE_MAP()

private:
    // GDI+
    ULONG_PTR m_gdiplusToken{};
    Gdiplus::Font* m_pFontLarge{};
    Gdiplus::Font* m_pFontMedium{};
    Gdiplus::Font* m_pFontSmall{};
    Gdiplus::Font* m_pFontDefect{};

    // 상태
    int m_nActiveCameraIndex{ 0 };

    // 프레임/결과 캐시
    CCriticalSection m_csFrame[MAX_CAMERAS];
    cv::Mat m_CurrentFrames[MAX_CAMERAS];

    CCriticalSection m_csResult;
    std::list<DefectData> m_DefectHistory;
    const size_t MAX_DEFECT_HISTORY = 200;

    // 그리기 헬퍼
    void DrawVideo(Gdiplus::Graphics& g, const Gdiplus::RectF& rectVideo);
    void DrawDashboard(Gdiplus::Graphics& g, const Gdiplus::RectF& rectDash);
    void DrawBitmap(Gdiplus::Graphics& g, const cv::Mat& image, const Gdiplus::RectF& rect, bool keepAR = true);
    void DrawTextWithShadow(Gdiplus::Graphics& g, const CString& text, Gdiplus::Font* font,
        const Gdiplus::RectF& rect, const Gdiplus::Brush* brush, Gdiplus::StringFormat* fmt);

    // 파일/CSV
    bool SaveJpegToFile(const cv::Mat& img, CString& outPath);   // JPEG 품질은 config.ini에서 읽음
    bool ExportHistoryToCsv(const CString& filePath);

    // config 읽기 (JPEG 품질)
    int ReadJpegQualityFromIni() const;
};
