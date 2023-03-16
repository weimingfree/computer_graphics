
// cg2022QB3DTransView.h : Ccg2022QB3DTransView ��Ľӿ�
//

#pragma once

class CDrawScene;

class Ccg2022QB3DTransView : public CView
{
protected: // �������л�����
	Ccg2022QB3DTransView();
	DECLARE_DYNCREATE(Ccg2022QB3DTransView)

// ����
public:
	Ccg2022QB3DTransDoc* GetDocument() const;

	CClientDC  *m_pDC;
	BOOL       m_autoPlay;
	CRect      m_viewRect;

	CDrawScene *m_drawScene;

// ����
public:
	BOOL bSetupPixelFormat();

	void DrawScene();

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
	virtual ~Ccg2022QB3DTransView();
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // cg2022QB3DTransView.cpp �еĵ��԰汾
inline Ccg2022QB3DTransDoc* Ccg2022QB3DTransView::GetDocument() const
   { return reinterpret_cast<Ccg2022QB3DTransDoc*>(m_pDocument); }
#endif

