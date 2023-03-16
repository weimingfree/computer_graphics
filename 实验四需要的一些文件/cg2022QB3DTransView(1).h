
// cg2022QB3DTransView.h : Ccg2022QB3DTransView 类的接口
//

#pragma once

class CDrawScene;

class Ccg2022QB3DTransView : public CView
{
protected: // 仅从序列化创建
	Ccg2022QB3DTransView();
	DECLARE_DYNCREATE(Ccg2022QB3DTransView)

// 特性
public:
	Ccg2022QB3DTransDoc* GetDocument() const;

	CClientDC  *m_pDC;
	BOOL       m_autoPlay;
	CRect      m_viewRect;

	CDrawScene *m_drawScene;

// 操作
public:
	BOOL bSetupPixelFormat();

	void DrawScene();

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
	virtual ~Ccg2022QB3DTransView();
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // cg2022QB3DTransView.cpp 中的调试版本
inline Ccg2022QB3DTransDoc* Ccg2022QB3DTransView::GetDocument() const
   { return reinterpret_cast<Ccg2022QB3DTransDoc*>(m_pDocument); }
#endif

