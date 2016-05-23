#if !defined(AFX_SCENETREE_H__3D222E81_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SCENETREE_H__3D222E81_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SceneTree.h : header file 
//

#include "afxcview.h"
#include "Tools/ScenePort/BaseNodePort.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneTree view

class CSceneTree : public CTreeView
{
private:
	// Mutators
	void			populateTree( HTREEITEM parent, BaseNodePort::Ref node );

	// Data
	CImageList		m_ImageList;
	HTREEITEM		m_Root;

	BaseNodePort *	m_pSelectedNode;

protected:
	CSceneTree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSceneTree)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneTree)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSceneTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSceneTree)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeDelete();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCut();
	afx_msg void OnViewRefresh();
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeOpen();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnPortCreate( UINT nID );
	afx_msg void OnUpdatePortCreate(CCmdUI *pCmdUI, UINT nID);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENETREE_H__3D222E81_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_)
