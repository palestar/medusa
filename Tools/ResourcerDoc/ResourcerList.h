#if !defined(AFX_RESOURCERLIST_H__1D0232A2_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_RESOURCERLIST_H__1D0232A2_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcerList.h : header file
//

#include "afxcview.h"

#include "DirectoryWatch.h"
#include "Standard/Array.h"

/////////////////////////////////////////////////////////////////////////////
// ResourcerList view

class ResourcerList : public CListView
{
	// Friends
	friend class ResourcePort;

private:
	// Data
	CImageList				m_ListIcons;

protected:
	ResourcerList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(ResourcerList)

// Attributes

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ResourcerList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ResourcerList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	friend class ResourcerTree;

	// Generated message map functions
protected:
	//{{AFX_MSG(ResourcerList)
	afx_msg void OnPortsDelete();
	afx_msg void OnEditRename();
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnViewRefresh();
	afx_msg void OnPortsProperties();
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnPortsShellopen();
	afx_msg void OnUpdatePortsDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePortsProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePortsShellopen(CCmdUI* pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDocumentBuildselected();
	afx_msg void OnUpdateDocumentBuildselected(CCmdUI* pCmdUI);
	afx_msg void OnPortsDependencies();
	afx_msg void OnUpdatePortsDependencies(CCmdUI* pCmdUI);
	afx_msg void OnPortsTouch();
	afx_msg void OnPortsUpgrade();
	afx_msg void OnUpdatePortsUpgrade(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnPortCreate( UINT nID );
	afx_msg void OnFileImport();

	bool ImportFile( const char * a_pFile );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCERLIST_H__1D0232A2_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
