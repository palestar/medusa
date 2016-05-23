#if !defined(AFX_PORTVIEW_H__E6231FD8_7E0A_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__E6231FD8_7E0A_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

#include "resource.h"
#include "ColorBtn.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CPortView : public CFormView
{
private:
	CColorBtn				m_ColorKey;

protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_FORMVIEW };
	CSpinButtonCtrl	m_SpinHeight;
	CSpinButtonCtrl	m_SpinWidth;
	CSpinButtonCtrl	m_SpinEnd;
	CSpinButtonCtrl	m_SpinStart;
	CSpinButtonCtrl	m_SpinBottom;
	CSpinButtonCtrl	m_SpinRight;
	CSpinButtonCtrl	m_SpinTop;
	CSpinButtonCtrl	m_SpinLeft;
	CSpinButtonCtrl	m_SpinHY;
	CSpinButtonCtrl	m_SpinHX;
	CStatic	m_Thumb;
	int		m_HotSpotX;
	int		m_HotSpotY;
	BOOL	m_DoCrop;
	int		m_Left;
	int		m_Top;
	int		m_Right;
	int		m_Bottom;
	BOOL	m_DoResize;
	BOOL	m_DoResample;
	BOOL	m_KeepFrames;
	int		m_Start;
	int		m_End;
	int		m_ResizeWidth;
	int		m_ResizeHeight;
	CString	m_Width;
	CString	m_Height;
	CString	m_Frames;
	BOOL	m_ColorKeyEnable;
	CComboBox m_PixelFormat;
	BOOL	m_bCreateMipMaps;
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
	afx_msg void OnView();
	afx_msg void OnUpdateView(CCmdUI* pCmdUI);
	afx_msg void OnImport();
	afx_msg void OnEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnImportAlpha();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__E6231FD8_7E0A_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
