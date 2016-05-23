// SelectImportPort.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "SelectImportPort.h"

#include "Port.h"



/////////////////////////////////////////////////////////////////////////////
// CSelectImportPort dialog


CSelectImportPort::CSelectImportPort(const char * ext, CWnd* pParent /*=NULL*/)
	: CDialog(CSelectImportPort::IDD, pParent), m_ImportExt( ext ), m_SelectedPort(-1)
{
	//{{AFX_DATA_INIT(CSelectImportPort)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectImportPort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectImportPort)
	DDX_Control(pDX, IDC_PORT_LIST, m_PortList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectImportPort, CDialog)
	//{{AFX_MSG_MAP(CSelectImportPort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectImportPort message handlers

BOOL CSelectImportPort::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Array< ClassKey > & ports = Port::s_PortImportExt[ m_ImportExt ];
	for(int i=0;i<ports.size();i++)
	{
		int port = Port::portIndex( ports[i] );
		int listItem = m_PortList.InsertItem( 0, Port::portName( port ) );
		m_PortList.SetItemData( listItem, port );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectImportPort::OnOK() 
{
	int selectedItem = m_PortList.GetNextItem( -1, LVNI_SELECTED );
	if ( selectedItem < 0 )
		return;

	m_SelectedPort = m_PortList.GetItemData( selectedItem );
	
	CDialog::OnOK();
}


