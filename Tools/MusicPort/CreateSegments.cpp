// CreateSegments.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
#include "CreateSegments.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateSegments dialog


CCreateSegments::CCreateSegments(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateSegments::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateSegments)
	m_Samples = 0;
	m_UseCue = FALSE;
	m_KeepBranches = FALSE;
	//}}AFX_DATA_INIT
}


void CCreateSegments::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateSegments)
	DDX_Control(pDX, IDC_EDIT1, m_SamplesCtrl);
	DDX_Text(pDX, IDC_EDIT1, m_Samples);
	DDX_Check(pDX, IDC_CHECK1, m_UseCue);
	DDX_Check(pDX, IDC_CHECK2, m_KeepBranches);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateSegments, CDialog)
	//{{AFX_MSG_MAP(CCreateSegments)
	ON_BN_CLICKED(IDC_CHECK1, OnUseCue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateSegments message handlers

BOOL CCreateSegments::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Samples = 44100;
	UpdateData( false );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateSegments::OnOK() 
{
	UpdateData( true );
	CDialog::OnOK();
}

void CCreateSegments::OnUseCue() 
{
	UpdateData( true );
	m_SamplesCtrl.EnableWindow( !m_UseCue );
}
