#if !defined(AFX_PORTDEPEND_H__063A3BC7_611A_11D3_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_PORTDEPEND_H__063A3BC7_611A_11D3_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortDepend.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortDepend dialog

class CPortDepend : public CDialog
{
// Construction
public:
	CPortDepend( Array< CharString > & dep, CWnd* pParent = NULL);   // standard constructor

	// Mutators
	void	buildTree( HTREEITEM parent );
	// Data
	Array< CharString >	m_Depend;

// Dialog Data
	//{{AFX_DATA(CPortDepend)
	enum { IDD = IDD_PORT_DEPEND };
	CTreeCtrl	m_DependTree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortDepend)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPortDepend)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenDepend(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTDEPEND_H__063A3BC7_611A_11D3_B451_00C04F6FF8BD__INCLUDED_)
