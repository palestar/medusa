// ExportDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ExportDialog.h"
#include "DirDialog.h"

#include "Standard/Registry.h"



/////////////////////////////////////////////////////////////////////////////
// CExportDialog dialog


CExportDialog::CExportDialog( CWnd* pParent /*=NULL*/)
	: CDialog(CExportDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDialog)
	m_ExportAnimation = FALSE;
	m_Scale = 0.0f;
	m_Images = _T("");
	m_ExportVertexAnimation = FALSE;
	m_Materials = _T("");
	m_OverwriteTextures = FALSE;
	m_bPrefixNames = FALSE;
	m_sPortsDirectory = _T("");
	//}}AFX_DATA_INIT
}


void CExportDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDialog)
	DDX_Check(pDX, IDC_CHECK1, m_ExportAnimation);
	DDX_Text(pDX, IDC_EDIT1, m_Scale);
	DDX_Text(pDX, IDC_EDIT5, m_Images);
	DDX_Check(pDX, IDC_CHECK7, m_ExportVertexAnimation);
	DDX_Text(pDX, IDC_EDIT6, m_Materials);
	DDX_Check(pDX, IDC_CHECK8, m_OverwriteTextures);
	DDX_Check(pDX, IDC_CHECK2, m_bPrefixNames);
	DDX_Text(pDX, IDC_EDIT7, m_sPortsDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportDialog, CDialog)
	//{{AFX_MSG_MAP(CExportDialog)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseImages)
	ON_BN_CLICKED(IDC_BUTTON3, OnBrowseMaterials)
	ON_BN_CLICKED(IDC_BUTTON4, OnBrowsePorts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDialog message handlers

BOOL CExportDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Registry settings( Registry::LOCAL, "MedusaExport" );

	m_Scale = 2.0f;
	m_sPortsDirectory = settings.get("m_sPortsDirectory", "" );
	m_Images = CString( settings.get( "m_Images", "" ) );
	m_Materials = CString( settings.get( "m_Materials", "" ) );
	m_bPrefixNames = settings.get( "m_bPrefixNames", (dword)0 );

	UpdateData( false );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDialog::OnOK() 
{
	UpdateData();

	if (! CharString( m_Images ).beginsWith( CharString(m_sPortsDirectory) ) )
	{
		MessageBox( _T("The textures directory must be a sub-directory of the ports directory!") );
		return;
	}
	if (! CharString( m_Materials ).beginsWith( CharString(m_sPortsDirectory) ) )
	{
		MessageBox( _T("The materials directory must be a sub-directory of the ports directory!") );
		return;
	}

	// save settings for next time so user doesn't have to specify the directories for each export...
	Registry settings( Registry::LOCAL, "MedusaExport" );
	settings.put( "m_sPortsDirectory", CharString( m_sPortsDirectory ) );
	settings.put( "m_Images", CharString( m_Images ) );
	settings.put( "m_Materials", CharString( m_Materials ) );
	settings.put( "m_bPrefixNames", m_bPrefixNames );

	CDialog::OnOK();
}

void CExportDialog::OnBrowseImages() 
{
	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Texture Directory");
	dialog.m_strTitle = _T("Select the texture directory?");

	if ( dialog.DoBrowse() )
	{
		m_Images = dialog.m_strPath;
		if ( m_Images[ m_Images.GetLength() - 1 ] != '\\')
			m_Images += _T("\\");

		UpdateData( false );
	}
}


void CExportDialog::OnBrowseMaterials() 
{
	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Material Directory");
	dialog.m_strTitle = _T("Select the material directory?");

	if ( dialog.DoBrowse() )
	{
		m_Materials = dialog.m_strPath;
		if ( m_Materials[ m_Materials.GetLength() - 1 ] != '\\')
			m_Materials += _T("\\");

		UpdateData( false );
	}
}

void CExportDialog::OnBrowsePorts() 
{
	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Ports Directory");
	dialog.m_strTitle = _T("Select the ports directory?");

	if ( dialog.DoBrowse() )
	{
		m_sPortsDirectory = dialog.m_strPath;
		if ( m_sPortsDirectory[ m_sPortsDirectory.GetLength() - 1 ] != '\\')
			m_sPortsDirectory += _T("\\");

		UpdateData( false );
	}
}
