// SelectBits.cpp : implementation file
//

#include "stdafx.h"
#include "PropertyList.h"
#include "SelectBits.h"

// CSelectBits dialog

IMPLEMENT_DYNAMIC(CSelectBits, CDialog)
CSelectBits::CSelectBits(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectBits::IDD, pParent), m_nBits( 0 ), m_pProperty( NULL )
{}

CSelectBits::~CSelectBits()
{}

void CSelectBits::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_LIST2, m_cBitList);
}


BEGIN_MESSAGE_MAP(CSelectBits, CDialog)
//   ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnItemclickList2)
//ON_NOTIFY(NM_CLICK, IDC_LIST2, OnNMClickList2)
END_MESSAGE_MAP()


// CSelectBits message handlers

BOOL CSelectBits::OnInitDialog()
{
   CDialog::OnInitDialog();

   CRect r;
   m_cBitList.GetClientRect( &r );

   m_cBitList.SetExtendedStyle(m_cBitList.GetStyle()|LVS_EX_CHECKBOXES);
   m_cBitList.InsertColumn(0,"Name",LVCFMT_LEFT,r.Width() );
   m_cBitList.InsertColumn(1,"Bit",LVCFMT_LEFT,100 );

   if ( m_pProperty != NULL )
   {
      for(int i=0;i<m_pProperty->optionCount();i++)
      {
         const Property::Option & option = m_pProperty->option( i );
         dword nBits = strtoul( option.sValue.cstr(), NULL, 10 );

         int nItem = m_cBitList.InsertItem( i, option.sName.cstr() );
         m_cBitList.SetItemText( nItem, 1, option.sName.cstr() );
         m_cBitList.SetItemData( nItem, nBits );

         if ( (nBits & m_nBits) == nBits )
            m_cBitList.SetCheck( nItem );
      }
   }

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectBits::OnOK()
{
   if ( m_pProperty != NULL )
   {
      dword nBits = 0;
      for(int i=0;i<m_cBitList.GetItemCount();i++)
         if ( m_cBitList.GetCheck( i ) )
         {
            nBits |= m_cBitList.GetItemData( i );
         }

      m_nBits = nBits;
   }

   CDialog::OnOK();
}

//void CSelectBits::OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult)
//{
//   NMITEMACTIVATE * pia =(NMITEMACTIVATE*)pNMHDR;
//   
//   if ( pia->iItem >= 0 )
//      m_cBitList.SetCheck( pia->iItem, !m_cBitList.GetCheck( pia->iItem ) );
//   // TODO: Add your control notification handler code here
//   *pResult = 0;
//}
