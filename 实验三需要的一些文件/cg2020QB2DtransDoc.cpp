
// cg2020QB2DtransDoc.cpp : Ccg2020QB2DtransDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "cg2020QB2Dtrans.h"
#endif

#include "cg2020QB2DtransDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Ccg2020QB2DtransDoc

IMPLEMENT_DYNCREATE(Ccg2020QB2DtransDoc, CDocument)

BEGIN_MESSAGE_MAP(Ccg2020QB2DtransDoc, CDocument)
END_MESSAGE_MAP()


// Ccg2020QB2DtransDoc ����/����

Ccg2020QB2DtransDoc::Ccg2020QB2DtransDoc()
{
	m_transDir = 0;        // 0:-X  1:+X  2:+Y  3:-Y
	m_transMode = 0;       // 0: translate  1: rotate  2: scale
	m_transSelect = 0;     // 0: Line  1: Polygon  2: Window
   
	// Space Line Initialization
	m_line.p1.x =  30;    m_line.p1.y =  30;      
	m_line.p2.x = 190;    m_line.p2.y = 190;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 2; j++)
			if (i == j) m_line.transMatrix[i][j] = 1.0f;
			else        m_line.transMatrix[i][j] = 0.0f;

	// Space Polygon Initialization
	m_polygon.pNumber = 4;
	m_polygon.points[0].x =   0;  m_polygon.points[0].y =   0;
	m_polygon.points[1].x = 150;  m_polygon.points[1].y = 150;
	m_polygon.points[2].x = 210;  m_polygon.points[2].y =   0;
	m_polygon.points[3].x =   0;  m_polygon.points[3].y =   0;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 2; j++)
			if (i == j) m_polygon.transMatrix[i][j] = 1.0f;
			else        m_polygon.transMatrix[i][j] = 0.0f;


	m_wndLx = 0;
	m_wndLy = 0;
	m_wndRx = 150;
	m_wndRy = 150;
}

Ccg2020QB2DtransDoc::~Ccg2020QB2DtransDoc()
{
}

BOOL Ccg2020QB2DtransDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}




// Ccg2020QB2DtransDoc ���л�

void Ccg2020QB2DtransDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void Ccg2020QB2DtransDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// ������������֧��
void Ccg2020QB2DtransDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:     strSearchContent = _T("point;rectangle;circle;ole object;")��
	SetSearchContent(strSearchContent);
}

void Ccg2020QB2DtransDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// Ccg2020QB2DtransDoc ���

#ifdef _DEBUG
void Ccg2020QB2DtransDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void Ccg2020QB2DtransDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// Ccg2020QB2DtransDoc ����
