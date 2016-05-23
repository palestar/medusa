// Texture.cpp : implementation file
//

#include "stdafx.h"
#include "Port.h"
#include "Texture.h"

#include "Tools\ResourcerDoc\SelectResource.h"
#include "Tools\ImagePort\Port.h"

// CTexture dialog

IMPLEMENT_DYNAMIC(CTexture, CDialog)
CTexture::CTexture( CWnd* pParent /*=NULL*/)
	: CDialog(CTexture::IDD, pParent)
	, m_nIndex(0)
	, m_nUV(0)
	, m_sImagePort(_T(""))
	, m_sParameters(_T(""))
	, m_nType(0)
	, m_bMipMap(TRUE)
{}

CTexture::~CTexture()
{}

BOOL CTexture::OnInitDialog()
{
	CDialog::OnInitDialog();

	for( int nType = 0; nType <= PrimitiveSurface::LAST; ++nType )
		m_cTypeList.AddString( PrimitiveSurface::typeText( (PrimitiveSurface::Type)nType ) );

	UpdateData( FALSE );

	return TRUE;
}

void CTexture::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cTypeList);
	DDX_Text(pDX, IDC_EDIT2, m_nIndex);
	DDX_Text(pDX, IDC_EDIT3, m_nUV);
	DDX_Text(pDX, IDC_EDIT1, m_sImagePort);
	DDX_Text(pDX, IDC_EDIT4, m_sParameters);
	DDX_CBIndex(pDX, IDC_COMBO1, m_nType);
	DDX_Check(pDX, IDC_CHECK1, m_bMipMap);
}


BEGIN_MESSAGE_MAP(CTexture, CDialog)
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenImage)
	ON_BN_CLICKED(IDC_BUTTON3, OnBrowseImage)
END_MESSAGE_MAP()


// CTexture message handlers

void CTexture::OnOpenImage()
{
	AfxGetApp()->OpenDocumentFile( m_sImagePort );
	OnCancel();		// close this dialog
}

void CTexture::OnBrowseImage()
{
	UpdateData();

	// classKey for ImagePort is 3914102874601741960
	CSelectResource dialog( CLASS_KEY(ImagePort) );
	if ( dialog.DoModal() == IDOK)
	{
		m_sImagePort = dialog.m_Selected;
		UpdateData(FALSE);
	}
}
