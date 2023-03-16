
// cg2022QBPolyFillView.h : Ccg2022QBPolyFillView ��Ľӿ�
//

#pragma once

#define N 64

class Ccg2022QBPolyFillView : public CView
{
protected: // �������л�����
	Ccg2022QBPolyFillView();
	DECLARE_DYNCREATE(Ccg2022QBPolyFillView)

// ����
public:
	Ccg2022QBPolyFillDoc* GetDocument() const;

	CClientDC *m_pDC;

	int m_pNumbers;                     // polygon input buffer by mouse
	CPoint m_pAccord[N], m_mousePoint;

	int m_Begin, m_End, m_edgeNumbers, m_Scan;
	float m_yMax[N], m_yMin[N], m_Xa[N], m_Dx[N];

// ����
public:
	void Fillpolygon(int pNumbers, CPoint *points, CDC *pDC);

	void pLoadPolygon(int pNumbers, CPoint *points);
	void pInsertLine(float x1, float y1, float x2, float y2);
	void pInclude();
	void pUpdateXvalue();
	void pXsort(int Begin, int i);
	void pFillScan(CDC* pDC);

	void SeedFill();

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~Ccg2022QBPolyFillView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // cg2022QBPolyFillView.cpp �еĵ��԰汾
inline Ccg2022QBPolyFillDoc* Ccg2022QBPolyFillView::GetDocument() const
   { return reinterpret_cast<Ccg2022QBPolyFillDoc*>(m_pDocument); }
#endif

