#pragma once

#define N 64

#include "cg2022QB3DtransDoc.h"

// CCgTransControl 窗体视图

class CCgTransControl : public CFormView
{
	DECLARE_DYNCREATE(CCgTransControl)

protected:
	CCgTransControl();           // 动态创建所使用的受保护的构造函数
	virtual ~CCgTransControl();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRANSCONTROL };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedXleft();
	afx_msg void OnClickedXright();
	afx_msg void OnClickedYup();
	afx_msg void OnClickedYdown();
	afx_msg void OnClickedZfront();
	afx_msg void OnClickedZback();
	int m_transSelect;
	afx_msg void OnSelchangeTransselect();
	afx_msg void OnClickedTransmode();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

private:
	int      m_objNumber;
	Object_p m_whoObject;                   // Object pointer
	COLORREF vcObjColor[SPACEOBJECTS];

	int   prjPlaneRetSize;       // Z-Buffer Array Size
	float *prjPlaneRetZ;         // Z-Buffer Array Memory

	void pTransToZbuffer(CRect dcRect);
	void pDrawLineObject(CDC *pDC, CRect dcRect);
	void pDrawLightObject(CDC *pDC, CRect dcRect, float maxShade, float minShade);
	void pDrawShadeLightObject(CDC *pDC, CRect dcRect, float maxShade, float minShade);

	int polyCount;
	BOOL m_bitmapOutput;
	float ymax[N], ymin[N];
	int ibegin, iend, scan, pdges;
	float Dx[N], Xa[N], Sc[N], Dc[N];

	void FillPolygon(CDC *pDC, int n, int *x, int *y, int *color, CRect dcRect);
	void Loadpolygon(int n, int *x, int *y, int *color);
	void PolyInsert(float x1, float y1, float x2, float y2, int c1, int c2);
	void UpdateXvalue();
	void XSort(int begin, int i);
	void Fillscan(CDC *pDC, CRect dcRect);
	void Include();

	// Interface for remove hidden part by Z-buffer Method. 
	float CalculateZValue(int x, int y, CRect dcRect, int i);

	//_VPIXEL* prjPlaneDibImage;
	int m_imageWidth, m_imageHeight;
	//BITMAPINFO* DisplayBitMapHeader;
	//char DisplayBitMapBuffer[sizeof(BITMAPINFO) + 16];

	void InitPrjPlaneData(int width, int height);
	//void CreateDisplayBitMap(int width, int height);
	//void _SetPixel(int x, int y, int red, int green, int blue);
};


