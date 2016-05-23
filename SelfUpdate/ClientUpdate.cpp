// ClientUpdate.cpp : implementation file
//

#define SELFUPDATE_DLL
#include "stdafx.h"
#include "ClientUpdate.h"

#include "File/FileDisk.h"
#include "Standard/CommandLine.h"
#include "Standard/MD5.h"
#include "Standard/CharString.h"
#include "Standard/Settings.h"
#include "Standard/Process.h"
#include "Standard/Time.h"

//----------------------------------------------------------------------------

ClientUpdate::ClientUpdate(const char * mirrorDirectory, const char * mirrorServer, int port, 
	const char * pCatalog /*= NULL*/, CWnd* pParent /*=NULL*/, 
	dword nSID /*= 0*/, bool bSelfUpdating /*= true*/ )
	: CDialog(ClientUpdate::IDD, pParent), m_bExit( false ),
	m_nSID( nSID ), m_bSelfUpdating( bSelfUpdating ), m_pUpdateThread( NULL ), m_bError( false )
{
	m_MirrorDirectory = mirrorDirectory;
	m_MirrorServer = mirrorServer;
	m_MirrorPort = port;
	m_nStartTime = 0;
	m_nTotalBytes = 0;
	m_nBytesReceived = 0;
	m_nFileSize = 0;
	m_nFileReceived = 0;

	if ( pCatalog != NULL )
		setLocalCatalog( pCatalog );
	
	// start the update thread
	m_pUpdateThread = new UpdateThread( this );
	m_pUpdateThread->resume();

	//{{AFX_DATA_INIT(ClientUpdate)
	m_FileStatus = _T("");
	m_Status = _T("");
	//}}AFX_DATA_INIT
}

ClientUpdate::~ClientUpdate()
{
	m_bExit = true;
	close();

	if ( m_pUpdateThread != NULL )
	{
		m_pUpdateThread->kill();

		delete m_pUpdateThread;
		m_pUpdateThread = NULL;
	}
}


void ClientUpdate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ClientUpdate)
	DDX_Control(pDX, IDC_SELF_UPDATE_PROGRESS2, m_FileProgress);
	DDX_Control(pDX, IDC_SELF_UPDATE_PROGRESS1, m_Progress);
	DDX_Text(pDX, IDC_SELF_UPDATE_FILESTATUS, m_FileStatus);
	DDX_Text(pDX, IDC_SELF_UPDATE_STATUS, m_Status);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ClientUpdate, CDialog)
	//{{AFX_MSG_MAP(ClientUpdate)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ClientUpdate message handlers

BOOL ClientUpdate::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Status.Format(_T("Connecting to %s..."), m_MirrorServer );
	UpdateData( false );

	// start update event
	SetTimer( 0x2, 100, NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ClientUpdate::OnTimer(UINT nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	switch( nIDEvent )
	{
	case 0x2:	// update event
		{
			AutoLock Lock( &m_Lock );
			if ( m_nTotalBytes > 0 )
			{
				CString title;
				title.Format( _T("%d%% Complete..."), (m_nBytesReceived * 100) / m_nTotalBytes );
				SetWindowText( title );

				m_Progress.SetRange32( 0, m_nTotalBytes / 1000 );
				m_Progress.SetPos( m_nBytesReceived / 1000 );

				dword nElapsed = Time::seconds() - m_nStartTime;
				dword nBPS = 0;
				if ( nElapsed > 0 )
					nBPS = m_nBytesReceived / nElapsed;
		
				m_Status.Format( "Downloading %s of %s at %s/sec", 
					formatBytes( m_nBytesReceived ), 
					formatBytes( m_nTotalBytes ), 
					formatBytes( nBPS ) );

				if ( m_nBytesReceived > 0 )
				{
					double dElapsed = nElapsed;
					dword nRemain = (dword)((dElapsed * (m_nTotalBytes - m_nBytesReceived)) / m_nBytesReceived);

					CString sTime;
					if ( nRemain >= 3600 )
						sTime.Format( "; %2.2u:%2.2u:%2.2u remain", nRemain / 3600, (nRemain / 60) % 60, nRemain % 60 );
					else
						sTime.Format( "; %2.2u:%2.2u remain", (nRemain / 60) % 60, nRemain % 60 );

					m_Status += sTime;
				}
			}

			if ( m_nFileSize > 0 )
			{
				m_FileStatus.Format( _T("Downloading '%s', %d%% Complete"), 
					m_sFileProgress, (m_nFileReceived * 100) / m_nFileSize );
				m_FileProgress.SetRange32( 0, m_nFileSize / 1000 );
				m_FileProgress.SetPos( m_nFileReceived / 1000 );
			}
			
			UpdateData( false );

			// check for exit
			if ( m_bExit )
			{
				KillTimer( 0x2 );
				Lock.release();

				close();

				EndDialog( IDOK );
			}
		}
		break;
	}
}

void ClientUpdate::OnCancel() 
{
	// stop the update timer
	KillTimer( 0x2 );

	// close the connection
	m_bExit = true;
	close();

	CDialog::OnCancel();
}

//----------------------------------------------------------------------------

void ClientUpdate::onStatus( const char * pStatus )
{
	AutoLock Lock( &m_Lock );
	m_Status = pStatus;
}

void ClientUpdate::onError( const char * pError )
{
	AutoLock Lock( &m_Lock );
	if (!m_bExit)
	{
		//::MessageBox( m_hWnd, pError, _T("Mirror Client"), MB_OK );
		m_Status = pError;
		m_bExit = true;
	}
}

void ClientUpdate::onProgress( dword nTotalBytes, dword nBytes )
{
	AutoLock Lock( &m_Lock );
	m_nTotalBytes = nTotalBytes;
	m_nBytesReceived = nBytes;
}

void ClientUpdate::onFileProgress( const char * pFile, dword nTotalBytes, dword nBytes )
{
	AutoLock Lock( &m_Lock );
	m_sFileProgress = pFile;
	m_nFileSize = nTotalBytes;
	m_nFileReceived = nBytes;
}

bool ClientUpdate::onFileDelete( const char * pFile )
{
	return false;
	//if ( strstr( pFile, "config.ini" ) )
	//	return false;	
	//return true;
}

//----------------------------------------------------------------------------

bool ClientUpdate::updateSelf( const char * pSection, const char * pAddress /*= NULL*/, int nPort /*= 0*/,
	const char * pConfig /*= NULL*/, const char * pCatalog /*= NULL*/ )
{
#ifndef _DEBUG
	// seek to the home directory
	FileDisk::seekHome();
#endif

	Settings settings( pSection, pConfig );
	if ( settings.get( "doUpdate", 1 ) != 0 )
	{
		TCHAR sExecutable[ MAX_PATH ];
		GetModuleFileName( GetModuleHandle(NULL), sExecutable, MAX_PATH );
		TCHAR sPath[ MAX_PATH ];
		strcpy_s( sPath, sizeof(sPath), sExecutable );
		
		// remove the EXE from the path
		TCHAR * pPathEnd = _tcsrchr( sPath, '\\' );
		if ( pPathEnd == NULL )
			return false;
		*(pPathEnd + 1) = 0;

		CString sUpdateExec = CString(sPath) + settings.get( "UpdateExec", "SmartUpdate.exe" );
		CString sUpdateExec2 = CString(sPath) + settings.get( "UpdateExec2", "SmartUpdate2.exe" );
		CString sMirrorAddress = settings.get( "mirrorAddress", pAddress != NULL ? pAddress : "mirror-server.palestar.com" );
		int		nMirrorPort = settings.get( "mirrorPort", nPort != 0 ? nPort : (int)0 );

		if ( nMirrorPort == 0 )
			return false;

		// get the newest files
		ClientUpdate update( sPath, sMirrorAddress, nMirrorPort ); 
		if ( update.DoModal() == IDCANCEL )
			return false;

		// get the command line and extract just the arguments
		CommandLine commands( CharString( GetCommandLine() ), true );
		CString sArguments = commands.restring( 1 );

		CString sPostExecute;
		sPostExecute.Format("%s %s", sExecutable, sArguments);

		CString sUpdateFile;
		sUpdateFile.Format( _T("%sUpdate.ini"), sPath );

		Settings updateConfig( "Update", (CharString)sUpdateFile );
		updateConfig.put( "ProcessId", GetCurrentProcessId() );
		updateConfig.put( "Path", (CharString) sPath );
		updateConfig.put( "PostExecute", (CharString)sPostExecute );

		// copy our current SmartUpdate.exe to SmartUpdate2.exe
		FileDisk::copyFile( sUpdateExec, sUpdateExec2 );

		CString sUpdateProcess;
		sUpdateProcess.Format(_T("%s %s"), sUpdateExec2, sUpdateFile );
		if ( Process::start( CharString( sUpdateProcess ) ) )
		{
			// update process launched, exit this application so it can update
			settings.put( "doUpdate", (dword)0 );
			exit( 0 );
		}

		// failed to start smart update
		return false;
	}

	// set the doUpdate back to true for the next time..
	settings.put( "doUpdate", 1 );
	return true;
}

//----------------------------------------------------------------------------

CString	ClientUpdate::formatBytes( dword bytes )
{
	CString string;

	if ( bytes > 1000000 )
		string.Format( _T("%.1f MB"), float( bytes ) / 1000000.0f );
	else
		string.Format( _T("%.1f KB"), float(bytes) / 1000.0f );

	return string;
}

//----------------------------------------------------------------------------

int ClientUpdate::doUpdate()
{
	if (! open( m_MirrorServer, m_MirrorPort, m_MirrorDirectory, NULL, m_bSelfUpdating ) )
	{
		// failed to open and the done flag not set means we didn't get the connection
		m_Status.Format( "Failed to connect to %s...", m_MirrorServer );
		m_bExit = m_bError = true;
		return -1;
	}
	if ( m_nSID != 0 )
		login( m_nSID );

	// set the start timer
	m_nStartTime = Time::seconds();
	
	// download the files..
	dword nJobId = syncronize();
	if ( nJobId != 0 )
		waitJob( nJobId, 86400 * 1000 );

	m_bExit = true;
	return 0;
}

//---------------------------------------------------------------------------------------------------
