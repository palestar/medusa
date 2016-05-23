#if !defined(AFX_PORTVIEW_H__952196F7_8DB1_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__952196F7_8DB1_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

#include "Display/DisplayDevice.h"
#include "Draw/Font.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CPortView : public CFormView
{
	// Data
	LOGFONT						m_LogicalFont;
	DisplayDevice::Ref			m_Device;
	Font::Link					m_Font;
	int							m_nOffset, m_nOffsetSize;

	// Mutators
	void				updateFontInformation();

protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_FORMVIEW };
	CStatic	m_Preview;
	CString	m_Characters;
	CString	m_Height;
	CString	m_Width;
	CString	m_Offset;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPortView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPortView)
	afx_msg void OnPickFont();
	afx_msg void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnPrevious();
	afx_msg void OnNext();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__952196F7_8DB1_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
