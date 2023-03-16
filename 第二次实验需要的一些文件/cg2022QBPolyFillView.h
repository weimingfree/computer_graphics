
// cg2022QBPolyFillView.h : Ccg2022QBPolyFillView 类的接口
//

#pragma once

#define N 64

class Ccg2022QBPolyFillView : public CView
{
protected: // 仅从序列化创建
	Ccg2022QBPolyFillView();
	DECLARE_DYNCREATE(Ccg2022QBPolyFillView)

// 特性
public:
	Ccg2022QBPolyFillDoc* GetDocument() const;

	CClientDC *m_pDC;

	int m_pNumbers;                     // polygon input buffer by mouse
	CPoint m_pAccord[N], m_mousePoint;

	int m_Begin, m_End, m_edgeNumbers, m_Scan;
	float m_yMax[N], m_yMin[N], m_Xa[N], m_Dx[N];

// 操作
public:
	void Fillpolygon(int pNumbers, CPoint *points, CDC *pDC);

	void pLoadPolygon(int pNumbers, CPoint *points);
	void pInsertLine(float x1, float y1, float x2, float y2);
	void pInclude();
	void pUpdateXvalue();
	void pXsort(int Begin, int i);
	void pFillScan(CDC* pDC);

	void SeedFill();

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~Ccg2022QBPolyFillView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // cg2022QBPolyFillView.cpp 中的调试版本
inline Ccg2022QBPolyFillDoc* Ccg2022QBPolyFillView::GetDocument() const
   { return reinterpret_cast<Ccg2022QBPolyFillDoc*>(m_pDocument); }
#endif

