#pragma once
class CFactoryVisionClientDoc : public CDocument
{
protected:
    CFactoryVisionClientDoc() noexcept;
    DECLARE_DYNCREATE(CFactoryVisionClientDoc)

public:
    virtual BOOL OnNewDocument() override;
    virtual void Serialize(CArchive& ar) override;

public:
    virtual ~CFactoryVisionClientDoc();
#ifdef _DEBUG
    virtual void AssertValid() const override;
    virtual void Dump(CDumpContext& dc) const override;
#endif

protected:
    DECLARE_MESSAGE_MAP()
};
