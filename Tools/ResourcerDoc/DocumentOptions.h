#if !defined(AFX_DOCUMENTOPTIONS_H__1D0232A6_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_DOCUMENTOPTIONS_H__1D0232A6_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DocumentOptions.h : header file
//

#include "ResourcerDoc.h"

/////////////////////////////////////////////////////////////////////////////
// DocumentOptions dialog

class DocumentOptions : public CDialog
{
// Construction
public:
	DocumentOptions(CResourcerDoc *pDoc, CWnd* pParent = NULL);   // standard constructor

private:
	CResourcerDoc *		m_pDoc;

// Dialog Data
	//{{AFX_DATA(DocumentOptions)
	enum { IDD = IDD_DOC_OPTIONS };
	CString	m_SourceDirectory;
	CString	m_Output;
	CString	m_MirrorServer;
	int		m_Port;
	CString	m_UID;
	CString	m_PW;
	CString	m_sLocaleFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DocumentOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DocumentOptions)
	afx_msg void OnBrowseSourceDirectory();
	afx_msg void OnBrowseOutput();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowseLocaleFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCUMENTOPTIONS_H__1D0232A6_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
