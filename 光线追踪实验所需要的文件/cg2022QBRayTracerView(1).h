
// cg2022QBRayTracerView.h : Ccg2022QBRayTracerView 类的接口
//

#pragma once

class CCgSmallpt;
class CCgRayTrace;
typedef unsigned int _vPIXEL;

class Ccg2022QBRayTracerView : public CView
{
protected: // 仅从序列化创建
	Ccg2022QBRayTracerView();
	DECLARE_DYNCREATE(Ccg2022QBRayTracerView)

// 特性
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

// 操作
public:
	void MitRayTraceRender();
	void SmallptRayTraceRender();
	void SmallptCudaRayTraceRender();
	//void SmallptOmpRayTraceRender();
	//void SmallptCudaRayTraceRender();
	//void SmallptPPMRayTraceRender();
	//void SmallptPPMCudaRayTraceRender();

	void CreateBitMap(int width, int height);

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
	virtual ~Ccg2022QBRayTracerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // cg2022QBRayTracerView.cpp 中的调试版本
inline Ccg2022QBRayTracerDoc* Ccg2022QBRayTracerView::GetDocument() const
   { return reinterpret_cast<Ccg2022QBRayTracerDoc*>(m_pDocument); }
#endif

