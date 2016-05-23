#if !defined(AFX_SETTINGS_H__B9438F39_AFBE_4442_8E64_F9AB38FA889E__INCLUDED_)
#define AFX_SETTINGS_H__B9438F39_AFBE_4442_8E64_F9AB38FA889E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Settings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog

class CSettings : public CDialog
{
// Construction
public:
	CSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettings)
	enum { IDD = IDD_SETTINGS };
	CListCtrl	m_cAlarms;
	CListCtrl	m_cServers;
	CString	m_sLogFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettings)
	virtual BOOL OnInitDialog();
	afx_msg void OnLog();
	afx_msg void OnAddServer();
	afx_msg void OnEditServer();
	afx_msg void OnDeleteServer();
	afx_msg void OnAddAlarm();
	afx_msg void OnEditAlarm();
	afx_msg void OnDeleteAlarm();
	afx_msg void OnToggleServers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void		updateServerList();
	void		updateAlarmList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGS_H__B9438F39_AFBE_4442_8E64_F9AB38FA889E__INCLUDED_)
