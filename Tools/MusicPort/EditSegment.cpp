// EditSegment.cpp : implementation file
//

#include "stdafx.h"
#include "EditSegment.h"

/////////////////////////////////////////////////////////////////////////////
// CEditSegment dialog


CEditSegment::CEditSegment(CWnd* pParent /*=NULL*/)
	: CDialog(CEditSegment::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditSegment)
	m_Begin = 0;
	m_End = 0;
	//}}AFX_DATA_INIT
}


void CEditSegment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditSegment)
	DDX_Text(pDX, IDC_EDIT1, m_Begin);
	DDX_Text(pDX, IDC_EDIT2, m_End);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditSegment, CDialog)
	//{{AFX_MSG_MAP(CEditSegment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditSegment message handlers

BOOL CEditSegment::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditSegment::OnOK() 
{
	UpdateData( true );

	if ( m_Begin > m_End )
		m_End = m_Begin;

	CDialog::OnOK();
}

//----------------------------------------------------------------------------
// EOF

