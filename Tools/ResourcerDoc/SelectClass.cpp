// SelectClass.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "SelectClass.h"

#include "Resource.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CSelectClass, CDialog)
CSelectClass::CSelectClass(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectClass::IDD, pParent), m_Base( Widget::classKey() ), m_Selected( NULL_CLASS )
{
}

CSelectClass::~CSelectClass()
{}

void CSelectClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_cClassTree);
}


BEGIN_MESSAGE_MAP(CSelectClass, CDialog)
END_MESSAGE_MAP()

static int EnumClasses( CTreeCtrl & tree, HTREEITEM hParent, const ClassKey & key )
{
   int nCount = 0;

   Factory * pFactory = Factory::findFactory( key );
   if ( pFactory != NULL )
   {
      nCount++;

      hParent = tree.InsertItem( pFactory->className(), hParent );
      tree.SetItemData( hParent, (DWORD)&pFactory->classKey() );

	  for(int i=0;i<Factory::typeCount( key );i++)
		nCount += EnumClasses( tree, hParent, Factory::type( key, i ) );
   }

   tree.SortChildren( hParent );
   tree.Expand( hParent, TVE_EXPAND );

   return nCount;

}

BOOL CSelectClass::OnInitDialog()
{
	CDialog::OnInitDialog();

	EnumClasses( m_cClassTree, TVI_ROOT, m_Base );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectClass::OnOK()
{
	HTREEITEM pSelected = m_cClassTree.GetSelectedItem();
	if ( pSelected != NULL )
	{
		m_Selected = *((ClassKey *)m_cClassTree.GetItemData( pSelected ));
		CDialog::OnOK();
	}
}
