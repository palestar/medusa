// CreateClass.cpp : implementation file
//

#include "stdafx.h"
#include "CreateClass.h"
#include "DirDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateClass dialog


CCreateClass::CCreateClass(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateClass)
	m_Directory = _T("");
	m_ClassName = _T("");
	m_CPP = _T("");
	m_H = _T("");
	//}}AFX_DATA_INIT
}


void CCreateClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateClass)
	DDX_Text(pDX, IDC_EDIT2, m_Directory);
	DDX_Text(pDX, IDC_EDIT1, m_ClassName);
	DDX_Text(pDX, IDC_CPP, m_CPP);
	DDX_Text(pDX, IDC_H, m_H);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateClass, CDialog)
	//{{AFX_MSG_MAP(CCreateClass)
	ON_EN_CHANGE(IDC_EDIT1, OnUpdateClassName)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowseDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateClass message handlers

void CCreateClass::OnUpdateClassName() 
{
	// get data from dialog
	UpdateData( true );
	m_H = m_Directory + m_ClassName + ".h";
	m_CPP = m_Directory + m_ClassName + ".cpp";
	// put data to dialog
	UpdateData( false );
}

void CCreateClass::OnBrowseDirectory() 
{
	CDirDialog dialog;
	dialog.m_strWindowTitle = "Source Directory";
	dialog.m_strTitle = "Please specify the location of your view class header and source files?";

	if ( dialog.DoBrowse() )
	{
		m_Directory = dialog.m_strPath;
		if ( m_Directory[ m_Directory.GetLength() - 1 ] != '\\')
			m_Directory += "\\";

		// put data to dialog
		UpdateData( false );
		// save setting to the registry
		OnUpdateClassName();
	}
}


BOOL CCreateClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	OnUpdateClassName();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
