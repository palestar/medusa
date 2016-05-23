// Settings.cpp : implementation file
//

#include "stdafx.h"
#include "ProfilerClient.h"
#include "ProfilerClientDlg.h"
#include "Settings.h"
#include "EditServer.h"
#include "EditAlarm.h"

#include "Standard/CharString.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog


CSettings::CSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettings)
	m_sLogFile = _T("");
	//}}AFX_DATA_INIT
}


void CSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettings)
	DDX_Control(pDX, IDC_LIST3, m_cAlarms);
	DDX_Control(pDX, IDC_LIST1, m_cServers);
	DDX_Text(pDX, IDC_EDIT1, m_sLogFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettings, CDialog)
	//{{AFX_MSG_MAP(CSettings)
	ON_BN_CLICKED(IDC_BUTTON7, OnLog)
	ON_BN_CLICKED(IDC_BUTTON1, OnAddServer)
	ON_BN_CLICKED(IDC_BUTTON2, OnEditServer)
	ON_BN_CLICKED(IDC_BUTTON3, OnDeleteServer)
	ON_BN_CLICKED(IDC_BUTTON4, OnAddAlarm)
	ON_BN_CLICKED(IDC_BUTTON5, OnEditAlarm)
	ON_BN_CLICKED(IDC_BUTTON6, OnDeleteAlarm)
	ON_BN_CLICKED(IDC_BUTTON8, OnToggleServers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettings message handlers

BOOL CSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	m_sLogFile = pParent->m_sLogFile;
	UpdateData( false );

	CRect rect;
	m_cServers.GetClientRect( &rect );
	m_cServers.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	int nWidth = rect.Width() / 4;
	m_cServers.InsertColumn( 0, _T("Address"), LVCFMT_LEFT, nWidth * 2, 0 );
	m_cServers.InsertColumn( 1, _T("Port"), LVCFMT_LEFT, nWidth, 1 );
	m_cServers.InsertColumn( 2, _T("Disabled"), LVCFMT_LEFT, nWidth, 2 );

	m_cAlarms.GetClientRect( &rect );
	m_cAlarms.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	nWidth = rect.Width() / 3;
	m_cAlarms.InsertColumn( 0, _T("Filter"), LVCFMT_LEFT, nWidth * 2, 0 );
	m_cAlarms.InsertColumn( 1, _T("Percent"), LVCFMT_LEFT, nWidth, 1 );

	updateServerList();
	updateAlarmList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettings::OnLog() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();

	CFileDialog dialog( FALSE, _T("log") );
	if ( dialog.DoModal() == IDOK )
		m_sLogFile = dialog.GetPathName();
	else
		m_sLogFile = _T("");
	UpdateData( false );

	pParent->m_sLogFile = m_sLogFile;
	pParent->saveSettings();
}

void CSettings::OnAddServer() 
{
	CEditServer dialog( this );
	if ( dialog.DoModal() == IDOK )
	{
		CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();

		CProfilerClientDlg::Server & server = pParent->m_Servers.push();
		server.bDisabled = dialog.m_bDisabled != 0;
		server.sAddress = dialog.m_sAddress;
		server.nPort = dialog.m_nPort;
		server.pClient = new ProfilerClient;

		pParent->saveSettings();

		updateServerList();
	}
}

void CSettings::OnEditServer() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<m_cServers.GetItemCount();i++)
	{
		if ( m_cServers.GetItemState( i, LVIS_SELECTED ) == 0 )
			continue;

		// selected list item found, bring up a dialog box to edit this server..
		CEditServer dialog( this );
		dialog.m_bDisabled = pParent->m_Servers[i].bDisabled;
		dialog.m_sAddress = pParent->m_Servers[i].sAddress;
		dialog.m_nPort = pParent->m_Servers[i].nPort;

		if ( dialog.DoModal() == IDOK )
		{
			pParent->m_Servers[i].bDisabled = dialog.m_bDisabled != 0;
			pParent->m_Servers[i].sAddress = dialog.m_sAddress;
			pParent->m_Servers[i].nPort = dialog.m_nPort;
			pParent->m_Servers[i].pClient->close();			// close the connection, to force the client to be reconnected on update..
		}
	}

	pParent->saveSettings();		// save any changes

	updateServerList();
}

void CSettings::OnDeleteServer() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<m_cServers.GetItemCount();i++)
	{
		if ( m_cServers.GetItemState( i, LVIS_SELECTED ) == 0 )
			continue;
		if ( MessageBox( _T("Are you sure you want to delete this server?"), m_cServers.GetItemText( i, 0 ), MB_YESNO ) != IDYES )
			continue;	

		delete pParent->m_Servers[i].pClient;
		pParent->m_Servers[i].pClient = NULL;
	}

	for(int j=0;j<pParent->m_Servers.size();)
	{
		if ( pParent->m_Servers[j].pClient == NULL )
			pParent->m_Servers.remove( j );
		else
			j++;
	}

	pParent->saveSettings();		// save any changes

	updateServerList();
}

void CSettings::OnToggleServers() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<m_cServers.GetItemCount();i++)
	{
		if ( m_cServers.GetItemState( i, LVIS_SELECTED ) == 0 )
			continue;
		pParent->m_Servers[i].bDisabled = !pParent->m_Servers[i].bDisabled;
	}

	pParent->saveSettings();		// save any changes
	updateServerList();
}

void CSettings::OnAddAlarm() 
{
	CEditAlarm dialog( this );
	if ( dialog.DoModal() == IDOK )
	{
		CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();

		CProfilerClientDlg::Alarm & alarm = pParent->m_Alarms.push();
		alarm.sNameFilter = dialog.m_sNameFilter;
		alarm.nPercent = dialog.m_nPercent;
		alarm.bAudibleAlarm = dialog.m_bAudibleAlaram != 0;

		pParent->saveSettings();

		updateAlarmList();
	}
}

void CSettings::OnEditAlarm() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<m_cAlarms.GetItemCount();i++)
	{
		if ( m_cAlarms.GetItemState( i, LVIS_SELECTED ) == 0 )
			continue;

		CProfilerClientDlg::Alarm & alarm = pParent->m_Alarms[i];

		// selected list item found, bring up a dialog box to edit this server..
		CEditAlarm dialog( this );
		dialog.m_sNameFilter = alarm.sNameFilter;
		dialog.m_nPercent = alarm.nPercent;
		dialog.m_bAudibleAlaram = alarm.bAudibleAlarm;

		if ( dialog.DoModal() == IDOK )
		{
			alarm.sNameFilter = dialog.m_sNameFilter;
			alarm.nPercent = dialog.m_nPercent;
			alarm.bAudibleAlarm = dialog.m_bAudibleAlaram != 0;
		}
	}

	pParent->saveSettings();		// save any changes

	updateAlarmList();
}

void CSettings::OnDeleteAlarm() 
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=m_cAlarms.GetItemCount()-1;i>=0;i--)
	{
		if ( m_cAlarms.GetItemState( i, LVIS_SELECTED ) == 0 )
			continue;

		pParent->m_Alarms.remove( i );
	}

	pParent->saveSettings();		// save any changes

	updateAlarmList();
}

//----------------------------------------------------------------------------

void CSettings::updateServerList()
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<pParent->m_Servers.size();i++)
	{
		CProfilerClientDlg::Server & server = pParent->m_Servers[i];

		if ( m_cServers.GetItemCount() <= i )
			m_cServers.InsertItem( i, CString(server.sAddress) );
		else
			m_cServers.SetItemText( i, 0, CString(server.sAddress) );
		m_cServers.SetItemText( i, 1, CString(CharString().format("%d", server.nPort )) );
		m_cServers.SetItemText( i, 2, server.bDisabled ? _T("Yes") : _T("No") );
	}

	while( m_cServers.GetItemCount() > pParent->m_Servers.size() )
		m_cServers.DeleteItem( m_cServers.GetItemCount() - 1 );
}

void CSettings::updateAlarmList()
{
	CProfilerClientDlg * pParent = (CProfilerClientDlg *)GetParent();
	for(int i=0;i<pParent->m_Alarms.size();i++)
	{
		CProfilerClientDlg::Alarm & alarm = pParent->m_Alarms[i];

		if ( m_cAlarms.GetItemCount() <= i )
			m_cAlarms.InsertItem( i, CString(alarm.sNameFilter) );
		else
			m_cAlarms.SetItemText( i, 0, CString(alarm.sNameFilter) );
		m_cAlarms.SetItemText( i, 1, CString(CharString().format("%d", alarm.nPercent )) );
	}

	while( m_cAlarms.GetItemCount() > pParent->m_Alarms.size() )
		m_cAlarms.DeleteItem( m_cAlarms.GetItemCount() - 1 );
}

//----------------------------------------------------------------------------
// EOF

