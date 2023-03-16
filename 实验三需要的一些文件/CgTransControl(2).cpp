// CgTransControl.cpp : 实现文件
//

#include "stdafx.h"
#include "cg2022QB2Dtrans.h"
#include "CgTransControl.h"

#include "cg2022QB2DtransDoc.h"

// CCgTransControl

IMPLEMENT_DYNCREATE(CCgTransControl, CFormView)

CCgTransControl::CCgTransControl()
	: CFormView(IDD_TRANSCONTROL)
	, m_transSelect(0)
{

}

CCgTransControl::~CCgTransControl()
{
}

void CCgTransControl::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_TRANSSELECT, m_transSelect);
}

BEGIN_MESSAGE_MAP(CCgTransControl, CFormView)
	ON_BN_CLICKED(IDC_XLEFT, &CCgTransControl::OnClickedXleft)
	ON_BN_CLICKED(IDC_XRIGHT, &CCgTransControl::OnClickedXright)
	ON_BN_CLICKED(IDC_YUP, &CCgTransControl::OnClickedYup)
	ON_BN_CLICKED(IDC_YDOWN, &CCgTransControl::OnClickedYdown)
	ON_BN_CLICKED(IDC_TRANSMODE, &CCgTransControl::OnClickedTransmode)
	ON_CBN_SELCHANGE(IDC_TRANSSELECT, &CCgTransControl::OnSelchangeTransselect)
END_MESSAGE_MAP()


// CCgTransControl 诊断

#ifdef _DEBUG
void CCgTransControl::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CCgTransControl::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCgTransControl 消息处理程序

#define WND_DELTAX 10
#define WND_DELTAY 10

void CCgTransControl::OnClickedXleft()
{
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	if (m_transSelect == 2) {
		pDoc->m_wndLx -= WND_DELTAX;
		pDoc->m_wndRx -= WND_DELTAX;
	}

	pDoc->m_transDir = 0;

	pDoc->UpdateAllViews(this);
}


void CCgTransControl::OnClickedXright()
{
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	if (m_transSelect == 2) {
		pDoc->m_wndLx += WND_DELTAX;
		pDoc->m_wndRx += WND_DELTAX;
	}

	pDoc->m_transDir = 1;

	pDoc->UpdateAllViews(this);
}


void CCgTransControl::OnClickedYup()
{
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	if (m_transSelect == 2) {
		pDoc->m_wndLy += WND_DELTAY;
		pDoc->m_wndRy += WND_DELTAY;
	}

	pDoc->m_transDir = 2;

	pDoc->UpdateAllViews(this);
}


void CCgTransControl::OnClickedYdown()
{
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	if (m_transSelect == 2) {
		pDoc->m_wndLy -= WND_DELTAY;
		pDoc->m_wndRy -= WND_DELTAY;
	}

	pDoc->m_transDir = 3;

	pDoc->UpdateAllViews(this);
}


void CCgTransControl::OnClickedTransmode()
{
	CButton *pButton = (CButton *)GetDlgItem(IDC_TRANSMODE);
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	pDoc->m_transMode++;
	if (pDoc->m_transMode > 2) pDoc->m_transMode = 0;

	switch (pDoc->m_transMode) {
	case 0: pButton->SetWindowText(_T("Translate"));  break;
	case 1: pButton->SetWindowText(_T("Rotate"));     break;
	case 2: pButton->SetWindowText(_T("Scale"));      break;
	}
}


void CCgTransControl::OnSelchangeTransselect()
{
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	UpdateData(TRUE);

	pDoc->m_transSelect = m_transSelect;

	UpdateData(FALSE);
}


void CCgTransControl::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CStatic *viewPlane = (CStatic *)GetDlgItem(IDC_VIEWPORT);
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	CDC     *viewPlaneCDC;
	CRect    viewPlaneRect;

	viewPlaneCDC = viewPlane->GetDC();
	viewPlane->GetClientRect(&viewPlaneRect);

	// Clear BackGround of DC
	CBrush brBlack(RGB(0, 0, 0));
	viewPlaneCDC->FillRect(viewPlaneRect, &brBlack);

	if (pDoc->m_lineVisible) ViewTransLine(viewPlaneCDC, viewPlaneRect);
	if (pDoc->m_polygonVisible) ViewTransPolygon(viewPlaneCDC, viewPlaneRect);

	// Must remember to release viewPlaneCDC at processing end.
	ReleaseDC(viewPlaneCDC);
}

void CCgTransControl::ViewTransLine(CDC* pDC, CRect dcRect)
{
	CPen newPen;
	CPen *oldPen;
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	// Create new red-color pen to Draw Clipping Line
	newPen.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	oldPen = (CPen *)pDC->SelectObject(&newPen);

	int vx1, vy1, vx2, vy2;

	int wndWidth = pDoc->m_wndRx - pDoc->m_wndLx;
	int wndHeight = pDoc->m_wndRy - pDoc->m_wndLy;

	float Sx = (float)(dcRect.right) / wndWidth;
	float Sy = (float)(dcRect.bottom) / wndHeight;

	// Please fill in the right code here ...

	vx1 = // Fill right code here......;
	vy1 = // Fill right code here......;
	vx2 = // Fill right code here......;
	vy2 = // Fill right code here......;

	pDC->MoveTo(vx1, dcRect.bottom - vy1);
	pDC->LineTo(vx2, dcRect.bottom - vy2);

	// Remember: must delete newPen every time.	
	pDC->SelectObject(oldPen);
	newPen.DeleteObject();
}

void CCgTransControl::ViewTransPolygon(CDC* pDC, CRect dcRect)
{
	CPen newPen;
	CPen *oldPen;
	Ccg2022QB2DtransDoc* pDoc = (Ccg2022QB2DtransDoc*)GetDocument();

	// Create new red-color pen to Draw Clipping Line
	newPen.CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
	oldPen = (CPen *)pDC->SelectObject(&newPen);

	CPoint m_VPolygon[N];

	int wndWidth = pDoc->m_wndRx - pDoc->m_wndLx;
	int wndHeight = pDoc->m_wndRy - pDoc->m_wndLy;

	float Sx = (float)(dcRect.right) / wndWidth;
	float Sy = (float)(dcRect.bottom) / wndHeight;

	for (int i = 0; i < pDoc->m_clipPolyNum; i++) {

		// Please fill in the right code here ...
		m_VPolygon[i].x = // Fill right code here......;
		m_VPolygon[i].y = // Fill right code here......;
	}

	pDC->MoveTo(m_VPolygon[0]);
	for (int i = 1; i < pDoc->m_clipPolyNum; i++) pDC->LineTo(m_VPolygon[i]);

	// Remember: must delete newPen every time.	
	pDC->SelectObject(oldPen);
	newPen.DeleteObject();
}