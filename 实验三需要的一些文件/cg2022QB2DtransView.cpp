
// cg2022QB2DtransView.cpp : Ccg2022QB2DtransView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "cg2022QB2Dtrans.h"
#endif

#include "cg2022QB2DtransDoc.h"
#include "cg2022QB2DtransView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ccg2022QB2DtransView

IMPLEMENT_DYNCREATE(Ccg2022QB2DtransView, CView)

BEGIN_MESSAGE_MAP(Ccg2022QB2DtransView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// Ccg2022QB2DtransView 构造/析构

Ccg2022QB2DtransView::Ccg2022QB2DtransView()
{
	// TODO: 在此处添加构造代码
	inputOK = FALSE;
	m_pNumbers = 0;

	m_wndWidth = 0;
	m_wndHeight = 0;
}

Ccg2022QB2DtransView::~Ccg2022QB2DtransView()
{
}

BOOL Ccg2022QB2DtransView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// Ccg2022QB2DtransView 绘制

void Ccg2022QB2DtransView::OnDraw(CDC* /*pDC*/)
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	CDC* pDC = GetDC();

	pDC->MoveTo(0, m_wndHeight / 2);
	pDC->LineTo(m_wndWidth, m_wndHeight / 2);
	pDC->MoveTo(m_wndWidth / 2, 0);
	pDC->LineTo(m_wndWidth / 2, m_wndHeight);

	CPen newPen;
	CPen *oldPen;

	newPen.CreatePen(PS_SOLID, 2, RGB(255, 0, 255));
	oldPen = (CPen *)pDC->SelectObject(&newPen);

	pDC->MoveTo(m_wndWidth / 2 + pDoc->m_wndLx, m_wndHeight / 2 - pDoc->m_wndLy);
	pDC->LineTo(m_wndWidth / 2 + pDoc->m_wndLx, m_wndHeight / 2 - pDoc->m_wndRy);
	pDC->LineTo(m_wndWidth / 2 + pDoc->m_wndRx, m_wndHeight / 2 - pDoc->m_wndRy);
	pDC->LineTo(m_wndWidth / 2 + pDoc->m_wndRx, m_wndHeight / 2 - pDoc->m_wndLy);
	pDC->LineTo(m_wndWidth / 2 + pDoc->m_wndLx, m_wndHeight / 2 - pDoc->m_wndLy);

	pDC->SelectObject(oldPen);
	newPen.DeleteObject();

	// First, calculate Space Transform Matrix.
	switch (pDoc->m_transSelect) {
	case 0: // Line
		CalculateMatrix(pDoc->m_line.transMatrix);
		break;
	case 1: // Polyon
		CalculateMatrix(pDoc->m_polygon.transMatrix);
		break;
	}

	// Then transform Space Graph & display.
	CPoint tp1, tp2, transPolygon[N];
	TransLine(&pDoc->m_line.p1, &pDoc->m_line.p2, &tp1, &tp2, pDoc->m_line.transMatrix);
	DisplayLine(pDC, tp1, tp2, RGB(255, 0, 0));

	// Cohn-Sutherland Subdivision Line Clip 
	int cx1, cy1, cx2, cy2;
	pDoc->m_lineVisible = FALSE;
	cx1 = tp1.x;    cy1 = tp1.y;
	cx2 = tp2.x;    cy2 = tp2.y;

	pDoc->m_lineVisible = ClipLine(&cx1, &cy1, &cx2, &cy2);

	if (pDoc->m_lineVisible) {

		pDoc->cp1.x = cx1;    pDoc->cp1.y = cy1;
		pDoc->cp2.x = cx2;    pDoc->cp2.y = cy2;

		DisplayLine(pDC, pDoc->cp1, pDoc->cp2, RGB(0, 255, 0));
	}

	// Processing polygon
	TransPolygon(pDoc->m_polygon.pNumber, pDoc->m_polygon.points,
		         transPolygon, pDoc->m_polygon.transMatrix);
	DisplayPolygon(pDC, pDoc->m_polygon.pNumber, transPolygon, RGB(0, 0, 255));

	// Sutherland-Hodgman Polygon Clip
	pDoc->m_polygonVisible = FALSE;

	if (ClipPolygon(pDoc->m_polygon.pNumber, transPolygon,
		&pDoc->m_clipPolyNum, pDoc->clipPolygon))
	{
		DisplayPolygon(pDC, pDoc->m_clipPolyNum, pDoc->clipPolygon, RGB(0, 255, 255));
		pDoc->m_polygonVisible = TRUE;
	}

	// Must remember to release pDC at processing end.
	ReleaseDC(pDC);

	pDoc->UpdateAllViews(this);
}


// Ccg2022QB2DtransView 打印

BOOL Ccg2022QB2DtransView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void Ccg2022QB2DtransView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void Ccg2022QB2DtransView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// Ccg2022QB2DtransView 诊断

#ifdef _DEBUG
void Ccg2022QB2DtransView::AssertValid() const
{
	CView::AssertValid();
}

void Ccg2022QB2DtransView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Ccg2022QB2DtransDoc* Ccg2022QB2DtransView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Ccg2022QB2DtransDoc)));
	return (Ccg2022QB2DtransDoc*)m_pDocument;
}
#endif //_DEBUG


// Ccg2022QB2DtransView 消息处理程序
void Ccg2022QB2DtransView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	m_wndWidth = cx;
	m_wndHeight = cy;
}

#define DELTAX 10
#define DELTAY 10
#define DELTATHETA 3.1415926f/36

void Ccg2022QB2DtransView::CalculateMatrix(float transMatrix[3][2])
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	switch (pDoc->m_transDir) {
	case 0: // -X
		switch (pDoc->m_transMode) {
		case 0: // Move
			TranslateMatrix(-DELTAX, 0, transMatrix);
			break;
		case 1: // rotate
			RotateMatrix(-sinf(DELTATHETA), cosf(DELTATHETA), transMatrix);
			break;
		case 2: // Scale

				// Fill right code here.
			break;
		}
		break;
	case 1: // +X
		switch (pDoc->m_transMode) {
		case 0: // Move
			TranslateMatrix(DELTAX, 0, transMatrix);
			break;
		case 1: // rotate
			RotateMatrix(sinf(DELTATHETA), cosf(DELTATHETA), transMatrix);
			break;
		case 2: // Scale

				// Fill right code here.
			break;
		}
		break;
	case 2: // +Y
		switch (pDoc->m_transMode) {
		case 0: // Move
			TranslateMatrix(0, DELTAY, transMatrix);
			break;
		case 1: // rotate
			RotateMatrix(sinf(DELTATHETA), cosf(DELTATHETA), transMatrix);
			break;
		case 2: // Scale

				// Fill right code here.
			break;
		}
		break;
	case 3: // -Y
		switch (pDoc->m_transMode) {
		case 0: // Move
			TranslateMatrix(0, -DELTAY, transMatrix);
			break;
		case 1: // rotate
			RotateMatrix(-sinf(DELTATHETA), cosf(DELTATHETA), transMatrix);
			break;
		case 2: // Scale

				// Fill right code here.
			break;
		}
		break;
	}
}

void Ccg2022QB2DtransView::RotateMatrix(float S, float C, float m[3][2])
{
	float temp;

	for (int i = 0; i < 3; i++) {

		// Fill right code here......
	}
}

void Ccg2022QB2DtransView::TranslateMatrix(float Tx, float Ty, float m[3][2])
{
	m[2][0] += Tx;
	m[2][1] += Ty;
}

void Ccg2022QB2DtransView::ScaleMatrix(float Sx, float Sy, float m[3][2])
{
	for (int i = 0; i < 3; i++) {

		// Fill right code here......
	}
}

void Ccg2022QB2DtransView::TransLine(CPoint *p1, CPoint *p2,
	                                CPoint *tp1, CPoint *tp2,
	                                float transMatrix[3][2])
{
	tp1->x = (int)// Fill right code here......;
	tp1->y = (int)// Fill right code here......;

	tp2->x = (int)// Fill right code here......;
	tp2->y = (int)// Fill right code here......;
}

void Ccg2022QB2DtransView::DisplayLine(CDC* pDC, CPoint p1, CPoint p2, COLORREF rgbColor)
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	CPen newPen;
	CPen *oldPen;
	CPoint VP1, VP2;

	newPen.CreatePen(PS_SOLID, 2, rgbColor);
	oldPen = (CPen *)pDC->SelectObject(&newPen);

	VP1.x = m_wndWidth / 2 + p1.x;
	VP1.y = m_wndHeight / 2 - p1.y;
	VP2.x = m_wndWidth / 2 + p2.x;
	VP2.y = m_wndHeight / 2 - p2.y;

	pDC->MoveTo(VP1);
	pDC->LineTo(VP2);

	pDC->SelectObject(oldPen);
	newPen.DeleteObject();
}

void Ccg2022QB2DtransView::TransPolygon(int pointNumber, CPoint spPolygon[N],
	CPoint transPolygon[N], float transMatrix[3][2])
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	for (int i = 0; i < pointNumber; i++) {
		transPolygon[i].x = // Fill right code here......;
		transPolygon[i].y = // Fill right code here......;

	}
}

void Ccg2022QB2DtransView::DisplayPolygon(CDC* pDC, int pointNumber,
	CPoint transPolygon[N], COLORREF rgbColor)
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	CPen newPen;
	CPen *oldPen;
	CPoint VPolygon[N];

	newPen.CreatePen(PS_SOLID, 2, rgbColor);
	oldPen = (CPen *)pDC->SelectObject(&newPen);

	for (int i = 0; i < pointNumber; i++) {
		VPolygon[i].x = m_wndWidth / 2 + transPolygon[i].x;
		VPolygon[i].y = m_wndHeight / 2 - transPolygon[i].y;
	}

	pDC->MoveTo(VPolygon[0]);
	for (int i = 1; i < pointNumber; i++) pDC->LineTo(VPolygon[i]);

	pDC->SelectObject(oldPen);
	newPen.DeleteObject();
}

// Cohn-Sutherland Subdivision Line Clip
int  Ccg2022QB2DtransView::ClipLine(int *x1, int *y1, int *x2, int *y2)

{
	int visible, m_window[4];
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	m_window[0] = pDoc->m_wndLx;    m_window[1] = pDoc->m_wndRx;
	m_window[2] = pDoc->m_wndRy;    m_window[3] = pDoc->m_wndLy;

	for (int i = 0; i < 4; i++) { // Along the WIN Border

		visible = LineVisible(x1, y1, x2, y2);
		if (visible == 1) return 1;         // Total Visible
		if (visible == 0) return 0;         // Total Unvisible

		if (LineCross(*x1, *y1, *x2, *y2, i)) {

			// Please fill in the right code here ...
			if (i < 2 && *x2 - *x1) {                       // Left , Right

				// Fill right code here......

			} else if (*y2 - *y1) {                         // Top    Bottom
															
				// Fill right code here......

			}
		}
	}
	return 1;
}

int Ccg2022QB2DtransView::LineVisible(int *x1, int *y1, int *x2, int *y2)
{
	int pcode1, pcode2;

	pcode1 = pCode(x1, y1);
	pcode2 = pCode(x2, y2);

	if (!pcode1 && !pcode2)    return 1;     // Visible
	if ((pcode1&pcode2) != 0)  return 0;     // Unvisible
	if (pcode1 == 0) {
		float temp;
		temp = *x1;  *x1 = *x2;  *x2 = temp;
		temp = *y1;  *y1 = *y2;  *y2 = temp;
	}
	return 2;
}

int Ccg2022QB2DtransView::pCode(int *x, int *y)
{
	int code = 0;
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	if (*x <= pDoc->m_wndLx)  code |= 1;
	if (*x >= pDoc->m_wndRx)  code |= 2;
	if (*y >= pDoc->m_wndRy)  code |= 4;
	if (*y <= pDoc->m_wndLy)  code |= 8;

	return code;
}

int Ccg2022QB2DtransView::LineCross(int x1, int y1, int x2, int y2, int i)
{
	int visible1, visible2;

	visible1 = pVisible(x1, y1, i);
	visible2 = pVisible(x2, y2, i);

	if (visible1 != visible2) return 1;
	else                      return 0;

}

int Ccg2022QB2DtransView::pVisible(int x, int y, int i)
{
	int visible = 0;
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	switch (i) {
	case 0: // Left
		if (x >= pDoc->m_wndLx)  visible = 1; break;
	case 1: // Right
		if (x <= pDoc->m_wndRx)  visible = 1; break;
	case 2: // Top
		if (y <= pDoc->m_wndRy)  visible = 1; break;
	case 3: // Bottom
		if (y >= pDoc->m_wndLy)  visible = 1; break;
	}
	return visible;
}

// Sutherland-Hodgman Polygon Clip
int Ccg2022QB2DtransView::ClipPolygon(int n, CPoint *tPoints, int *cn, CPoint *cPoints)
{
	int Nin, Nout, ix, iy, Sx, Sy;
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	Nin = n;
	for (int i = 0; i < 4; i++) {  // Along the window border
		*cn = 0;
		for (int j = 0; j < Nin; j++) {  // Scan polygon every point and line.
			if (j > 0) {
				if (LineCross(Sx, Sy, tPoints[j].x, tPoints[j].y, i)) {
					interSect(Sx, Sy, tPoints[j].x, tPoints[j].y, i, &ix, &iy);
					outPut(ix, iy, cn, cPoints);
				}
			}
			Sx = tPoints[j].x;
			Sy = tPoints[j].y;
			if (pVisible(Sx, Sy, i)) outPut(Sx, Sy, cn, cPoints);
		}

		Nin = *cn;
		if (*cn == 0) return 0;
		for (int j = 0; j < Nin; j++) {
			tPoints[j].x = cPoints[j].x;
			tPoints[j].y = cPoints[j].y;
		}

		if (cPoints[0].x != cPoints[Nin - 1].x ||
			cPoints[0].y != cPoints[Nin - 1].y) {

			tPoints[Nin].x = cPoints[Nin].x = cPoints[0].x;
			tPoints[Nin].y = cPoints[Nin].y = cPoints[0].y;

			Nin++;
			*cn = Nin;
		}
	}
	return 1;
}

void Ccg2022QB2DtransView::interSect(int Sx, int  Sy, int Px, int Py,
	int  i, int *ix, int *iy)
{
	Ccg2022QB2DtransDoc* pDoc = GetDocument();

	// Please fill in the right code here ...
	switch (i) {
	case 0: // Left
		*ix = pDoc->m_wndLx;
		*iy = 	// Fill right code here......;
		break;
	case 1: // Right
		*ix = pDoc->m_wndRx;
		*iy = // Fill right code here......;
		break;
	case 2: // Top
		*iy = pDoc->m_wndRy;
		*ix = // Fill right code here......;
		break;
	case 3: // Bottom
		*iy = pDoc->m_wndLy;
		*ix = // Fill right code here......;
		break;
	}
}

void Ccg2022QB2DtransView::outPut(int x, int y, int *cn, CPoint *cPoints)
{
	cPoints[*cn].x = x;
	cPoints[*cn].y = y;
	(*cn)++;
}

