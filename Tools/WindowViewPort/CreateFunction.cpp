// CreateFunction.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
#include "CreateFunction.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateFunction dialog


CCreateFunction::CCreateFunction(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateFunction::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateFunction)
	m_FunctionName = _T("");
	m_Prototype = _T("");
	//}}AFX_DATA_INIT
}


void CCreateFunction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateFunction)
	DDX_Text(pDX, IDC_EDIT1, m_FunctionName);
	DDX_Text(pDX, IDC_PROTOTYPE, m_Prototype);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateFunction, CDialog)
	//{{AFX_MSG_MAP(CCreateFunction)
	ON_EN_CHANGE(IDC_EDIT1, OnUpdatePrototype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateFunction message handlers

BOOL CCreateFunction::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData( false );

	OnUpdatePrototype();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateFunction::OnUpdatePrototype() 
{
	// get data from dialog
	UpdateData( true );
	m_Prototype = m_Template;
	m_Prototype.Replace( _T("$$function$$"), m_FunctionName );
	// put data to dialog
	UpdateData( false );
}

void CCreateFunction::OnOK() 
{
	CDialog::OnOK();
}


