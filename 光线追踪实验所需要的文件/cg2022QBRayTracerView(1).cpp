
// cg2022QBRayTracerView.cpp : Ccg2022QBRayTracerView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "cg2022QBRayTracer.h"
#endif

#include "cg2022QBRayTracerDoc.h"
#include "cg2022QBRayTracerView.h"
#include "..\MIT-RayTrace\RayTrace.h"
#include "..\smallpt\smallpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ccg2022QBRayTracerView

IMPLEMENT_DYNCREATE(Ccg2022QBRayTracerView, CView)

BEGIN_MESSAGE_MAP(Ccg2022QBRayTracerView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// Ccg2022QBRayTracerView 构造/析构

Ccg2022QBRayTracerView::Ccg2022QBRayTracerView()
{
	// TODO: 在此处添加构造代码
	rayTracerDibImage = NULL;
	m_mitRayTracer = new CCgRayTrace();
	m_smallptRayTracer = new CCgSmallpt();
}

Ccg2022QBRayTracerView::~Ccg2022QBRayTracerView()
{
}

BOOL Ccg2022QBRayTracerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// Ccg2022QBRayTracerView 绘制

void Ccg2022QBRayTracerView::OnDraw(CDC* /*pDC*/)
{
	Ccg2022QBRayTracerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	switch (pDoc->m_rayTraceMethod) {
	case 0: // smallpt
		SmallptRayTraceRender();
		break;
	case 1: // smallptCuda
		SmallptCudaRayTraceRender();
		break;
	case 2: // MIT-RayTrace
		MitRayTraceRender();
		break;
	}
}


// Ccg2022QBRayTracerView 打印

BOOL Ccg2022QBRayTracerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void Ccg2022QBRayTracerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void Ccg2022QBRayTracerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// Ccg2022QBRayTracerView 诊断

#ifdef _DEBUG
void Ccg2022QBRayTracerView::AssertValid() const
{
	CView::AssertValid();
}

void Ccg2022QBRayTracerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Ccg2022QBRayTracerDoc* Ccg2022QBRayTracerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Ccg2022QBRayTracerDoc)));
	return (Ccg2022QBRayTracerDoc*)m_pDocument;
}
#endif //_DEBUG


// Ccg2022QBRayTracerView 消息处理程序
void Ccg2022QBRayTracerView::CreateBitMap(int width, int height)
{
	for (int i = 0; i < sizeof(BITMAPINFOHEADER) + 16; i++) rayTracerBitMapBuffer[i] = 0;
	rayTracerBitMapHeader = (BITMAPINFO *)&rayTracerBitMapBuffer;
	rayTracerBitMapHeader->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	rayTracerBitMapHeader->bmiHeader.biPlanes = 1;
	rayTracerBitMapHeader->bmiHeader.biBitCount = 32;
	rayTracerBitMapHeader->bmiHeader.biCompression = BI_BITFIELDS;
	rayTracerBitMapHeader->bmiHeader.biWidth = width;
	rayTracerBitMapHeader->bmiHeader.biHeight = height;

	((unsigned long*)rayTracerBitMapHeader->bmiColors)[0] = 255 << 16;
	((unsigned long*)rayTracerBitMapHeader->bmiColors)[1] = 255 << 8;
	((unsigned long*)rayTracerBitMapHeader->bmiColors)[2] = 255;

}

// MIT RayTracer Render Processing Functions.
void Ccg2022QBRayTracerView::MitRayTraceRender()
{
	static BOOL opBusy = FALSE;

	if (opBusy) return;

	opBusy = TRUE;

	CDC     *pDC;
	CRect    pDCRet;

	pDC = GetDC();
	GetClientRect(&pDCRet);

	// Clear BackGround of DC
	CBrush brBlack(RGB(0, 0, 0));
	pDC->FillRect(pDCRet, &brBlack);

	// Generate Bitmap Buffer
	if (rayTracerDibImage) delete rayTracerDibImage;

	m_rtImageWidth = 1280;
	m_rtImageHeight = 720;
	CreateBitMap(m_rtImageWidth, m_rtImageHeight);
	rayTracerDibImage = new _vPIXEL[m_rtImageWidth*m_rtImageHeight];

	// Calculate Ray Tracing result.
	DWORD  opStart, opFinish;
	Ccg2022QBRayTracerDoc* pDoc = GetDocument();

	opStart = GetTickCount();        // 记录测试开始时间，单位是毫秒

	if (m_mitRayTracer) m_mitRayTracer->runRayTrace(rayTracerDibImage, 4); // pDoc->m_threadNum);

	CClientDC ClientDC(pDC->GetWindow());
	::StretchDIBits(ClientDC.m_hDC,
		0, 0, m_rtImageWidth, m_rtImageHeight,
		0, 0, m_rtImageWidth, m_rtImageHeight,
		rayTracerDibImage, rayTracerBitMapHeader,
		DIB_RGB_COLORS, SRCCOPY);

	opFinish = GetTickCount();       // 记录测试结束时间，单位是毫秒

	pDoc->m_runTime = (opFinish - opStart) / 1000.0f;

//	pDoc->UpdateAllViews(this);

	opBusy = FALSE;

	// Must remember to release prjPlaneCDC every time after using.
	ReleaseDC(pDC);
}

// Smallpt RayTracer Render Processing Functions.
void Ccg2022QBRayTracerView::SmallptRayTraceRender()
{
	static BOOL opBusy = FALSE;

	if (opBusy) return;

	opBusy = TRUE;

	CDC     *pDC;
	CRect    pDCRet;

	pDC = GetDC();
	GetClientRect(&pDCRet);

	// Clear BackGround of DC
	CBrush brBlack(RGB(0, 0, 0));
	pDC->FillRect(pDCRet, &brBlack);

	// Generate Bitmap Buffer
	if (rayTracerDibImage) delete rayTracerDibImage;

	m_rtImageWidth = 512;
	m_rtImageHeight = 384;
	int width = 0 - m_rtImageWidth;
	int height = 0 - m_rtImageHeight;
	//m_rtImageWidth = 1280;
	//m_rtImageHeight = 720;
	CreateBitMap(m_rtImageWidth, m_rtImageHeight);
	rayTracerDibImage = new _vPIXEL[m_rtImageWidth*m_rtImageHeight];

	// Calculate Ray Tracing result.
	DWORD  opStart, opFinish;
	Ccg2022QBRayTracerDoc* pDoc = GetDocument();

	opStart = GetTickCount();        // 记录测试开始时间，单位是毫秒

	if (m_smallptRayTracer) m_smallptRayTracer->smallptRayTrace(m_rtImageWidth, m_rtImageHeight, rayTracerDibImage, 10, 4);

	CClientDC ClientDC(pDC->GetWindow());
	::StretchDIBits(ClientDC.m_hDC,
		0, 0, m_rtImageWidth, m_rtImageHeight,
		0, m_rtImageHeight, m_rtImageWidth, height,
		rayTracerDibImage, rayTracerBitMapHeader,
		DIB_RGB_COLORS, SRCCOPY);

	opFinish = GetTickCount();       // 记录测试结束时间，单位是毫秒

	pDoc->m_runTime = (opFinish - opStart) / 1000.0f;

	pDoc->UpdateAllViews(this);

	opBusy = FALSE;

	// Must remember to release prjPlaneCDC every time after using.
	ReleaseDC(pDC);
}

// smallptCuda RayTracer Render Processing Functions.
extern "C"  int smallptCudaRayTrace(unsigned int *bmpImage);

void Ccg2022QBRayTracerView::SmallptCudaRayTraceRender()
{
	static BOOL opBusy = FALSE;

	if (opBusy) return;

	opBusy = TRUE;

	CDC     *pDC;
	CRect    pDCRet;

	pDC = GetDC();
	GetClientRect(&pDCRet);

	// Clear BackGround of DC
	CBrush brBlack(RGB(0, 0, 0));
	pDC->FillRect(pDCRet, &brBlack);

	// Generate Bitmap Buffer
	if (rayTracerDibImage) delete rayTracerDibImage;

	m_rtImageWidth = 512;
	m_rtImageHeight = 384;
	CreateBitMap(m_rtImageWidth, -m_rtImageHeight);
	rayTracerDibImage = new _vPIXEL[m_rtImageWidth*m_rtImageHeight];

	// Calculate Ray Tracing result.
	DWORD  opStart, opFinish;
	Ccg2022QBRayTracerDoc* pDoc = GetDocument();

	opStart = GetTickCount();        // 记录测试开始时间，单位是毫秒

	smallptCudaRayTrace(rayTracerDibImage);

	CClientDC ClientDC(pDC->GetWindow());
	int sx = pDCRet.right / 2 - m_rtImageWidth / 2;
	int sy = pDCRet.bottom / 2 - m_rtImageHeight / 2;

	::StretchDIBits(ClientDC.m_hDC,
		sx, sy, m_rtImageWidth, m_rtImageHeight,
		0, 0, m_rtImageWidth, m_rtImageHeight,
		rayTracerDibImage, rayTracerBitMapHeader,
		DIB_RGB_COLORS, SRCCOPY);

	opFinish = GetTickCount();       // 记录测试结束时间，单位是毫秒

	pDoc->m_runTime = (opFinish - opStart) / 1000.0f;

//	pDoc->UpdateAllViews(this);

	opBusy = FALSE;

	// Must remember to release prjPlaneCDC every time after using.
	ReleaseDC(pDC);
}