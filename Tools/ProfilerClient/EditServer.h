#if !defined(AFX_EDITSERVER_H__C8D71CED_5158_4BDB_9E82_CB4B6A48B21E__INCLUDED_)
#define AFX_EDITSERVER_H__C8D71CED_5158_4BDB_9E82_CB4B6A48B21E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditServer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditServer dialog

class CEditServer : public CDialog
{
// Construction
public:
	CEditServer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditServer)
	enum { IDD = IDD_EDIT_SERVER };
	CString	m_sAddress;
	int		m_nPort;
	BOOL	m_bDisabled;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditServer)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSERVER_H__C8D71CED_5158_4BDB_9E82_CB4B6A48B21E__INCLUDED_)
