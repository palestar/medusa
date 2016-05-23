#if !defined(AFX_EXPORTDIALOG_H__177761E2_A3DF_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_EXPORTDIALOG_H__177761E2_A3DF_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDialog.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CExportDialog dialog

class CExportDialog : public CDialog
{
// Construction
public:
	CExportDialog( CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportDialog)
	enum { IDD = IDD_EXPORTDIALOG };
	BOOL	m_ExportAnimation;
	float	m_Scale;
	CString	m_Images;
	BOOL	m_ExportVertexAnimation;
	CString	m_Materials;
	BOOL	m_OverwriteTextures;
	BOOL	m_bPrefixNames;
	CString	m_sPortsDirectory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowseImages();
	afx_msg void OnBrowseMaterials();
	afx_msg void OnBrowsePorts();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDIALOG_H__177761E2_A3DF_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
