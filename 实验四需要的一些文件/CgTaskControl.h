#pragma once

#define N 64

#include "cg2022QB3DtransDoc.h"

// CCgTaskControl 窗体视图

class CCgTaskControl : public CFormView
{
	DECLARE_DYNCREATE(CCgTaskControl)

protected:
	CCgTaskControl();           // 动态创建所使用的受保护的构造函数
	virtual ~CCgTaskControl();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKCONTROL };
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
	afx_msg void OnSelchangeTransselect();
	afx_msg void OnClickedTransmode();
	afx_msg void OnClickedXleft();
	afx_msg void OnClickedXright();
	afx_msg void OnClickedYup();
	afx_msg void OnClickedYdown();
	afx_msg void OnClickedZfront();
	afx_msg void OnClickedZback();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
private:
	int      m_objNumber;
	Object_p m_whoObject;                   // Object pointer
	COLORREF vcObjColor[SPACEOBJECTS];

	int   prjPlaneRetSize;       // Z-Buffer Array Size
	float *prjPlaneRetZ;         // Z-Buffer Array Memory

	void pTransToZbuffer(CRect dcRect);
	void pDrawLineObject(CDC *pDC, CRect dcRect);
};


