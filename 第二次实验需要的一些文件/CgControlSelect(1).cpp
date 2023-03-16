// CgControlSelect.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "cg2022QBPolyFill.h"
#include "CgControlSelect.h"

#include "cg2022QBPolyFillDoc.h"

// CCgControlSelect

IMPLEMENT_DYNCREATE(CCgControlSelect, CFormView)

CCgControlSelect::CCgControlSelect()
	: CFormView(IDD_CONTROLSELECT)
	, m_opSelect(_T("��ǰ����:None"))
{

}

CCgControlSelect::~CCgControlSelect()
{
}

void CCgControlSelect::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OPSTRING, m_opSelect);
}

BEGIN_MESSAGE_MAP(CCgControlSelect, CFormView)
	ON_BN_CLICKED(IDC_POLYFILL, &CCgControlSelect::OnClickedPolyfill)
	ON_BN_CLICKED(IDC_SEEDFILL, &CCgControlSelect::OnClickedSeedfill)
END_MESSAGE_MAP()


// CCgControlSelect ���

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


// CCgControlSelect ��Ϣ�������


void CCgControlSelect::OnClickedPolyfill()
{
	Ccg2022QBPolyFillDoc* pDoc = (Ccg2022QBPolyFillDoc*)GetDocument();

	pDoc->m_opSelect = 0;

	UpdateData(TRUE);

	m_opSelect.Format(_T("��ǰ����:��������\r\n")); 

//	m_taskPrompt.Format(_T("Sum: %ld Time:%.5f\r\n"), sum); m_taskPrompt += _T("\r\n");

	UpdateData(FALSE);
}


void CCgControlSelect::OnClickedSeedfill()
{
	Ccg2022QBPolyFillDoc* pDoc = (Ccg2022QBPolyFillDoc*)GetDocument();

	pDoc->m_opSelect = 1;
}
