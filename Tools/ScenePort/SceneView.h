#if !defined(AFX_SCENEVIEW_H__B5D5B622_D7B5_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SCENEVIEW_H__B5D5B622_D7B5_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SceneView.h : header file 
//

/////////////////////////////////////////////////////////////////////////////
// CSceneView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.h"

class CSceneView : public CFormView
{
protected:
	CSceneView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSceneView)

// Form Data
public:
	//{{AFX_DATA(CSceneView)
	enum { IDD = IDD_SCENEVIEW };
	float	m_Fps;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSceneView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSceneView)
	afx_msg void OnUpdateScene();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENEVIEW_H__B5D5B622_D7B5_11D2_B451_00C04F6FF8BD__INCLUDED_)
