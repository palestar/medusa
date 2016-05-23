// DocumentOptions.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"

#include "DocumentOptions.h"
#include "DirDialog.h"

#include "System/LocalizedString.h"

#include <io.h>		// _access



/////////////////////////////////////////////////////////////////////////////
// DocumentOptions dialog


DocumentOptions::DocumentOptions(CResourcerDoc *pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(DocumentOptions::IDD, pParent), m_pDoc(pDoc)
{
	//{{AFX_DATA_INIT(DocumentOptions)
	m_SourceDirectory = _T("");
	m_Output = _T("");
	m_MirrorServer = _T("");
	m_Port = 0;
	m_UID = _T("");
	m_PW = _T("");
	m_sLocaleFile = _T("");
	//}}AFX_DATA_INIT
}


void DocumentOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DocumentOptions)
	DDX_Text(pDX, IDC_EDIT1, m_SourceDirectory);
	DDX_Text(pDX, IDC_EDIT3, m_Output);
	DDX_Text(pDX, IDC_EDIT2, m_MirrorServer);
	DDX_Text(pDX, IDC_EDIT4, m_Port);
	DDX_Text(pDX, IDC_EDIT5, m_UID);
	DDX_Text(pDX, IDC_EDIT6, m_PW);
	DDX_Text(pDX, IDC_EDIT7, m_sLocaleFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DocumentOptions, CDialog)
	//{{AFX_MSG_MAP(DocumentOptions)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowseSourceDirectory)
	ON_BN_CLICKED(IDC_BUTTON3, OnBrowseOutput)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseLocaleFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DocumentOptions message handlers

void DocumentOptions::OnBrowseSourceDirectory() 
{
	UpdateData(TRUE);

	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Port Folder");
	dialog.m_strTitle = _T("Port Folder?");

	if ( dialog.DoBrowse() )
	{
		m_SourceDirectory = dialog.m_strPath;
		if ( m_SourceDirectory[ m_SourceDirectory.GetLength() - 1 ] != '\\')
			m_SourceDirectory += "\\";

		UpdateData(FALSE);
	}
}

void DocumentOptions::OnBrowseOutput() 
{
	UpdateData(TRUE);

	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Output Folder");
	dialog.m_strTitle = _T("Output WOB Files?");

	if ( dialog.DoBrowse() )
	{
		m_Output = dialog.m_strPath;
		if ( m_Output[ m_Output.GetLength() - 1 ] != '\\')
			m_Output += '\\';

		UpdateData(FALSE);
	}
}

BOOL DocumentOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_SourceDirectory = m_pDoc->m_RootDirectory;
	m_Output = m_pDoc->m_BrokerFolder;
	m_MirrorServer = m_pDoc->m_MirrorServer;
	m_Port = m_pDoc->m_MirrorServerPort;
	m_UID = m_pDoc->m_UID;
	m_PW = m_pDoc->m_PW;
	m_sLocaleFile = m_pDoc->m_sLocaleFile;

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DocumentOptions::OnOK() 
{
	UpdateData( TRUE );		
	
	if ( m_SourceDirectory.IsEmpty() || _taccess( m_SourceDirectory, 0 ) )
	{
		MessageBox(_T("Source directory is invalid"));
		return;
	}
	if ( m_Output.IsEmpty() || _taccess( m_Output, 0 )  )
	{
		MessageBox(_T("Output directory is invalid"));
		return;
	}

	m_pDoc->m_RootDirectory	= m_SourceDirectory;
	m_pDoc->m_BrokerFolder	= m_Output;
	m_pDoc->m_MirrorServer = m_MirrorServer;
	m_pDoc->m_MirrorServerPort = m_Port;
	m_pDoc->m_UID = m_UID;
	m_pDoc->m_PW = m_PW;
	m_pDoc->m_sLocaleFile = m_sLocaleFile;

	// load in the locale file, do not clear the existing text..
	LocalizedString::locale().load( m_sLocaleFile, false );

	//m_pDoc->SetModifiedFlag();

	CDialog::OnOK();
}

void DocumentOptions::OnBrowseLocaleFile() 
{
	UpdateData(TRUE);

    CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		_T("Locale Files (*.txt)|*.txt||"));

	if ( dialog.DoModal() == IDOK )
	{
		m_sLocaleFile = dialog.GetPathName();
		UpdateData( FALSE );
	}
}
