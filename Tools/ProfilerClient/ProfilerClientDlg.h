// ProfilerClientDlg.h : header file
//

#if !defined(AFX_PROFILERCLIENTDLG_H__850A03D8_2CD8_486F_838F_2CCD1227ADB7__INCLUDED_)
#define AFX_PROFILERCLIENTDLG_H__850A03D8_2CD8_486F_838F_2CCD1227ADB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network/ProfilerClient.h"



/////////////////////////////////////////////////////////////////////////////
// CProfilerClientDlg dialog

class CProfilerClientDlg : public CDialog
{
// Construction
public:
	CProfilerClientDlg( CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CProfilerClientDlg)
	enum { IDD = IDD_PROFILERCLIENT_DIALOG };
	CTreeCtrl	m_cProfileTree;
	CString	m_sStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfilerClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProfilerClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSettings();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Data
	CImageList				m_ImageList;

public:
	struct Server
	{
		bool				bDisabled;
		CharString			sAddress;
		int					nPort;
		ProfilerClient *	pClient;
		qword				nPeakCPU;		// peak CPU usage

		Server & operator=( const Server & copy )
		{
			bDisabled = copy.bDisabled;
			sAddress = copy.sAddress;
			nPort = copy.nPort;
			pClient = copy.pClient;
			nPeakCPU = copy.nPeakCPU;
			return *this;
		}
	};
	struct Alarm
	{
		CharString			sNameFilter;
		int					nPercent;
		bool				bAudibleAlarm;

		Alarm & operator=( const Alarm & copy )
		{
			sNameFilter = copy.sNameFilter;
			nPercent = copy.nPercent;
			bAudibleAlarm = copy.bAudibleAlarm;
			return *this;
		}
	};

	Array< Server >		m_Servers;
	Array< Alarm >		m_Alarms;
	CharString			m_sLogFile;
	bool				m_bAlarm;

	void				loadSettings();
	void				saveSettings();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROFILERCLIENTDLG_H__850A03D8_2CD8_486F_838F_2CCD1227ADB7__INCLUDED_)
