// ChoosePorts.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "Resource.h"
#include "ChoosePorts.h"

/////////////////////////////////////////////////////////////////////////////
// CChoosePorts dialog


CChoosePorts::CChoosePorts(CWnd* pParent /*=NULL*/)
	: CDialog(CChoosePorts::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChoosePorts)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChoosePorts::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChoosePorts)
	DDX_Control(pDX, IDC_PORT_LIST, m_PortList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChoosePorts, CDialog)
	//{{AFX_MSG_MAP(CChoosePorts)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_BUTTON1, OnRemoveAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoosePorts message handlers

BOOL CChoosePorts::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for(int i=0;i<Port::s_PortLibPaths.size();i++)
		m_PortList.InsertItem( 0, Port::s_PortLibPaths[i] );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChoosePorts::OnOK() 
{
	Port::s_PortLibPaths.release();
	for(int i=0;i<m_PortList.GetItemCount();i++)
		Port::s_PortLibPaths.push( m_PortList.GetItemText(i,0) );

	CDialog::OnOK();
}

void CChoosePorts::OnAdd() 
{
#ifndef _DEBUG
    CFileDialog dialog(TRUE, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, 
		_T("Ports|*Port.dll|Libraries|*.dll||"));
#else
    CFileDialog dialog(TRUE, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, 
		_T("Ports|*PortD.dll|Libraries|*.dll||"));
#endif

	TCHAR fileBuffer[ 16 * 1024 + 1];
	fileBuffer[0] = 0;

	dialog.m_ofn.lpstrFile = fileBuffer;
	dialog.m_ofn.nMaxFile = 16 * 1024;

	if (dialog.DoModal() == IDOK)
	{
		POSITION pos = dialog.GetStartPosition();
		while( pos != NULL )
		{
			CString portFile = dialog.GetNextPathName( pos );
			m_PortList.InsertItem( 0, portFile );
		}
    }
}

void CChoosePorts::OnRemove() 
{
	int removeItem = m_PortList.GetNextItem(-1, LVNI_SELECTED );
	m_PortList.DeleteItem( removeItem );
}

void CChoosePorts::OnRemoveAll() 
{
	m_PortList.DeleteAllItems();
}
