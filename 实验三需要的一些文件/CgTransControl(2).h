#pragma once



// CCgTransControl 窗体视图

class CCgTransControl : public CFormView
{
	DECLARE_DYNCREATE(CCgTransControl)

protected:
	CCgTransControl();           // 动态创建所使用的受保护的构造函数
	virtual ~CCgTransControl();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRANSCONTROL };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_transSelect;
	afx_msg void OnClickedXleft();
	afx_msg void OnClickedXright();
	afx_msg void OnClickedYup();
	afx_msg void OnClickedYdown();
	afx_msg void OnClickedTransmode();
	afx_msg void OnSelchangeTransselect();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

private:
	void ViewTransLine(CDC* pDC, CRect dcRect);
	void ViewTransPolygon(CDC* pDC, CRect dcRect);
};


