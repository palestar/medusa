#pragma once


// CPropertyView view
#include "Tools/ResourcerDoc/PropertyList.h"


class CPropertyView : public CView
{
	DECLARE_DYNCREATE(CPropertyView)

protected:
	CPropertyView();           // protected constructor used by dynamic creation
	virtual ~CPropertyView();

	CPropertyList		 m_cList;

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
   afx_msg void OnEditAdd();
   afx_msg void OnUpdateEditAdd(CCmdUI *pCmdUI);
   afx_msg void OnEditClear();
   afx_msg void OnUpdateEditClear(CCmdUI *pCmdUI);
   afx_msg void OnEditDelete();
   afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
protected:
   virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};


