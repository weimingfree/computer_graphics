
// cg2022QB2DtransView.h : Ccg2022QB2DtransView ��Ľӿ�
//

#pragma once


class Ccg2022QB2DtransView : public CView
{
protected: // �������л�����
	Ccg2022QB2DtransView();
	DECLARE_DYNCREATE(Ccg2022QB2DtransView)

// ����
public:
	Ccg2022QB2DtransDoc* GetDocument() const;

	BOOL inputOK;
	CClientDC *m_pDC;

	int m_pNumbers;                     // polygon input buffer
	CPoint m_pAccord[N], m_mousePoint;

	int m_wndWidth, m_wndHeight;
// ����
public:
	void TransLine(CPoint *p1, CPoint *p2, CPoint *tp1, CPoint *tp2,
		           float transMatrix[3][2]);
	void DisplayLine(CDC* pDC, CPoint p1, CPoint p2, COLORREF rgbColor);

	void TransPolygon(int pointNumber, CPoint spPolygon[N],
		              CPoint transPolygon[N], float transMatrix[3][2]);
	void DisplayPolygon(CDC* pDC, int pointNumber,
		               CPoint transPolygon[N], COLORREF rgbColor);

	void CalculateMatrix(float transMatrix[3][2]);
	void RotateMatrix(float S, float C, float m[3][2]);
	void ScaleMatrix(float Sx, float Sy, float m[3][2]);
	void TranslateMatrix(float Tx, float Ty, float m[3][2]);

	int ClipLine(int *x1, int *y1, int *x2, int *y2);
	int pCode(int *x, int *y);
	int LineVisible(int *x1, int *y1, int *x2, int *y2);
	int pVisible(int x, int y, int i);
	int LineCross(int x1, int y1, int x2, int y2, int i);

	int ClipPolygon(int n, CPoint *tPoints, int *cn, CPoint *cPoints);
	void outPut(int x, int y, int *cn, CPoint *cPoints);
	void interSect(int Sx, int  Sy, int Px, int Py, int  i, int *ix, int *iy);

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
	virtual ~Ccg2022QB2DtransView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // cg2022QB2DtransView.cpp �еĵ��԰汾
inline Ccg2022QB2DtransDoc* Ccg2022QB2DtransView::GetDocument() const
   { return reinterpret_cast<Ccg2022QB2DtransDoc*>(m_pDocument); }
#endif

