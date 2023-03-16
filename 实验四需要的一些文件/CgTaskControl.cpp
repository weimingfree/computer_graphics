// CgTaskControl.cpp : 实现文件
//

#include "stdafx.h"
#include "cg2022QB3DTrans.h"
#include "CgTaskControl.h"

#include "cg2022QB3DTransDoc.h"

// CCgTaskControl

IMPLEMENT_DYNCREATE(CCgTaskControl, CFormView)

CCgTaskControl::CCgTaskControl()
	: CFormView(IDD_TASKCONTROL)
	, m_transSelect(0)
{
	vcObjColor[BALL] = RGB(255, 0, 0);
	vcObjColor[CUBE] = RGB(255, 255, 0);
	vcObjColor[TRIANGLE] = RGB(0, 0, 255);
}

CCgTaskControl::~CCgTaskControl()
{
}

void CCgTaskControl::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_TRANSSELECT, m_transSelect);
}

BEGIN_MESSAGE_MAP(CCgTaskControl, CFormView)
	ON_CBN_SELCHANGE(IDC_TRANSSELECT, &CCgTaskControl::OnSelchangeTransselect)
	ON_BN_CLICKED(IDC_TRANSMODE, &CCgTaskControl::OnClickedTransmode)
	ON_BN_CLICKED(IDC_XLEFT, &CCgTaskControl::OnClickedXleft)
	ON_BN_CLICKED(IDC_XRIGHT, &CCgTaskControl::OnClickedXright)
	ON_BN_CLICKED(IDC_YUP, &CCgTaskControl::OnClickedYup)
	ON_BN_CLICKED(IDC_YDOWN, &CCgTaskControl::OnClickedYdown)
	ON_BN_CLICKED(IDC_ZFRONT, &CCgTaskControl::OnClickedZfront)
	ON_BN_CLICKED(IDC_ZBACK, &CCgTaskControl::OnClickedZback)
END_MESSAGE_MAP()


// CCgTaskControl 诊断

#ifdef _DEBUG
void CCgTaskControl::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CCgTaskControl::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCgTaskControl 消息处理程序
#define OFFSET      0.5f
#define DELTAANGLE  5.0f

void CCgTaskControl::OnSelchangeTransselect()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	UpdateData(TRUE);

	pDoc->m_transSelect = m_transSelect;

	UpdateData(FALSE);
}


void CCgTaskControl::OnClickedTransmode()
{
	CButton *pButton = (CButton *)GetDlgItem(IDC_TRANSMODE);
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	pDoc->m_transMode++;
	if (pDoc->m_transMode > 1) pDoc->m_transMode = 0;

	if (pDoc->m_transMode == 0)  pButton->SetWindowText(_T("Translate"));
	else                         pButton->SetWindowText(_T("Rotate"));
}


void CCgTaskControl::OnClickedXleft()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[0] -= OFFSET;
		else                        pDoc->m_xAngle -= DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightX[0] -= OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightX[1] -= OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {    // Eye Trans
		pDoc->eyeX -= OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else pDoc->m_transDir = 0;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnClickedXright()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[0] += OFFSET;
		else                        pDoc->m_xAngle += DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightX[0] += OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightX[1] += OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {      // Eye Trans
		pDoc->eyeX += OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else pDoc->m_transDir = 1;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnClickedYup()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[1] += OFFSET;
		else                        pDoc->m_yAngle += DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightY[0] += OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightY[1] += OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {      // Eye Trans
		pDoc->eyeY += OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else	pDoc->m_transDir = 3;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnClickedYdown()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[1] -= OFFSET;
		else                        pDoc->m_yAngle -= DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightY[0] -= OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightY[1] -= OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {      // Eye Trans
		pDoc->eyeY -= OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else	pDoc->m_transDir = 2;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnClickedZfront()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[2] += OFFSET;
		else                        pDoc->m_zAngle += DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightZ[0] += OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightZ[1] += OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {      // Eye Trans
		pDoc->eyeZ += OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else	pDoc->m_transDir = 4;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnClickedZback()
{
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (pDoc->m_transSelect == SCENE) {           // Scene Trans
		if (pDoc->m_transMode == 0) pDoc->m_translateVector[2] -= OFFSET;
		else                        pDoc->m_zAngle -= DELTAANGLE;
	}
	else if (pDoc->m_transSelect == LIGHT0) {     // Light0 Trans
		pDoc->lightZ[0] -= OFFSET;
	}
	else if (pDoc->m_transSelect == LIGHT1) {   // Light1 Trans
		pDoc->lightZ[1] -= OFFSET;
	}
	else if (pDoc->m_transSelect == EYE) {      // Eye Trans
		pDoc->eyeZ -= OFFSET;
		//		pDoc->pCreateClipBox();
	}
	else	pDoc->m_transDir = 5;

	pDoc->UpdateAllViews(this);
}


void CCgTaskControl::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CDC     *prjPlaneCDC;
	CRect    prjPlaneRet;
	static BOOL first = TRUE;
	CStatic *prjPlane = (CStatic *)GetDlgItem(IDC_VIEWPORT);
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	AssertValid();

	DWORD  opStart, opFinish;

	opStart = GetTickCount();       // 记录测试开始时间，单位是毫秒

	prjPlaneCDC = prjPlane->GetDC();
	prjPlane->GetClientRect(&prjPlaneRet);

	// Clear BackGround of DC
	CBrush brBlack(RGB(0, 0, 0));
	prjPlaneCDC->FillRect(prjPlaneRet, &brBlack);

	for (m_objNumber = pDoc->m_objectNum - 1; m_objNumber >= 0; m_objNumber--) {

		m_whoObject = &pDoc->m_spaceObjects[m_objNumber];

		pTransToZbuffer(prjPlaneRet);
		pDrawLineObject(prjPlaneCDC, prjPlaneRet);
	}

	// Must remember to release prjPlaneCDC every time after using.
	ReleaseDC(prjPlaneCDC);
}

void CCgTaskControl::pTransToZbuffer(CRect dcRect)
{
	int i, j;
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	float vxScale = (float)dcRect.right / (pDoc->winRx - pDoc->winLx);
	float vyScale = (float)dcRect.bottom / (pDoc->winRy - pDoc->winLy);
	for (i = 0; i < m_whoObject->polyCount; i++) {
//		if (m_whoObject->objectSpace[i].polyVisible) {
			for (j = 0; j < m_whoObject->objectSpace[i].polyCount; j++) {
				m_whoObject->objectSpace[i].zBufferObject[j].x =
					(int)((m_whoObject->objectSpace[i].projectObject[j].x - pDoc->winLx) *	vxScale + 0.5f);
				m_whoObject->objectSpace[i].zBufferObject[j].y = dcRect.bottom -
					(int)((m_whoObject->objectSpace[i].projectObject[j].y - pDoc->winLy) *	vyScale + 0.5f);
			}
//		}
	}
}

void CCgTaskControl::pDrawLineObject(CDC *pDC, CRect dcRect)
{
	CPen newPen;
	CPen *oldPen;
	int i, j, x1, y1, x2, y2;
	Ccg2022QB3DTransDoc* pDoc = (Ccg2022QB3DTransDoc*)GetDocument();

	if (m_objNumber <= TRIANGLE)
		newPen.CreatePen(PS_SOLID, 1, vcObjColor[m_objNumber]);
	else
		newPen.CreatePen(PS_SOLID, 1, RGB(255, 255, 0));

	oldPen = (CPen *)pDC->SelectObject(&newPen);

	// Draw Object in DC
	//CRgn clipRgn;
	//clipRgn.CreateRectRgn(dcRect.left, dcRect.top, dcRect.right, dcRect.bottom);
	//pDC->SelectClipRgn(&clipRgn);

	for (i = 0; i < m_whoObject->polyCount; i++) {
//		if (m_whoObject->objectSpace[i].polyVisible) {
			x1 = m_whoObject->objectSpace[i].zBufferObject[0].x;
			y1 = m_whoObject->objectSpace[i].zBufferObject[0].y;
			for (j = 1; j < m_whoObject->objectSpace[i].polyCount; j++) {
				x2 = m_whoObject->objectSpace[i].zBufferObject[j].x;
				y2 = m_whoObject->objectSpace[i].zBufferObject[j].y;
				// draw a line
				pDC->MoveTo(x1, y1);
				pDC->LineTo(x2, y2);
				x1 = x2;
				y1 = y2;
			}
//		}
	}

	pDC->SelectObject(oldPen);
	newPen.DeleteObject();
}