// PropertyEditorView.h : interface of the CPropertyEditorView class
//

#pragma once
#include "PropertyList.h"
#include "ResourcerDoc.h"
#include "Port.h"

class CPropertyEditorView : public CView
{
protected: // create from serialization only
	CPropertyEditorView();
	DECLARE_DYNCREATE(CPropertyEditorView)

// Attributes
public:
	Port *			     GetDocument() const;
	CPropertyList		 m_cList;

// Operations
public:

// Overrides
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CPropertyEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
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

#ifndef _DEBUG  // debug version in PropertyEditorView.cpp
inline Port* CPropertyEditorView::GetDocument() const
   { return reinterpret_cast<Port*>(m_pDocument); }
#endif

