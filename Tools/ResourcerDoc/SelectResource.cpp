// SelectResource.cpp : implementation file
//

#include "stdafx.h"

#include "SelectResource.h"
#include "ResourcerDoc.h"
#include "Resource.h"

#include "File/Path.h"
#include "File/FindFile.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectResource dialog


CSelectResource::CSelectResource( const ClassKey &typeKey, CWnd* pParent /*=NULL*/)
	: m_TypeKey( typeKey ), CDialog(CSelectResource::IDD, pParent)
{
	m_Selected = "";
	m_CurrentDirectory = ""; 

	//{{AFX_DATA_INIT(CSelectResource)
	//}}AFX_DATA_INIT
}

void CSelectResource::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectResource)
	DDX_Control(pDX, IDC_FILE_TREE, m_FileTree );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectResource, CDialog)
	//{{AFX_MSG_MAP(CSelectResource)
	ON_NOTIFY( NM_DBLCLK, IDC_FILE_TREE, OnDblclkFileTree )
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_FILE_TREE, OnTvnItemexpanding)
	ON_BN_CLICKED(IDC_BUTTON1, OnNull)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectResource message handlers

void CSelectResource::BuildTree( const CharString & sPath, const CharString & sFolder, HTREEITEM hParent )
{
	hParent = m_FileTree.InsertItem( sFolder, 0, 0, hParent );
	m_FileTree.SetItemData( hParent, 0 );
	
	TVITEM tvitem;
	memset( &tvitem, 0, sizeof(tvitem) );
	tvitem.hItem = hParent;
	tvitem.mask = TVIF_CHILDREN;
	tvitem.cChildren = 1;

	m_FileTree.SetItem( &tvitem );		// force folders to show buttons always..

	Path SearchPath;
	SearchPath.setDirectory( sPath + sFolder );

	// add the folders to the list
	FindFile ff( SearchPath.directory() + "*.*" );
	ff.sortDirectories();

	for(int i=0;i<ff.directoryCount();i++)
	{
		CharString sFolder = ff.directory( i );
		if ( sFolder[0] == '.' )
			continue;
		BuildTree( SearchPath.directory(), sFolder + "\\", hParent );
	}
	//m_FileTree.SortChildren( hParent );

	if ( _stricmp( SearchPath.directory(), CResourcerDoc::document()->currentDirectory() ) == 0 )
	{
		m_FileTree.SelectItem( hParent );
		m_FileTree.Expand( hParent, TVE_EXPAND );
	}
}

BOOL CSelectResource::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	m_ImageList.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_ImageList.Add( pApp->LoadIcon( IDI_FOLDER ) );
	for(int j=0;j<Port::portCount();j++)
		m_ImageList.Add( Port::portIcon( j ));
	
	m_FileTree.SetImageList( &m_ImageList, TVSIL_NORMAL );

	CResourcerDoc * pDoc = CResourcerDoc::document();
	if ( pDoc == NULL )
		return FALSE;

	BuildTree( "", pDoc->rootDirectory(), TVI_ROOT );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectResource::OnOK() 
{
	HTREEITEM hSelected = m_FileTree.GetSelectedItem();
	if ( hSelected && m_FileTree.GetItemData( hSelected ) == 1 )
	{
		m_Selected = "";
		while( hSelected != NULL )
		{
			m_Selected = m_FileTree.GetItemText( hSelected ) + m_Selected;
			hSelected = m_FileTree.GetParentItem( hSelected );
		}

		m_Selected = CResourcerDoc::document()->relative( m_Selected );
		CDialog::OnOK();
	}
}

void CSelectResource::OnNull() 
{
	m_Selected = "";
	CDialog::OnOK();
}

void CSelectResource::OnDblclkFileTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();
}

void CSelectResource::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	HTREEITEM hExpanding = pNMTreeView->itemNew.hItem;
	if ( hExpanding && m_FileTree.GetItemData( hExpanding ) == 0 )
	{
		CString sFolder = "";

		HTREEITEM hPath = hExpanding;
		while( hPath != NULL )
		{
			sFolder = m_FileTree.GetItemText( hPath ) + sFolder;
			hPath = m_FileTree.GetParentItem( hPath );
		}

		FindFile ff( sFolder + "*.prt" );
		ff.sortFiles();

		for(int j=0;j<ff.fileCount();j++)
		{
			CharString sFileName = ff.file( j );

			Port::PortInfo info;
			if ( Port::portInfo( sFolder + sFileName, info ) )
			{
				if ( Factory::isType( m_TypeKey, info.classKey ) )
				{
					HTREEITEM hItem = m_FileTree.InsertItem( sFileName, info.type + 1, info.type + 1, hExpanding );
					m_FileTree.SetItemData( hItem, 1 );
				}
			}
		}

		m_FileTree.SetItemData( hExpanding, 2 );
		//m_FileTree.SortChildren( hExpanding );
	}
}

//-------------------------------------------------------------------------------
// EOF


