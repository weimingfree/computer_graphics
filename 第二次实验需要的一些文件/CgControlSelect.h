#pragma once



// CCgControlSelect 窗体视图

class CCgControlSelect : public CFormView
{
	DECLARE_DYNCREATE(CCgControlSelect)

protected:
	CCgControlSelect();           // 动态创建所使用的受保护的构造函数
	virtual ~CCgControlSelect();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONTROLSELECT };
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
	afx_msg void OnClickedPolyfill();
	afx_msg void OnClickedSeedfill();
};


