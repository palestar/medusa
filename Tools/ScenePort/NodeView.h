#if !defined(AFX_NODEVIEW_H__841ED1A5_9F09_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_NODEVIEW_H__841ED1A5_9F09_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NodeView.h : header file
// 

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CNodeView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CNodeView : public CFormView
{
private:
	// Data
	bool			m_ControlUpdate;

protected:
	CNodeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CNodeView)

// Form Data
public:
	//{{AFX_DATA(CNodeView)
	enum { IDD = IDD_NODEVIEW };
	float	m_X;
	float	m_Y;
	float	m_Z;
	float	m_IX;
	float	m_IY;
	float	m_IZ;
	float	m_JX;
	float	m_JY;
	float	m_JZ;
	float	m_KX;
	float	m_KY;
	float	m_KZ;
	float	m_Scale;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CNodeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CNodeView)
	afx_msg void OnIdentity();
	afx_msg void OnUpdateNode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__841ED1A5_9F09_11D2_B451_00C04F6FF8BD__INCLUDED_)
