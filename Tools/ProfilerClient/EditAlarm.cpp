// EditAlarm.cpp : implementation file
//

#include "stdafx.h"
#include "ProfilerClient.h"
#include "EditAlarm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditAlarm dialog


CEditAlarm::CEditAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CEditAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditAlarm)
	m_bAudibleAlaram = FALSE;
	m_sNameFilter = _T("");
	m_nPercent = 0;
	//}}AFX_DATA_INIT
}


void CEditAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditAlarm)
	DDX_Check(pDX, IDC_CHECK1, m_bAudibleAlaram);
	DDX_Text(pDX, IDC_EDIT1, m_sNameFilter);
	DDX_Text(pDX, IDC_EDIT2, m_nPercent);
	DDV_MinMaxInt(pDX, m_nPercent, 0, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditAlarm, CDialog)
	//{{AFX_MSG_MAP(CEditAlarm)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditAlarm message handlers
