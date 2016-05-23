// SelectNode.cpp : implementation file
//

#include "stdafx.h"
#include "SelectNode.h"

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectNode dialog


CSelectNode::CSelectNode(ScenePort * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CSelectNode::IDD, pParent), m_pDoc( pDoc ), m_pSelected( NULL )
{
	//{{AFX_DATA_INIT(CSelectNode)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectNode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectNode)
	DDX_Control(pDX, IDC_TREE1, m_Nodes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectNode, CDialog)
	//{{AFX_MSG_MAP(CSelectNode)
	ON_BN_CLICKED(IDC_BUTTON1, OnNULL)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectNode message handlers

BOOL CSelectNode::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// get the root node
	BaseNodePort * pRoot = m_pDoc->m_Root;
	// build the tree
	buildTree( TVI_ROOT, pRoot );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectNode::OnOK() 
{
	HTREEITEM selectedItem = m_Nodes.GetSelectedItem();
	if ( selectedItem )
	{
		m_pSelected = (BaseNodePort *)m_Nodes.GetItemData( selectedItem );
		CDialog::OnOK();
	}
}

void CSelectNode::OnNULL() 
{
	m_pSelected = NULL;
	CDialog::OnOK();
}

//----------------------------------------------------------------------------

void CSelectNode::buildTree( HTREEITEM hParent, BaseNodePort * pNode )
{
	HTREEITEM item = m_Nodes.InsertItem( pNode->name(), hParent );
	m_Nodes.SetItemData( item, (DWORD)pNode );

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		buildTree( item, pNode->child(i) );

	m_Nodes.Expand( item, TVE_EXPAND );
}

//----------------------------------------------------------------------------
// EOF