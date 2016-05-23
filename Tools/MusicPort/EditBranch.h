#if !defined(AFX_EDITBRANCH_H__5846D9A4_EC76_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_EDITBRANCH_H__5846D9A4_EC76_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBranch.h : header file
//

#include "Port.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CEditBranch dialog

class CEditBranch : public CDialog
{
// Construction
public:
	CEditBranch( MusicPort * pDoc, CWnd* pParent = NULL);   // standard constructor

	MusicPort *		m_pDoc;

	int				m_Trigger;		// trigger index
	bool			m_Condition;	// condition
	int				m_Segment;		// segment index

// Dialog Data
	//{{AFX_DATA(CEditBranch)
	enum { IDD = IDD_EDITBRANCH };
	CComboBox	m_SegmentSelect;
	CComboBox	m_TriggerSelect;
	int		m_ConditionSelect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBranch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditBranch)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBRANCH_H__5846D9A4_EC76_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
