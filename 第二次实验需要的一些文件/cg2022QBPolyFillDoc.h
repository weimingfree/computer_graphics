
// cg2022QBPolyFillDoc.h : Ccg2022QBPolyFillDoc ��Ľӿ�
//


#pragma once


class Ccg2022QBPolyFillDoc : public CDocument
{
protected: // �������л�����
	Ccg2022QBPolyFillDoc();
	DECLARE_DYNCREATE(Ccg2022QBPolyFillDoc)

// ����
public:
	int m_opSelect;
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
	virtual ~Ccg2022QBPolyFillDoc();
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
