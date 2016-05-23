#if !defined(AFX_RESOURCERTREE_H__1D0232A1_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_RESOURCERTREE_H__1D0232A1_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcerTree.h : header file
//

#include "afxcview.h"
#include "DirectoryWatch.h"

/////////////////////////////////////////////////////////////////////////////
// ResourcerTree view

class ResourcerTree : public CTreeView
{
private:
	// Mutators
	void			populateTree( const CString &path, HTREEITEM parent );
	// Helpers
	bool			removeDirectory( const char * pPath );
	CString			path( HTREEITEM hTop );

	// Data
	CImageList		m_ImageList;
	HTREEITEM		m_Root;

protected:
	ResourcerTree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(ResourcerTree)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ResourcerTree)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ResourcerTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(ResourcerTree)
	afx_msg void OnTreeCreate();
	afx_msg void OnTreeDelete();
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditRename();
	afx_msg void OnViewRefresh();
	afx_msg void OnDocumentBuildselected();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPortsTouch();
	afx_msg void OnPortsUpgrade();
	afx_msg void OnUpdatePortsUpgrade(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePortsTouch(CCmdUI *pCmdUI);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCERTREE_H__1D0232A1_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
