// CgControlSelect.cpp : 实现文件
//

#include "stdafx.h"
#include "cg2022QBPolyFill.h"
#include "CgControlSelect.h"

#include "cg2022QBPolyFillDoc.h"

// CCgControlSelect

IMPLEMENT_DYNCREATE(CCgControlSelect, CFormView)

CCgControlSelect::CCgControlSelect()
	: CFormView(IDD_CONTROLSELECT)
{

}

CCgControlSelect::~CCgControlSelect()
{
}

void CCgControlSelect::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCgControlSelect, CFormView)
	ON_BN_CLICKED(IDC_POLYFILL, &CCgControlSelect::OnClickedPolyfill)
	ON_BN_CLICKED(IDC_SEEDFILL, &CCgControlSelect::OnClickedSeedfill)
END_MESSAGE_MAP()


// CCgControlSelect 诊断

#ifdef _DEBUG
void CCgControlSelect::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CCgControlSelect::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCgControlSelect 消息处理程序


void CCgControlSelect::OnClickedPolyfill()
{
	Ccg2022QBPolyFillDoc* pDoc = (Ccg2022QBPolyFillDoc*)GetDocument();

	pDoc->m_opSelect = 0;
}


void CCgControlSelect::OnClickedSeedfill()
{
	Ccg2022QBPolyFillDoc* pDoc = (Ccg2022QBPolyFillDoc*)GetDocument();

	pDoc->m_opSelect = 0;
}
