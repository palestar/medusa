// ProfilerClientDlg.cpp : implementation file
//

#include "stdafx.h"
#pragma warning( disable : 4146 )

#include "ProfilerClient.h"
#include "ProfilerClientDlg.h"
#include "Settings.h"

#include "Standard/CommandLine.h"
#include "Debug/Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientDlg dialog

CProfilerClientDlg::CProfilerClientDlg( CWnd* pParent /*=NULL*/)
	: CDialog(CProfilerClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProfilerClientDlg)
	m_sStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProfilerClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfilerClientDlg)
	DDX_Control(pDX, IDC_TREE1, m_cProfileTree);
	DDX_Text(pDX, IDC_STATUS, m_sStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProfilerClientDlg, CDialog)
	//{{AFX_MSG_MAP(CProfilerClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, OnSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfilerClientDlg message handlers

BOOL CProfilerClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	m_ImageList.Create( 14,14,ILC_COLOR32 | ILC_MASK,0,0); 
	m_ImageList.Add( AfxGetApp()->LoadIcon(IDI_STATE_NORMAL) );
	m_ImageList.Add( AfxGetApp()->LoadIcon(IDI_STATE_ALARM) );
	m_cProfileTree.SetImageList( &m_ImageList, TVSIL_NORMAL );

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// load all servers and alarams
	loadSettings();

	// start the timer
	SetTimer( 0x1, 1000, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProfilerClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CProfilerClientDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CProfilerClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

float GetPercent( qword nCPU, qword nTotalCPU )
{
	if ( nTotalCPU > 0 )
		return ((float)nCPU * 100.0f) / (float)nTotalCPU;
	return 0.0f;
}

void DeleteChildItems( CTreeCtrl & tree, HTREEITEM hParent )
{
	HTREEITEM hChild = tree.GetChildItem( hParent );
	while( hChild )
	{
		HTREEITEM hNext = tree.GetNextSiblingItem( hChild );
		tree.DeleteItem( hChild );

		hChild = hNext;
	}
}

void CProfilerClientDlg::OnTimer(UINT nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	if ( nIDEvent == 0x1 )
	{
		// reset the alarm state
		m_bAlarm = false;

		HTREEITEM hServer = m_cProfileTree.GetRootItem();

		int nItem = 0;
		for(int j=0;j<m_Servers.size();j++)
		{
			Server & server = m_Servers[j];
			qword nTotalCPU = server.pClient->totalCPU();

			CharString sName;
			sName.format( "%s:%d - CPU: %s, CPU %%: %.2f%%", server.sAddress, server.nPort,
				FormatNumber<char,qword>( server.nPeakCPU ),
				GetPercent( server.nPeakCPU, nTotalCPU) );
			server.nPeakCPU = 0;

			bool bServerAlarm = false;

			// update / create the root item 
			if ( hServer == NULL || m_cProfileTree.GetItemData(hServer) != ((dword)server.pClient) )
			{
				hServer = m_cProfileTree.InsertItem( sName, TVI_ROOT, hServer );
				m_cProfileTree.SetItemData( hServer, (dword)server.pClient );
			}
			else
				m_cProfileTree.SetItemText( hServer, sName );		// update the existing item name

			// make sure we are connected to the server, if not try to connect...
			if (!server.pClient->connected() && !server.bDisabled )
			{
				if (! server.pClient->open( new Socket("ZLIB"), server.sAddress, server.nPort ) )
					bServerAlarm = m_bAlarm = true;
			}

			server.pClient->update();

			if ( !server.bDisabled && server.pClient->connected() )
			{
				server.pClient->lock();
				server.pClient->sendUpdate();

				// update our list
				for(int i=0;i<server.pClient->profileCount();i++)
				{
					const ProfilerClient::Profile & profile = server.pClient->profile( i );

					if ( profile.nCPU > server.nPeakCPU )
						server.nPeakCPU = profile.nCPU;

					// check alarm items
					bool bItemAlarm = false;
					for(int k=0;k<m_Alarms.size();k++)
					{
						Alarm & alarm = m_Alarms[ k ];
						if ( alarm.sNameFilter.length() > 0 && profile.sName.compareCommand( alarm.sNameFilter ) != 0 )
							continue;		// filter doesn't match
						
						if ( GetPercent( profile.nCPU, nTotalCPU ) >= alarm.nPercent )
						{
							bServerAlarm = bItemAlarm = true;
							if ( alarm.bAudibleAlarm )
								m_bAlarm = true;

							// record log entry
							LOG_CRITICAL( "CProfilerClientDlg", "Alarm triggered on %s:%d (%s), Thread: %u, CPU: %s, CPU %%: %.2f%%, Hits: %s",
								server.sAddress, server.nPort, profile.sName, profile.nThread, 
								FormatNumber<char,qword>( profile.nCPU ), 
								GetPercent( profile.nCPU, nTotalCPU),
								FormatNumber<char,qword>( profile.nHits ) );
						}
					}

					// search for the thread item
					HTREEITEM hThread = m_cProfileTree.GetChildItem( hServer );
					while( hThread != NULL )
					{
						if ( m_cProfileTree.GetItemData( hThread ) == profile.nThread )
							break;
						hThread = m_cProfileTree.GetNextSiblingItem( hThread );
					}

					if ( hThread == NULL )
					{
						// new thread, create tree item...
						hThread = m_cProfileTree.InsertItem( CharString().format("Thread: %u", profile.nThread) , hServer );
						m_cProfileTree.SetItemData( hThread, profile.nThread );
						m_cProfileTree.SortChildren( hServer );
					}

					HTREEITEM hPrevious = NULL;
					HTREEITEM hItem = m_cProfileTree.GetChildItem( hThread );
					while( hItem != NULL )
					{
						int nCompare = _strnicmp( m_cProfileTree.GetItemText( hItem ), profile.sName, profile.sName.length() );
						if ( nCompare >= 0 )
							break;

						hPrevious = hItem;
						hItem = m_cProfileTree.GetNextSiblingItem( hItem );
					}

					CharString sText;
					sText.format( "%s - CPU: %s, CPU %: %.2f%%, Hits: %s, Memory: %s k", profile.sName, 
						FormatNumber<char,qword>( profile.nCPU ),
						GetPercent( profile.nCPU, nTotalCPU ),
						FormatNumber<char,qword>( profile.nHits ),
						FormatNumber<char,int>( profile.nBytes / 1024 ) );

					if ( hItem == NULL )
						hItem = m_cProfileTree.InsertItem( sText, hThread, hPrevious );
					else
						m_cProfileTree.SetItemText( hItem, sText );
					m_cProfileTree.SetItemImage( hItem, bItemAlarm, bItemAlarm );
				}

				// check thread tree items
				HTREEITEM hThread = m_cProfileTree.GetChildItem( hServer );
				while( hThread != NULL )
				{
					dword nThreadId = m_cProfileTree.GetItemData( hThread );
					HTREEITEM hNext = m_cProfileTree.GetNextSiblingItem( hThread );

					bool bFound = false;
					for(int i=0;i<server.pClient->profileCount() && !bFound;i++)
						if ( server.pClient->profile( i ).nThread == nThreadId )
							bFound = true;

					if ( bFound )
					{
						// check child items, set image state
						bool bItemAlarm = false;
						
						HTREEITEM hItem = m_cProfileTree.GetChildItem( hThread );
						while( hItem != NULL )
						{
							int nImage, nSelected;
							m_cProfileTree.GetItemImage( hItem, nImage, nSelected);

							if ( nImage != 0 )
							{
								bItemAlarm = true;
								break;
							}

							hItem = m_cProfileTree.GetNextSiblingItem( hItem );
						}

						m_cProfileTree.SetItemImage( hThread, bItemAlarm, bItemAlarm );
					}
					else
					{
						// thread is gone, remove this tree item...
						m_cProfileTree.DeleteItem( hThread );
					}

					hThread = hNext;
				}


				server.pClient->unlock();
			}
			else
			{
				// failed to connect to server or server is disabled...
				DeleteChildItems( m_cProfileTree, hServer );
			}

			// set the tree item image
			m_cProfileTree.SetItemImage( hServer, bServerAlarm, bServerAlarm );
			// get the next tree item
			hServer = m_cProfileTree.GetNextSiblingItem( hServer );
		} // end for

		// remove extra tree items
		while( hServer != NULL )
		{
			HTREEITEM hNext = m_cProfileTree.GetNextSiblingItem( hServer );
			m_cProfileTree.DeleteItem( hServer );

			hServer = hNext;
		}

		/*
		m_sStatus = CString( CharString().format("Connected to %s:%d, Total CPU: %s", 
			server.sAddress, server.nPort, FormatNumber<char,qword>( nTotalCPU ) ) );
		*/
		//UpdateData( false );

		// play audible alerts
		if ( m_bAlarm )
			MessageBeep( MB_ICONEXCLAMATION );
	}
}

void CProfilerClientDlg::OnSettings() 
{
	if( KillTimer( 0x1 ) )
	{
		CSettings( this ).DoModal();
		SetTimer( 0x1, 1000, NULL );
	}
}

//----------------------------------------------------------------------------

const TCHAR * pSECTION = _T("ProfilerClient");

void CProfilerClientDlg::loadSettings()
{
	CWinApp * pApp = AfxGetApp();

	m_sLogFile = pApp->GetProfileString( pSECTION, _T("LogFile"), _T(".\\ProfilerClient.log") );

	// if a server/port was provided on the commandline, then use that instead of the ones stored in the registy
	CommandLine args( pApp->m_lpCmdLine );
	if ( args.argumentCount() == 2 )
	{
		Server & server = m_Servers.push();
		server.bDisabled = false;
		server.sAddress = args.argument( 0 );
		server.nPort = args.argumentInt( 1 );
		server.pClient = new ProfilerClient;
		server.nPeakCPU = 0;
	}
	else
	{
		int nServerCount = pApp->GetProfileInt( pSECTION, _T("ServerCount"), 0 );
		for(int i=0;i<nServerCount;i++)
		{
			CharString sBaseName;
			sBaseName.format("Server%d", i );

			Server & server = m_Servers.push();
			server.bDisabled = pApp->GetProfileInt( pSECTION, CString( sBaseName + "Disabled" ), 0 ) != 0;
			server.sAddress = pApp->GetProfileString( pSECTION, CString( sBaseName + "Address" ) );
			server.nPort = pApp->GetProfileInt( pSECTION, CString( sBaseName + "Port"), 0 );
			server.pClient = new ProfilerClient;
			server.nPeakCPU = 0;
		}
	}

	int nAlarmCount = pApp->GetProfileInt( pSECTION, _T("AlarmCount"), 0 );
	for(int j=0;j<nAlarmCount;j++)
	{
		CharString sBaseName;
		sBaseName.format("Alarm%d", j );

		Alarm & alarm = m_Alarms.push();
		alarm.sNameFilter = pApp->GetProfileString( pSECTION, CString( sBaseName + "Filter") );
		alarm.nPercent = pApp->GetProfileInt( pSECTION, CString( sBaseName + "Percent"), 0 ); 
		alarm.bAudibleAlarm = pApp->GetProfileInt( pSECTION, CString( sBaseName + "Audible"), 0 ) != 0;
	}
}

void CProfilerClientDlg::saveSettings()
{
	CWinApp * pApp = AfxGetApp();

	pApp->WriteProfileString( pSECTION, _T("LogFile"), CString(m_sLogFile) );
	pApp->WriteProfileInt( pSECTION, _T("ServerCount"), m_Servers.size() );
	for(int i=0;i<m_Servers.size();i++)
	{
		CharString sBaseName;
		sBaseName.format("Server%d", i );

		Server & server = m_Servers[i];
		pApp->WriteProfileInt( pSECTION, CString(sBaseName + "Disabled"), server.bDisabled );
		pApp->WriteProfileString( pSECTION, CString( sBaseName + "Address"), CString(server.sAddress) );
		pApp->WriteProfileInt( pSECTION, CString(sBaseName + "Port"), server.nPort );
	}

	pApp->WriteProfileInt( pSECTION, _T("AlarmCount"), m_Alarms.size() );
	for(int j=0;j<m_Alarms.size();j++)
	{
		CharString sBaseName;
		sBaseName.format("Alarm%d", j );

		Alarm & alarm = m_Alarms[j];
		pApp->WriteProfileString( pSECTION, CString(sBaseName + "Filter"), CString(alarm.sNameFilter) );
		pApp->WriteProfileInt( pSECTION, CString(sBaseName + "Percent"), alarm.nPercent );
		pApp->WriteProfileInt( pSECTION, CString(sBaseName + "Audible"), alarm.bAudibleAlarm );
	}
}

//----------------------------------------------------------------------------
// EOF

