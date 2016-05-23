#if !defined(AFX_CREATEDIRECTORY_H__1D0232B5_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_CREATEDIRECTORY_H__1D0232B5_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateDirectory.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CreateDirectory dialog

#undef CreateDirectory
class CreateDirectory : public CDialog
{
// Construction
public:
	CreateDirectory(const CString &root,CWnd* pParent = NULL);   // standard constructor

private:
	// Data
	CString			m_Root;

// Dialog Data
	//{{AFX_DATA(CreateDirectory)
	enum { IDD = IDD_CREATE_DIRECTORY };
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CreateDirectory)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CreateDirectory)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEDIRECTORY_H__1D0232B5_7498_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
