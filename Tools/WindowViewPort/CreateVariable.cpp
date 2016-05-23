// CreateVariable.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
#include "CreateVariable.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateVariable dialog


CCreateVariable::CCreateVariable(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateVariable::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateVariable)
	m_Prototype = _T("");
	m_VariableName = _T("");
	//}}AFX_DATA_INIT
}


void CCreateVariable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateVariable)
	DDX_Text(pDX, IDC_PROTOTYPE, m_Prototype);
	DDX_Text(pDX, IDC_EDIT1, m_VariableName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateVariable, CDialog)
	//{{AFX_MSG_MAP(CCreateVariable)
	ON_EN_CHANGE(IDC_EDIT1, OnUpdatePrototype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateVariable message handlers

BOOL CCreateVariable::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateData( false );
	OnUpdatePrototype();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateVariable::OnUpdatePrototype() 
{
	// get data from dialog
	UpdateData( true );
	m_Prototype = m_PrototypeTemplate;
	m_Prototype.Replace( _T("$$variable$$"), m_VariableName );
	// put data to dialog
	UpdateData( false );
}
