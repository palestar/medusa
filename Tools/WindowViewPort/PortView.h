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
#include "Tools/ScenePort/BaseNodePort.h"

class CPortView : public CFormView
{
protected:
	CPortView();           // protected constructor used by dynamic creationw
	DECLARE_DYNCREATE(CPortView)

	void			createViewClass();
	void			updateNodeTree();
	void			updateNodeTree( HTREEITEM parent, BaseNodePort * pNode );
	void			updateMessageList();
	void			updateFunctionList();
	void			updateVariableList();

	CString			loadH();
	bool			saveH( const char * headerText );
	CString			loadCPP();
	bool			saveCPP( const char * sourceText );

	bool			insertLine( CString & text, 
						const TCHAR * sectionBegin, 
						const TCHAR * sectionEnd, 
						const TCHAR * line );

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_NODEVIEW };
	CListCtrl	m_Messages;
	CButton	m_AddVariableCtrl;
	CButton	m_AddFunctionCtrl;
	CListCtrl	m_Variables;
	CListCtrl	m_Functions;
	CTreeCtrl	m_Nodes;
	CString	m_H;
	CString	m_CPP;
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
	afx_msg void OnAddFunction();
	afx_msg void OnAddVariable();
	afx_msg void OnSelectedNode(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectMessage(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenNode(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddFunctionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewClass();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
