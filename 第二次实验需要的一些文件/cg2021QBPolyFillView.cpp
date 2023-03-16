
// cg2021QBPolyFillView.cpp : Ccg2021QBPolyFillView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "cg2021QBPolyFill.h"
#endif

#include "cg2021QBPolyFillDoc.h"
#include "cg2021QBPolyFillView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ccg2021QBPolyFillView
static const int m_patternData[7][8] = {
	0,0,0,1,0,0,0,
	0,0,1,0,1,0,0,
	0,1,0,0,0,1,0,
	1,0,0,0,0,0,1,
	1,1,1,1,1,1,1,
	1,0,0,0,0,0,1,
	1,0,0,0,0,0,1,
	1,0,0,0,0,0,1,
};

IMPLEMENT_DYNCREATE(Ccg2021QBPolyFillView, CView)

BEGIN_MESSAGE_MAP(Ccg2021QBPolyFillView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Ccg2021QBPolyFillView 构造/析构

Ccg2021QBPolyFillView::Ccg2021QBPolyFillView()
{
	// TODO: 在此处添加构造代码
	m_pDC = NULL;
	m_pNumbers = 0;
}

Ccg2021QBPolyFillView::~Ccg2021QBPolyFillView()
{
}

BOOL Ccg2021QBPolyFillView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// Ccg2021QBPolyFillView 绘制

void Ccg2021QBPolyFillView::OnDraw(CDC* /*pDC*/)
{
	Ccg2021QBPolyFillDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// Ccg2021QBPolyFillView 打印

BOOL Ccg2021QBPolyFillView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void Ccg2021QBPolyFillView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void Ccg2021QBPolyFillView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// Ccg2021QBPolyFillView 诊断

#ifdef _DEBUG
void Ccg2021QBPolyFillView::AssertValid() const
{
	CView::AssertValid();
}

void Ccg2021QBPolyFillView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Ccg2021QBPolyFillDoc* Ccg2021QBPolyFillView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Ccg2021QBPolyFillDoc)));
	return (Ccg2021QBPolyFillDoc*)m_pDocument;
}
#endif //_DEBUG


// Ccg2021QBPolyFillView 消息处理程序


void Ccg2021QBPolyFillView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_pNumbers < N) {
		m_mousePoint = point;
		m_pAccord[m_pNumbers] = point;
		m_pNumbers++;
	}

	CView::OnLButtonDown(nFlags, point);
}


void Ccg2021QBPolyFillView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	Ccg2021QBPolyFillDoc* pDoc = GetDocument();

	m_pDC->MoveTo(m_pAccord[m_pNumbers - 1]);
	m_pDC->LineTo(m_pAccord[0]);

	m_pAccord[m_pNumbers] = m_pAccord[0];
	m_pNumbers++;

	if (pDoc->m_opSelect)
		Fillpolygon(m_pNumbers, m_pAccord, m_pDC);
	else
		SeedFill();

	m_pNumbers = 0;

	CView::OnLButtonDblClk(nFlags, point);
}


void Ccg2021QBPolyFillView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_pNumbers) {

		m_pDC->SetROP2(2);
		m_pDC->MoveTo(m_pAccord[m_pNumbers - 1]);
		m_pDC->LineTo(m_mousePoint);

		m_mousePoint = point;
		m_pDC->MoveTo(m_pAccord[m_pNumbers - 1]);
		m_pDC->LineTo(m_mousePoint);
	}

	CView::OnMouseMove(nFlags, point);
}


int Ccg2021QBPolyFillView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDC = new CClientDC(this);

	return 0;
}

void Ccg2021QBPolyFillView::Fillpolygon(int pNumbers, CPoint *points, CDC *pDC)
{
	m_edgeNumbers = 0;
	pLoadPolygon(pNumbers, points);

	m_Begin = m_End = 0;
	m_Scan = (int)m_yMax[0];
	pInclude();
	pUpdateXvalue();
	while (m_Begin != m_End) {
		pFillScan(pDC);
		m_Scan--;
		pInclude();
		pUpdateXvalue();
	}
}

void Ccg2021QBPolyFillView::pLoadPolygon(int pNumbers, CPoint *points)
{
	float x1, y1, x2, y2;

	x1 = points[0].x;    y1 = points[0].y + 0.5;
	for (int i = 1; i < pNumbers; i++) {
		x2 = points[i].x;    y2 = points[i].y + 0.5;
		if (y1 - y2) pInsertLine(x1, y1, x2, y2);
		x1 = x2;      y1 = y2;
	}
}

void Ccg2021QBPolyFillView::pInsertLine(float x1, float y1, float x2, float y2)
{
	int i;
	float Ymax, Ymin;

	Ymax = (y2 > y1) ? y2 : y1;
	Ymin = (y2 < y1) ? y2 : y1;
	i = m_edgeNumbers;
	while (i > 0 && Ymax > m_yMax[i - 1]) {
		m_yMax[i] = m_yMax[i - 1];		m_yMin[i] = m_yMin[i - 1];
		m_Xa[i] = m_Xa[i - 1];   		m_Dx[i] = m_Dx[i - 1];
		i--;
	}
	m_yMax[i] = Ymax;   	m_yMin[i] = Ymin;
	if (y2 > y1) m_Xa[i] = x2;
	else         m_Xa[i] = x1;
	m_Dx[i] = (x2 - x1) / (y2 - y1);   	m_edgeNumbers++;
}

void Ccg2021QBPolyFillView::pInclude()
{

	while (m_End < m_edgeNumbers && m_yMax[m_End] > m_Scan) {
		m_Xa[m_End] = m_Xa[m_End] - 0.5*m_Dx[m_End];
		m_Dx[m_End] = -m_Dx[m_End];
		m_End++;
	}
}

void Ccg2021QBPolyFillView::pUpdateXvalue()
{
	int i, start = m_Begin;

	for (i = start; i < m_End; i++) {
		if (m_Scan > m_yMin[i]) {
			m_Xa[i] += m_Dx[i];
			pXsort(m_Begin, i);
		}
		else {
			for (int j = i; j > m_Begin; j--) {
				m_yMin[i] = m_yMin[i - 1];
				m_Xa[i] = m_Xa[i - 1];
				m_Dx[i] = m_Dx[i - 1];
			}
			m_Begin++;
		}
	}
}

void Ccg2021QBPolyFillView::pXsort(int Begin, int i)
{
	float temp;

	while (i > Begin && m_Xa[i] < m_Xa[i - 1]) {
		temp = m_Xa[i];   m_Xa[i] = m_Xa[i - 1];   m_Xa[i - 1] = temp;
		temp = m_Dx[i];   m_Dx[i] = m_Dx[i - 1];   m_Dx[i - 1] = temp;
		temp = m_yMin[i]; m_yMin[i] = m_yMin[i - 1]; m_yMin[i - 1] = temp;
		i--;
	}
}

void Ccg2021QBPolyFillView::pFillScan(CDC* pDC)
{
	int x, y;

	//	pDC->SetROP2(10);
	for (int i = m_Begin; i < m_End; i += 2) {
		//pDC->MoveTo(m_Xa[i], m_Scan);
		//pDC->LineTo(m_Xa[i + 1], m_Scan);
		y = m_Scan;
		for (int x = m_Xa[i]; x < m_Xa[i + 1]; x++)
			if (m_patternData[y % 7][x % 8])
				pDC->SetPixel(x, y, RGB(255, 0, 0));

	}
}

void Ccg2021QBPolyFillView::SeedFill()
{

}