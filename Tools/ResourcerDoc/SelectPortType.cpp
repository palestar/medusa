// SelectPortType.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "SelectPortType.h"

#include "Port.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectPortType dialog


CSelectPortType::CSelectPortType(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectPortType::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectPortType)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_SelectedPort = -1;
}


void CSelectPortType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPortType)
	DDX_Control(pDX, IDC_PORT_LIST, m_PortList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectPortType, CDialog)
	//{{AFX_MSG_MAP(CSelectPortType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPortType message handlers

void CSelectPortType::OnOK() 
{
	int selectedItem = m_PortList.GetNextItem( -1, LVNI_SELECTED );
	if ( selectedItem >= 0 )
		m_SelectedPort = m_PortList.GetItemData( selectedItem );

	CDialog::OnOK();
}

BOOL CSelectPortType::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<Port::portCount();i++)
	{
		int listItem = m_PortList.InsertItem( 0, Port::portName( i ) );
		m_PortList.SetItemData( listItem, i );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
