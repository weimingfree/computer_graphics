
// cg2022QBRayTracerView.h : Ccg2022QBRayTracerView ��Ľӿ�
//

#pragma once

class CCgSmallpt;
class CCgRayTrace;
typedef unsigned int _vPIXEL;

class Ccg2022QBRayTracerView : public CView
{
protected: // �������л�����
	Ccg2022QBRayTracerView();
	DECLARE_DYNCREATE(Ccg2022QBRayTracerView)

// ����
public:
	Ccg2022QBRayTracerDoc* GetDocument() const;

	// RayTracer Bitmap Buffer
	int m_threadsNum;
	_vPIXEL *rayTracerDibImage;
	BITMAPINFO* rayTracerBitMapHeader;
	int m_rtImageWidth, m_rtImageHeight;
	char rayTracerBitMapBuffer[sizeof(BITMAPINFO) + 16];

	CCgRayTrace    *m_mitRayTracer;
	CCgSmallpt     *m_smallptRayTracer;

// ����
public:
	void MitRayTraceRender();
	void SmallptRayTraceRender();
	void SmallptCudaRayTraceRender();
	//void SmallptOmpRayTraceRender();
	//void SmallptCudaRayTraceRender();
	//void SmallptPPMRayTraceRender();
	//void SmallptPPMCudaRayTraceRender();

	void CreateBitMap(int width, int height);

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
	virtual ~Ccg2022QBRayTracerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // cg2022QBRayTracerView.cpp �еĵ��԰汾
inline Ccg2022QBRayTracerDoc* Ccg2022QBRayTracerView::GetDocument() const
   { return reinterpret_cast<Ccg2022QBRayTracerDoc*>(m_pDocument); }
#endif

