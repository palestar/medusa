#if !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.h"
#include "ColorBtn.h"

class CPortView : public CFormView
{
	friend class CColorBtn;

	CColorBtn		m_TextColor;

protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_NODEVIEW };
	CListCtrl	m_ResourceList;
	CString	m_Class;
	CString	m_Name;
	BOOL	m_Visible;
	float	m_Alpha;
	int		m_Left;
	int		m_Top;
	int		m_Right;
	int		m_Bottom;
	BOOL	m_Docked;
	BOOL	m_CanMove;
	BOOL	m_CanSize;
	BOOL	m_CanScroll;
	BOOL	m_EffectFadeIn;
	BOOL	m_EffectHighlight;
	BOOL	m_EffectRollUV;
	BOOL	m_HasBack;
	BOOL	m_CanDock;
	BOOL	m_DockX;
	BOOL	m_DockY;
	CString	m_WindowStyle;
	BOOL	m_Document;
	BOOL	m_NoClip;
	BOOL	m_HCenter;
	BOOL	m_VCenter;
	BOOL	m_DockZ;
	BOOL	m_DockOutside;
	int		m_Height;
	int		m_Width;
	BOOL	m_ParentWidth;
	BOOL	m_ParentHeight;
	BOOL	m_CanFocus;
	BOOL	m_User1;
	BOOL	m_User2;
	BOOL	m_User3;
	BOOL	m_User4;
	BOOL	m_User5;
	BOOL	m_User6;
	BOOL	m_User7;
	BOOL	m_User8;
	BOOL	m_SmoothUL;
	BOOL	m_SmoothUR;
	BOOL	m_SmoothLR;
	BOOL	m_SmoothLL;
	BOOL	m_AnimBeam;
	BOOL	m_AnimBack;
	BOOL	m_AnimFlare;
	CString	m_Help;
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
	afx_msg void OnUpdateNode();
	afx_msg void OnBrowseClass();
	afx_msg void OnAddResource();
	afx_msg void OnRemoveResource();
	afx_msg void OnEditResource(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRenameResource(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenWindowStyle();
	afx_msg void OnBrowseWindowStyle();
	afx_msg void OnUpdateWidthHeight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
