// CreateDirectory.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"

#include "CreateDirectory.h"
#include "File/FileDisk.h"



/////////////////////////////////////////////////////////////////////////////
// CreateDirectory dialog


CreateDirectory::CreateDirectory(const CString &root,CWnd* pParent /*=NULL*/)
	: CDialog(CreateDirectory::IDD, pParent), m_Root(root)
{
	//{{AFX_DATA_INIT(CreateDirectory)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}


void CreateDirectory::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CreateDirectory)
	DDX_Text(pDX, IDC_DIR_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CreateDirectory, CDialog)
	//{{AFX_MSG_MAP(CreateDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CreateDirectory message handlers

void CreateDirectory::OnOK() 
{
	UpdateData( TRUE );

	if ( !FileDisk::createDirectory( m_Root + m_Name )  )
	{
		MessageBox( m_Root + m_Name, _T("Failed to create directory") );
		CDialog::OnCancel();
	}

	CDialog::OnOK();
}
