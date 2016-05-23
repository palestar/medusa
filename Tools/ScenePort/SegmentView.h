#if !defined(AFX_SEGMENTVIEW_H__496406E4_D5FD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_SEGMENTVIEW_H__496406E4_D5FD_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SegmentView.h : header file 
//

/////////////////////////////////////////////////////////////////////////////
// CSegmentView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.h"

class CSegmentView : public CFormView
{
protected:
	CSegmentView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSegmentView)

// Form Data
public:
	//{{AFX_DATA(CSegmentView)
	enum { IDD = IDD_SEGMENTVIEW };
	CComboBox	m_Segments;
	int		m_CurrentSegment;
	CString	m_SegmentName;
	float	m_BeginTime;
	float	m_EndTime;
	BOOL	m_InFrames;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSegmentView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSegmentView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSegmentView)
	afx_msg void OnAddSegment();
	afx_msg void OnRemoveSegment();
	afx_msg void OnChangedSegment();
	afx_msg void OnUpdateNode();
	afx_msg void OnInFrames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEGMENTVIEW_H__496406E4_D5FD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
