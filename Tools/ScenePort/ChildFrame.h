#if !defined(AFX_CHILDFRAME_H__BFA4A018_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_CHILDFRAME_H__BFA4A018_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChildFrame.h : header file 
//

#include "Tools/ScenePort/ScenePortDll.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame frame

class DLL CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
protected:
	CChildFrame();           // protected constructor used by dynamic creation

public:
	CToolBar		m_ToolBar;
	CToolBar		m_NodeBar;
	CStatusBar		m_StatusBar;	

	CMenu			m_Menu;

	CSplitterWnd	m_Splitter;
	CSplitterWnd	m_Splitter2;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CMDIFrameWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChildFrame();

	// Generated message map functions
	//{{AFX_MSG(CChildFrame)
	afx_msg void OnSceneEditsegments();
	afx_msg void OnSceneProperties();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnSceneBits();
	afx_msg void OnRenderSdetaillevellow();
	afx_msg void OnUpdateRenderSdetaillevellow(CCmdUI* pCmdUI);
	afx_msg void OnRenderSdetailevelmedium();
	afx_msg void OnUpdateRenderSdetailevelmedium(CCmdUI* pCmdUI);
	afx_msg void OnRenderSdetaillevelhigh();
	afx_msg void OnUpdateRenderSdetaillevelhigh(CCmdUI* pCmdUI);
	afx_msg void OnRenderSshowalignedhull();
	afx_msg void OnUpdateRenderSshowalignedhull(CCmdUI* pCmdUI);
	afx_msg void OnRenderSshowhull();
	afx_msg void OnUpdateRenderSshowhull(CCmdUI* pCmdUI);
	afx_msg void OnRenderSshownodeaxis();
	afx_msg void OnUpdateRenderSshownodeaxis(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRAME_H__BFA4A018_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_)
