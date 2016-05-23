#if !defined(AFX_CLASSHEIRARCHY_H__B1D8E0C2_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_CLASSHEIRARCHY_H__B1D8E0C2_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClassHeirarchy.h : header file
//

#include "Tools/Resourcer/Resource.h"
#include "Factory/ClassKey.h"

////////////////////////////////////////////////////////////////////////////
// CClassHeirarchy dialog

class CClassHeirarchy : public CDialog
{
// Construction
public:
	CClassHeirarchy(CWnd* pParent = NULL);   // standard constructor

	void			buildTree( HTREEITEM parent, const ClassKey &parentKey );

// Dialog Data
	//{{AFX_DATA(CClassHeirarchy)
	enum { IDD = IDD_CLASS_HEIRARCHY };
	CTreeCtrl	m_Tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClassHeirarchy)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CClassHeirarchy)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASSHEIRARCHY_H__B1D8E0C2_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
