
// cg2022QBRayTracerDoc.h : Ccg2022QBRayTracerDoc ��Ľӿ�
//


#pragma once


class Ccg2022QBRayTracerDoc : public CDocument
{
protected: // �������л�����
	Ccg2022QBRayTracerDoc();
	DECLARE_DYNCREATE(Ccg2022QBRayTracerDoc)

// ����
public:
	float m_runTime;

	int m_rayTraceMethod;
// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// ʵ��
public:
	virtual ~Ccg2022QBRayTracerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// ����Ϊ����������������������ݵ� Helper ����
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
