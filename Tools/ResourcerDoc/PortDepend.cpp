// PortDepend.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "Resource.h"
#include "PortDepend.h"



/////////////////////////////////////////////////////////////////////////////
// CPortDepend dialog


CPortDepend::CPortDepend(Array< CharString > & dep, CWnd* pParent /*=NULL*/)
	: CDialog(CPortDepend::IDD, pParent), m_Depend( dep )
{
	//{{AFX_DATA_INIT(CPortDepend)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPortDepend::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortDepend)
	DDX_Control(pDX, IDC_TREE1, m_DependTree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortDepend, CDialog)
	//{{AFX_MSG_MAP(CPortDepend)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnOpenDepend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortDepend message handlers

BOOL CPortDepend::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<m_Depend.size();i++)
		if ( m_Depend[i].length() > 0 )
			buildTree( m_DependTree.InsertItem( m_Depend[i] ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPortDepend::OnOpenDepend(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM treeItem = m_DependTree.GetSelectedItem();
	if ( treeItem  )
		AfxGetApp()->OpenDocumentFile( m_DependTree.GetItemText( treeItem ) );

	*pResult = 0;
}

//----------------------------------------------------------------------------

void CPortDepend::buildTree( HTREEITEM parent )
{
	// load the port
	Port::Ref port = Port::portLoad( m_DependTree.GetItemText( parent ) );
	if ( port.valid() )
	{
		Array< CharString > depend;
		port->dependencies( depend );

		for(int i=0;i<depend.size();i++)
			if ( depend[i].length() > 0 )
				buildTree( m_DependTree.InsertItem( depend[i], parent ) );
	}
}

//----------------------------------------------------------------------------
// EOF
