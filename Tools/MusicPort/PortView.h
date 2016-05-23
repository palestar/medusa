#if !defined(AFX_PORTVIEW_H__9CE1E827_E7A0_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_PORTVIEW_H__9CE1E827_E7A0_11D2_B451_00C04F6FF8BD__INCLUDED_

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

#include "Audio/AudioDevice.h"
#include "Audio/Music.h"
#include "Resource.h"

class CPortView : public CFormView
{
	// Data
	bool			m_Initialized;

	CToolBar		m_ToolBar;
	CImageList		m_SegmentImage;
	CImageList		m_BranchImage;
	CImageList		m_TriggerImage;

	AudioDevice::Ref
					m_Device;
	Music::Link		m_Music;
	int				m_ActiveTriggerIndex;

	// Mutators
	void			updateSegmentList();
	void			updateBranchList();
	void			updateTriggerList();
	void			updateDialog();

protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_FORMVIEW };
	CListCtrl	m_Triggers;
	CListCtrl	m_Branches;
	CListCtrl	m_Segments;
	CString	m_Wave;
	CString	m_ActiveSegment;
	CString	m_ActiveTrigger;
	CString	m_Rate;
	CString	m_Channels;
	CString	m_BranchCount;
	CString	m_Bits;
	CString	m_Samples;
	CString	m_SegmentCount;
	CString	m_TriggerCount;
	int		m_BufferSize;
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
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
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
	afx_msg void OnOpenWave();
	afx_msg void OnBrowseWave();
	afx_msg void OnMusicPlay();
	afx_msg void OnUpdateMusicPlay(CCmdUI* pCmdUI);
	afx_msg void OnMusicStop();
	afx_msg void OnUpdateMusicStop(CCmdUI* pCmdUI);
	afx_msg void OnSegmentsAutocreate();
	afx_msg void OnSegmentsCreate();
	afx_msg void OnSegmentsDelete();
	afx_msg void OnUpdateSegmentsDelete(CCmdUI* pCmdUI);
	afx_msg void OnSegmentsOpen();
	afx_msg void OnSegmentsOpenList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateSegmentsOpen(CCmdUI* pCmdUI);
	afx_msg void OnBranchesCreate();
	afx_msg void OnUpdateBranchesCreate(CCmdUI* pCmdUI);
	afx_msg void OnBranchesDelete();
	afx_msg void OnUpdateBranchesDelete(CCmdUI* pCmdUI);
	afx_msg void OnBranchesOpen();
	afx_msg void OnBranchesOpenList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateBranchesOpen(CCmdUI* pCmdUI);
	afx_msg void OnTriggersCreate();
	afx_msg void OnTriggersDelete();
	afx_msg void OnUpdateTriggersDelete(CCmdUI* pCmdUI);
	afx_msg void OnTriggersOpen();
	afx_msg void OnTriggersOpenList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateTriggersOpen(CCmdUI* pCmdUI);
	afx_msg void OnTriggersActivate();
	afx_msg void OnUpdateTriggersActivate(CCmdUI* pCmdUI);
	afx_msg void OnRenameSegment(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnChangeSegment(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRenameTrigger(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateBufferSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__9CE1E827_E7A0_11D2_B451_00C04F6FF8BD__INCLUDED_)
