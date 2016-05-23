#if !defined(AFX_SELECTIMPORTPORT_H__DF72BEE1_7640_11D3_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SELECTIMPORTPORT_H__DF72BEE1_7640_11D3_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectImportPort.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectImportPort dialog

class CSelectImportPort : public CDialog
{
// Construction
public:
	CSelectImportPort( const char * ext, CWnd* pParent = NULL);   // standard constructor

	CString		m_ImportExt;
	int			m_SelectedPort;

// Dialog Data
	//{{AFX_DATA(CSelectImportPort)
	enum { IDD = IDD_SELECT_IMPORT_PORT };
	CListCtrl	m_PortList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectImportPort)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectImportPort)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTIMPORTPORT_H__DF72BEE1_7640_11D3_B451_00C04F6FF8BD__INCLUDED_)
