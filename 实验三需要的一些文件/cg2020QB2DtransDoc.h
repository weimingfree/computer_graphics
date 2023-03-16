
// cg2020QB2DtransDoc.h : Ccg2020QB2DtransDoc 类的接口
//


#pragma once

#define N 64
typedef struct line {
	CPoint p1, p2;
	float transMatrix[3][2];
} Line_t, *Line_p;

typedef struct polygon {
	int    pNumber;
	CPoint points[N];
	float transMatrix[3][2];
} Polygon_t, *Polygon_p;

class Ccg2020QB2DtransDoc : public CDocument
{
protected: // 仅从序列化创建
	Ccg2020QB2DtransDoc();
	DECLARE_DYNCREATE(Ccg2020QB2DtransDoc)

// 特性
public:

	Line_t m_line;
	Polygon_t m_polygon;

	int m_transDir, m_transMode, m_transSelect;

	BOOL m_lineVisible;
	CPoint cp1, cp2;

	int    m_clipPolyNum;
	BOOL m_polygonVisible;
	CPoint clipPolygon[N];

	int m_wndLx, m_wndLy, m_wndRx, m_wndRy;     // Space Window (Lx, Ly)-(Rx,Ry)
// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~Ccg2020QB2DtransDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
