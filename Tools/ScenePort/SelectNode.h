#if !defined(AFX_SELECTNODE_H__EE888921_E92A_11D3_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_SELECTNODE_H__EE888921_E92A_11D3_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectNode.h : header file
//

#include "Port.h"
#include "ScenePortDll.h"

#define IDD_SELECTNODE                  10014

/////////////////////////////////////////////////////////////////////////////
// CSelectNode dialog

class DLL CSelectNode : public CDialog
{
// Construction
public:
	CSelectNode(ScenePort * pDoc, CWnd* pParent = NULL);   // standard constructor

	// Mutators
	void				buildTree( HTREEITEM hParent, BaseNodePort * pNode );

	// Data
	ScenePort *			m_pDoc;
	BaseNodePort *		m_pSelected;

// Dialog Data
	//{{AFX_DATA(CSelectNode)
	enum { IDD = IDD_SELECTNODE };
	CTreeCtrl	m_Nodes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectNode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectNode)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnNULL();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTNODE_H__EE888921_E92A_11D3_BA92_00C0DF22DE85__INCLUDED_)
