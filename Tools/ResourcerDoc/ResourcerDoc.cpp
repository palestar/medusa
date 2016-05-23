// ResourcerDoc.cpp : implementation of the CResourcerDoc class
//

#include "stdafx.h"
#include "Resource.h"

#include "ResourcerDoc.h"
#include "DocumentOptions.h"
#include "ChoosePorts.h"
#include "ChildFrm.h"
#include "ProgDlg.h"

#include "File/Path.h"
#include "File/FindFile.h"
#include "Standard/StringHash64.h"
#include "System/LocalizedString.h"
#include "File/FileDisk.h"
#include "Network/MirrorClient.h"
#include "Resource/Collection.h"

//----------------------------------------------------------------------------

// in the old code, we generated the key from the full path, which make the resource keys
// dependent on where the ports were stored. Now the key is generated only off a relative
// path in the root directory.
const StringHash64 WIDGET_SEED( "m:\\Ports\\" );

/////////////////////////////////////////////////////////////////////////////
// CResourcerDoc

IMPLEMENT_DYNCREATE(CResourcerDoc, CDocument)

BEGIN_MESSAGE_MAP(CResourcerDoc, CDocument)
	//{{AFX_MSG_MAP(CResourcerDoc)
	ON_COMMAND(ID_DOCUMENT_OPTIONS, OnDocumentOptions)
	ON_COMMAND(ID_DOCUMENT_BUILD, OnDocumentBuild)
	ON_COMMAND(ID_DOCUMENT_CLEAN, OnDocumentClean)
	ON_COMMAND(ID_DOCUMENT_CLEAROUTPUT, OnDocumentClearoutput)
	ON_COMMAND(ID_DOCUMENT_SYNCRONIZE, OnDocumentSyncronize)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
END_MESSAGE_MAP()

//----------------------------------------------------------------------------

CResourcerDoc * CResourcerDoc::s_pDocument = NULL;

//-------------------------------------------------------------------------------

CResourcerDoc::CResourcerDoc() : m_pMirrorClient( NULL ), m_nSyncronizeJob( 0 )
{
	m_bBulding = false;
	m_nBuildStart = 0;
	m_CurrentDirectory = "";
	m_pMirrorClient = new ResourcerMirrorClient();
	m_MirrorServerPort = 0;
}

CResourcerDoc::~CResourcerDoc()
{
	s_pDocument = NULL;
	delete m_pMirrorClient;
}

BOOL CResourcerDoc::OnNewDocument()
{
	if ( s_pDocument != NULL )
	{
		MessageBox(NULL,TEXT("Resourcer document already open"),TEXT("Error"), MB_OK );
		return FALSE;
	}

	if (!CDocument::OnNewDocument())
		return FALSE;

	bool bDocumentReady = false;
	while(! bDocumentReady )
	{
		if (!bDocumentReady && DocumentOptions(this).DoModal() != IDOK)
			return FALSE;

		if (! (bDocumentReady = m_Broker.open( m_BrokerFolder, true, false )) )
			MessageBox(NULL,TEXT("Failed to open data folder!"),TEXT("Error"), MB_OK );
	}

	// initialize the locale file
	if ( m_sLocaleFile.length() > 0 )
		LocalizedString::locale().setLocaleFile( m_sLocaleFile );

	// set the document
	s_pDocument = this;

	return TRUE;
}

const dword RESOURCER_ID = 'RSR ';
const dword VERSION_090299 = 90299;
const dword VERSION_072701 = 72701;
const dword VERSION_081601 = 81601;
const dword VERSION_20040606 = 20040606;

BOOL CResourcerDoc::OnOpenDocument(const char * lpszPathName) 
{
	if ( s_pDocument != NULL )
	{
		MessageBox(NULL,TEXT("Resourcer document already open"),TEXT("Error"), MB_OK );
		return FALSE;
	}

	try {
		InStream input( new FileDisk( lpszPathName, FileDisk::READ ) );
		
		// validate the id
		dword id, version;
		input >> id >> version;

		if ( id != RESOURCER_ID )
			return FALSE;

		switch( version )
		{
		case VERSION_20040606:
			input >> m_RootDirectory;
			input >> m_BrokerFolder;
			input >> m_MirrorServer;
			input >> m_MirrorServerPort;
			input >> m_UID;
			input >> m_PW;
			input >> m_sLocaleFile;
			break;
		case VERSION_081601:
			input >> m_RootDirectory;
			input >> m_BrokerFolder;
			input >> m_MirrorServer;
			input >> m_MirrorServerPort;
			input >> m_UID;
			input >> m_PW;
			m_sLocaleFile = m_BrokerFolder + TEXT("locale.txt");
			break;
		case VERSION_072701:
			input >> m_RootDirectory;
			input >> m_BrokerFolder;
			input >> m_DependentTree;
			input >> m_MirrorServer;
			input >> m_MirrorServerPort;
			input >> m_UID;
			input >> m_PW;
			m_sLocaleFile = m_BrokerFolder + TEXT("locale.txt");
			m_DependentTree.release();
			break;
		case VERSION_090299:
			input >> m_RootDirectory;
			input >> m_BrokerFolder;
			input >> m_DependentTree;
			m_sLocaleFile = m_BrokerFolder + TEXT("locale.txt");
			break;
		default:
			return FALSE;
		}

		if ( !m_RootDirectory.endsWith( "\\") )
			m_RootDirectory += '\\';
		if ( !m_BrokerFolder.endsWith( "\\") )
			m_BrokerFolder += '\\';

		bool bDocumentReady = false;
		while(! bDocumentReady )
		{
			if (! (bDocumentReady = m_Broker.open( m_BrokerFolder, true, false )) )
				MessageBox(NULL,TEXT("Failed to open data folder!"),TEXT("Error"), MB_OK );

			// if we failed, display the options dialog and try again or close the document
			if (!bDocumentReady && DocumentOptions(this).DoModal() != IDOK)
				return FALSE;
		}

		// load the locale file
		if ( m_sLocaleFile.length() > 0 )
			LocalizedString::locale().load( m_sLocaleFile, true );
		// set the document
		s_pDocument = this;
	}
	catch( File::FileError )
	{
		return FALSE;
	}

	m_ReverseHash.clear();

	// attempt to load the manifest file as well..
	CharString sKeyManifest( Path( lpszPathName ).directory() + "KeyManifest.txt" );

	FILE * pFile = fopen( sKeyManifest, "r" );
	if ( pFile != NULL )
	{
		char sLine[ MAX_PATH + 64 ];
		while( fgets( sLine, sizeof(sLine), pFile ) )
		{
			if ( sLine[0] == '#' || sLine[0] == 0 )
				continue;

			char * pEquals = strchr( sLine, '=' );
			if ( pEquals != NULL )
			{
				*pEquals = 0;
				
				WidgetKey nKey( sLine );
				CharString sFile( pEquals + 1 );
				m_ReverseHash[ nKey ] = sFile;
			}
		}

		fclose( pFile );
	}
	

	return TRUE;
}

BOOL CResourcerDoc::OnSaveDocument(const char * lpszPathName) 
{
	// attempt to save the manifest file as well..
	CharString sKeyManifest( Path( lpszPathName ).directory() + "KeyManifest.txt" );

	FILE * pFile = fopen( sKeyManifest, "w" );
	if ( pFile != NULL )
	{
		for( ReverseHash::iterator iKey = m_ReverseHash.begin();
			iKey != m_ReverseHash.end(); ++iKey )
		{
			fprintf( pFile, "%s=%s\n", iKey->first.string().cstr(), iKey->second.cstr() );
		}
		fclose( pFile );
	}

	bool bSaved = false;
	try {
		OutStream output( new FileDisk( lpszPathName, FileDisk::WRITE ) );
		output << RESOURCER_ID;
		output << VERSION_20040606;
		output << m_RootDirectory;
		output << m_BrokerFolder;
		output << m_MirrorServer;
		output << m_MirrorServerPort;
		output << m_UID;
		output << m_PW;
		output << m_sLocaleFile;
		bSaved = true;
	}
	catch( File::FileError )
	{}

	if (! bSaved )
		return FALSE;			// failed to save!

	// save the localized strings
	if ( m_sLocaleFile.length() > 0 )
		LocalizedString::locale().save( m_sLocaleFile );

	SetModifiedFlag( false );

	return TRUE;
}

void CResourcerDoc::OnCloseDocument() 
{
	// auto-save the resourcer document
	if (! GetPathName().IsEmpty() )
		OnSaveDocument( GetPathName() );

	CDocument::OnCloseDocument();
}

#ifdef _DEBUG
void CResourcerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CResourcerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------------------

const char * CResourcerDoc::rootDirectory() const
{
	return( m_RootDirectory );
}

const char * CResourcerDoc::brokerFolder() const
{
	return( m_BrokerFolder );
}

const char * CResourcerDoc::currentDirectory() const
{
	return( m_CurrentDirectory );
}

CharString CResourcerDoc::relative( const char * pFileName ) const
{
	CharString sFile( pFileName );
	sFile.replace( '/', '\\' );		// make sure all the slashes are the correct direction...

	while(! FileDisk::fileExists( m_RootDirectory + sFile ) )
	{
		int nNextSlash = sFile.find( '\\' );
		if ( nNextSlash < 0 )
			return pFileName;		// could not resolve the file, return the original file name..

		sFile.right( sFile.length() - (nNextSlash + 1) );
	}

	return sFile;
}

CharString CResourcerDoc::resolve( const char * pFileName ) const
{
	CharString sRelative = relative( pFileName );
	if ( sRelative.length() > 0 )
		return m_RootDirectory + sRelative;

	// failed to resolve the file
	return "";
}


//----------------------------------------------------------------------------

void CResourcerDoc::setCurrentDirectory( const char * path )
{
	m_CurrentDirectory = path;
}

void CResourcerDoc::pumpMessages()
{
    // pump messages
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
		TranslateMessage(&msg);
		DispatchMessage(&msg);  
    }
}

void CResourcerDoc::beginBuild()
{
	if (! m_bBulding )
	{
		m_nBuildStart = time( NULL );
		pushMessage( CharString().format("Begin Build, %s", CTime( m_nBuildStart ).Format( "%a %b %d, %Y  %H:%M:%S" )) );

		m_bBulding = true;
		m_Built.release();
		m_Keys.clear();
	}
}

void CResourcerDoc::endBuild()
{
	if ( m_bBulding )
	{
		m_bBulding = false;

		time_t endtime = time( NULL );
		pushMessage( CharString().format("End Build, %u seconds...", endtime - m_nBuildStart) );
	}
}

bool CResourcerDoc::updateBroker( const char * path, bool bRecurse )
{
	Path SearchPath( path );

	// recurse up the directory heirarchy
	if ( bRecurse )
	{
		FindFile ff( SearchPath.directory() + "*.*" );
		for(int i=0;i<ff.directoryCount();i++)
		{
			CharString sDirectory = ff.directory( i );
			if ( updateBroker( SearchPath.directory() + sDirectory + "\\", true ) )
				return true;
		}
	}

	FindFile ff( SearchPath.path() + "*.PRT" );
	for(int i=0;i<ff.fileCount();i++)
	{
		if ( updateBroker( SearchPath.directory() + ff.file( i ) ) )
			return true;
	}

	return false;
}

bool CResourcerDoc::updateBroker( const char * pFileName )
{
	CharString sFileName( resolve( pFileName ) );
	// handle empty port link
	if ( sFileName.length() == 0 )
		return false;

	// generate the key for this file
	WidgetKey nKey( portKey( sFileName ) );
	// check if this branch has already been built
	if ( m_Built.find( nKey ).valid() )
		return false;
	// mark this branch as checked
	m_Built[ nKey ] = 1;

	//pushMessage( CharString().format( "Updating %s...", sFileName ) );
	setStatus( CharString().format( "Updating %s...", sFileName ) );
	pumpMessages();

	// get the file time of the port file... 
	dword version = FileDisk::fileDate( sFileName );
	if ( version == 0 )
	{
		pushMessage( CharString().format("%s ; file missing!", sFileName ) );
		return true;			// return if file doesn't exist
	}

	// check the version of the resource already stored in the broker (if any) 
	if ( m_Broker.version( nKey ) >= version )
	{
		// version in broker is the same... so just enum dependencies..
		if (! m_DependentTree.find( nKey ).valid() )
		{
			// load the port from disk
			Port::Ref pPort = Port::portLoad( sFileName );
			if ( pPort.valid() )
			{
				// get the port dependents
				DependentArray dependents;
				pPort->dependencies( dependents );

				// save the dependents in this document
				m_DependentTree[ nKey ] = dependents;
			}
		}

		// make sure the dependents are up to date
		DependentArray dependents = m_DependentTree[ nKey ];
		for(int i=0;i<dependents.size();i++)
		{
			if ( updateBroker( dependents[i] ) )
				pushMessage( CharString().format("%s ; Failed to build linked resource!", sFileName) );
		}

		// resource is up to date, just return
		return false;
	}

	// load the port from disk
	Port::Ref pPort = Port::portLoad( sFileName );
	if ( ! pPort.valid() )
	{
		pushMessage( CharString().format("%s ; Failed to load port!", sFileName ) );
		return true;
	}

	if (! checkKeyUnique( nKey ) )
	{
		pushMessage( CharString().format("%s ; Key conflict detected for port.", sFileName ) );
		return false;
	}

	// get the port dependents
	DependentArray dependents;
	pPort->dependencies( dependents );

	// save the dependents in this document
	m_DependentTree[ nKey ] = dependents;

	// make sure the dependents are up to date
	for(int i=0;i<dependents.size();i++)
		if ( updateBroker( dependents[i] ) )
			pushMessage( CharString().format("%s ; Failed to build linked resource!", sFileName) );

	// create the resource
	Resource::Ref pResource = pPort->createResource();
	if (! pResource )
	{
		pushMessage( CharString().format("%s ; Port returned NULL resource!", sFileName) );
		return true;
	}

	// set the resource key
	pResource->setKey( nKey );
	// store the resource
	m_Broker.store( pResource, version, pPort->autoLoad() );

	return false;
}

Resource::Link CResourcerDoc::getResource( const char * pFileName )
{
	// resolve filename into full path to port...
	CharString sFileName( resolve( pFileName ) );
	// update the resource
	updateBroker( sFileName );

	// return a link with the correct key, this doesn't load the object from the broker...
	Resource::Link pResource( portKey( sFileName ) );
	pResource.valid();

	return pResource;
}

void CResourcerDoc::upgradePorts( const char * pMask, bool bRecurse )
{
	Path SearchPath( pMask );

	FindFile ff( SearchPath.path() );
	for(int i=0;i<ff.fileCount();i++)
	{
		bool bUpgraded = false;

		CharString sPortFile = SearchPath.directory() + ff.file( i );

		// get the original date/time of the file
		dword fileTime = FileDisk::fileDate( sPortFile );
		if ( fileTime != 0 )
		{
			Port::Ref port = Port::portLoad( sPortFile, true );
			if ( port.valid() )
			{
				// save the port back to disk
				port->SetModifiedFlag();
				port->OnFileSave();

				// release the port
				port = NULL;

				if ( FileDisk::touchFile( sPortFile, fileTime ) )
					bUpgraded = true;
			}
		}

		pushMessage( CharString().format("%s ; %s", sPortFile.cstr(), bUpgraded ? "Upgraded" : "Failed to Upgrade" ) );
		pumpMessages();
	}

	// recurse up the directory heirarchy
	if ( bRecurse )
	{
		FindFile ff( SearchPath.directory() + "*.*" );
		for(int i=0;i<ff.directoryCount();i++)
		{
			CharString sDirectory = ff.directory( i );
			if ( sDirectory[0] == '.' )
				continue;
			upgradePorts( SearchPath.directory() + sDirectory + "\\" + SearchPath.file(), true );
		}
	}
}

void CResourcerDoc::touchPorts( const char * pMask, bool bRecurse )
{
	Path SearchPath( pMask );

	FindFile ff( SearchPath.path() );
	for(int i=0;i<ff.fileCount();i++)
	{
		bool bTouched = false;

		CharString sPortFile = SearchPath.directory() + ff.file( i );
		bTouched = FileDisk::touchFile( sPortFile );

		pushMessage( CharString().format("%s ; %s", sPortFile.cstr(), bTouched ? "Touched" : "Failed to Touch" ) );
		pumpMessages();
	}

	// recurse up the directory heirarchy
	if ( bRecurse )
	{
		FindFile ff( SearchPath.directory() + "*.*" );
		for(int i=0;i<ff.directoryCount();i++)
		{
			CharString sDirectory = ff.directory( i );
			if ( sDirectory[0] == '.' )
				continue;
			touchPorts( SearchPath.directory() + sDirectory + "\\" + SearchPath.file(), true );
		}
	}
}

void CResourcerDoc::pushMessage( const char * pMessage )
{
	LOG_STATUS( "Resourcer", pMessage );

	m_MessageLock.lock();
	m_Messages.push( pMessage );
	m_MessageLock.unlock();
}

void CResourcerDoc::flushMessages()
{
	m_MessageLock.lock();
	m_Messages.release();
	m_MessageLock.unlock();
}

void CResourcerDoc::setStatus( const char * pStatus )
{
	m_MessageLock.lock();
	m_sStatus = pStatus;
	m_MessageLock.unlock();
}

WidgetKey CResourcerDoc::portKey( const char * pFileName )
{
	// generate the key from the filaname relative to the root directory
	CharString sFileName = relative( pFileName );
	if ( sFileName.length() == 0 )
		return NULL_WIDGET;

	WidgetKey nKey = StringHash64( sFileName /*, WIDGET_SEED*/ ).hash();
	m_ReverseHash[ nKey ] = pFileName;

	return nKey;
}

//----------------------------------------------------------------------------

CharString CResourcerDoc::portFile( const WidgetKey & nKey )
{
	ReverseHash::iterator iFind = m_ReverseHash.find( nKey );
	if ( iFind != m_ReverseHash.end() )
		return iFind->second;

	// port not found
	return CharString("");
}

bool CResourcerDoc::checkKeyUnique( const WidgetKey & a_Key )
{
	if (! m_bBulding )
		return true;		// we only check keys when building..

	if ( m_Keys.find( a_Key ) != m_Keys.end() )
		return false;

	m_Keys.insert( a_Key );
	return true;
}

void CResourcerDoc::onPortSaved( const WidgetKey & a_Key )
{
	m_Built.remove( a_Key );
	m_Keys.erase( a_Key );			// prevent checkKeyUnique() from returning a false because we re-save the port..
}

//-------------------------------------------------------------------------------

void CResourcerDoc::OnDocumentOptions() 
{
	DocumentOptions(this).DoModal();
}

void CResourcerDoc::OnDocumentBuild() 
{
	time_t startTime = time( NULL );
	pushMessage( CharString().format("Begin Build, %s", CTime( startTime ).Format( "%a %b %d, %Y  %H:%M:%S" )) );

	updateBroker( rootDirectory(), true );

	time_t endtime = time( NULL );
	pushMessage( CharString().format("End Build, %u seconds...", endtime - startTime) );
}

void CResourcerDoc::OnDocumentClean() 
{
	// close the broker
	m_Broker.close();
	// delete the broker files
	WIN32_FIND_DATA	ff;

	// delete all the files and directories in this directory
	HANDLE ffh = FindFirstFile( m_BrokerFolder + "*.*" , &ff);
	if (ffh == INVALID_HANDLE_VALUE)
		return;

	BOOL found = true;
	while(found)
	{
		if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
		{
			if ( ! DeleteFile( m_BrokerFolder + ff.cFileName) )
			{
				MessageBox( NULL, m_BrokerFolder + ff.cFileName, "Failed to delete file", MB_OK );
				return;
			}
		}

		found = FindNextFile(ffh,&ff);
	}
	FindClose(ffh);
	
	// re-create the broker file
	m_Broker.open( m_BrokerFolder, true, false );
}

void CResourcerDoc::OnDocumentClearoutput() 
{
	flushMessages();
}


//----------------------------------------------------------------------------

void CResourcerDoc::addDocTemplate()
{
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	pApp->AddDocTemplate( new CMultiDocTemplate(
		IDR_RSRTYPE,
		RUNTIME_CLASS(CResourcerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		NULL /*RUNTIME_CLASS(CView)*/) );
}

CResourcerDoc * CResourcerDoc::document()
{
	return s_pDocument;
}

//----------------------------------------------------------------------------

void CResourcerDoc::OnDocumentSyncronize() 
{
	if ( m_nSyncronizeJob != 0 && m_pMirrorClient->checkJob( m_nSyncronizeJob ) == 0 )
		return;		// still syncronizing...

	CWaitCursor wait;

	pushMessage( CharString().format("Begin Syncronize, %s", CTime( CTime::GetCurrentTime() ).Format( "%a %b %d, %Y  %H:%M:%S" )) );
	if ( !m_pMirrorClient->connected() && !m_pMirrorClient->open( m_MirrorServer, m_MirrorServerPort, m_RootDirectory, NULL, false ) )
		MessageBox(NULL,TEXT("Failed to connect to mirror server!"),TEXT("Error"), MB_OK );
	if ( !m_pMirrorClient->loggedIn() && !m_pMirrorClient->login( m_UID, m_PW ) )
		MessageBox(NULL,TEXT("Failed to login to mirror server!"),TEXT("Error"), MB_OK );
	
	if ( m_pMirrorClient->connected() )
		m_nSyncronizeJob = m_pMirrorClient->syncronize( true );
}

//----------------------------------------------------------------------------
// EOF

