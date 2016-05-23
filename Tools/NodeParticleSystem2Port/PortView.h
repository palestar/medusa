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

class CPortView : public CFormView
{
protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_NODEVIEW };
	int		m_Template;
	int		m_ParticleCount;
	float	m_MinLife;
	float	m_MaxLife;
	float	m_MinOriginX;
	float	m_MinOriginY;
	float	m_MinOriginZ;
	float	m_MaxOriginX;
	float	m_MaxOriginY;
	float	m_MaxOriginZ;
	float	m_MinVelocityX;
	float	m_MinVelocityY;
	float	m_MinVelocityZ;
	float	m_MaxVelocityX;
	float	m_MaxVelocityY;
	float	m_MaxVelocityZ;
	float	m_MinAccelerationX;
	float	m_MinAccelerationY;
	float	m_MinAccelerationZ;
	float	m_MaxAccelerationX;
	float	m_MaxAccelerationY;
	float	m_MaxAccelerationZ;
	float	m_Life;
	float	m_Visible;
	float	m_VisibleV;
	float	m_VisibleA;
	float	m_Scale;
	float	m_ScaleV;
	float	m_ScaleA;
	float	m_Alpha;
	float	m_AlphaV;
	float	m_AlphaA;
	CString	m_MaterialPort;
	float	m_ScaleVar;
	int		m_OriginType;
	BOOL	m_ReverseTime;
	DWORD	m_Seed;
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
	afx_msg void OnApplyTemplate();
	afx_msg void OnOpenMaterial();
	afx_msg void OnBrowseMaterial();
	afx_msg void OnUpdateNode();
	afx_msg void OnGenerate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
