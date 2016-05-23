#if !defined(AFX_SCENEBITS_H__010A5F22_80B1_11D3_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SCENEBITS_H__010A5F22_80B1_11D3_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SceneBits.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSceneBits form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.h"

class CSceneBits : public CFormView
{
protected:
	CSceneBits();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSceneBits)

// Form Data
public:
	//{{AFX_DATA(CSceneBits)
	enum { IDD = IDD_DIALOG1 };
	BOOL	m_10;
	BOOL	m_1;
	BOOL	m_11;
	BOOL	m_12;
	BOOL	m_13;
	BOOL	m_14;
	BOOL	m_15;
	BOOL	m_16;
	BOOL	m_17;
	BOOL	m_18;
	BOOL	m_19;
	BOOL	m_2;
	BOOL	m_20;
	BOOL	m_21;
	BOOL	m_22;
	BOOL	m_23;
	BOOL	m_24;
	BOOL	m_25;
	BOOL	m_26;
	BOOL	m_27;
	BOOL	m_28;
	BOOL	m_29;
	BOOL	m_3;
	BOOL	m_30;
	BOOL	m_31;
	BOOL	m_32;
	BOOL	m_4;
	BOOL	m_5;
	BOOL	m_6;
	BOOL	m_7;
	BOOL	m_8;
	BOOL	m_9;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneBits)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSceneBits();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSceneBits)
	afx_msg void OnUpdateScene();
	afx_msg void OnClearAll();
	afx_msg void OnSetAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENEBITS_H__010A5F22_80B1_11D3_B451_00C04F6FF8BD__INCLUDED_)
