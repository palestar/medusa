/*
	Port.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ChildFrm.h"
#include "ResourcerDoc.h"
#include "Port.h"
#include "PropertyEditorView.h"
#include "SelectPortType.h"
#include "SelectImportPort.h"
#include "ChoosePorts.h"

#include "Resource.h"

#include "File/FileDisk.h"
#include "File/FileMemory.h"
#include "Standard/StringHash64.h"
#include "File/Path.h"

//---------------------------------------------------------------------------------------------------

static bool IsTextPort( InStream & input )
{
	input.file()->rewind();

	// check for the PRT\n signature as the first line, if found then set the FF_TEXT filter..
	char sSig[ 4 ];
	input.read( sSig, 4 );
	if ( sSig[0] == 'P' && sSig[1] == 'R' && sSig[2] == 'T' && sSig[3] == '\n' )
	{
		input.setFilter( FF_TEXT );
		return true;
	}

	// pre-signature port, so rewind and leave as binary mode..
	input.file()->rewind();		
	input.setFilter( 0 );

	return false;
}

//----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(Port, CDocument)
	//{{AFX_MSG_MAP(Port)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL Port::OnNewDocument()
{
	// this shouldn't be called, because the PortTemplate should handle creation of a new document
	ASSERT( 0 );
	return FALSE;
}

BOOL Port::OnOpenDocument(const char * lpszPathName) 
{
	// this shouldn't be called, because the PortTemplate should handle opening documents
	ASSERT( 0 );
	return FALSE;
}

void Port::OnFileSave() 
{
	UpdateAllViews( NULL, true, this );
	//if ( ! IsModified() )
	//	return;

	CString fileName( GetPathName() );
	if ( fileName.IsEmpty() )
	{
		OnFileSaveAs();
		
		fileName = GetPathName();
		if ( fileName.IsEmpty() )
			return;
	}

	updateKey();

	if ( m_pFileLock != NULL )
	{
		// save at the beginning of the file
		m_pFileLock->setPosition( 0 );

		// stream out this object
		OutStream output( m_pFileLock, FF_TEXT );
		output.writeLine( "PRT" );
		output << this;

		// trim the file
		m_pFileLock->setEOF();
	
		// invalidate build information
		CResourcerDoc::document()->onPortSaved( m_Key );
	}
	else
		MessageBox(NULL,TEXT("Cannot save this resource port; it isn't locked"), GetTitle(), MB_OK );

	SetModifiedFlag( false );

	return;
}

void Port::OnFileSaveAs() 
{
	bool fileNameValid = false;

	CResourcerDoc * pDoc = CResourcerDoc::document();
	CString currentDirectory( pDoc != NULL ? pDoc->currentDirectory() : TEXT("") );

	CFileDialog fileDialog( FALSE, TEXT("PRT"), currentDirectory + GetTitle(), 
		OFN_EXPLORER|OFN_OVERWRITEPROMPT,TEXT("Port Files (*.prt)|*.prt||") );
	while( !fileNameValid )
	{
		if ( fileDialog.DoModal() != IDOK )
			return;

		try {
			File *pFile = new FileDisk( CharString(fileDialog.GetPathName()), 
				FileDisk::READ_WRITE );

			fileNameValid = true;
			setFileLock( pFile );	
		}
		catch(...)
		{
			MessageBox(NULL,TEXT("Failed to open file; please try a different filename"), fileDialog.GetPathName(), MB_OK );
		}
	}

	SetPathName( fileDialog.GetPathName(), false );
	SetTitle( fileDialog.GetFileTitle() );
	SetModifiedFlag();

	OnFileSave();
}

BOOL Port::CanCloseFrame(CFrameWnd* pFrame) 
{
	//UpdateAllViews( NULL, true, this );

	// check for any other view of this document, if so don't prompt to save the object
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT( pView );

		CFrameWnd* pFrame = pView->GetParentFrame();
		
		// assume frameless views are ok to close
		if (pFrame != NULL)
		{
			// assumes 1 document per frame
			ASSERT( pFrame );
			if (pFrame->m_nWindow > 0)
				return TRUE;        // more than one frame refering to us
		}
	}

	if ( (locked() && IsModified()) || GetPathName().IsEmpty() )
	{
		if ( MessageBox(NULL,TEXT("Port has been modified; do you wish to save before closing?"),GetTitle(), MB_YESNO) == IDYES )
			OnFileSave();
	}

	return TRUE;
}

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( Port, Widget );

BEGIN_ABSTRACT_PROPERTY_LIST( Port, Widget );
END_PROPERTY_LIST();

Port::Port() : m_bViewActive( false )
{
	// we will handle deletion in OnChangedViewList()...
	m_bAutoDelete = false;
}

Port::~Port()
{
	OnCloseDocument();
}

//----------------------------------------------------------------------------

void Port::OnChangedViewList()
{
	CDocument::OnChangedViewList();

	//TRACE( CharString().format( "OnChangedViewList() - (%s) reference count = %d", GetPathName(), referenceCount() ) );

	if ( !m_bViewActive && !m_viewList.IsEmpty() )
	{
		//TRACE( "Grabbing reference" );
		m_bViewActive = true;
		grabReference();
	}
	else if ( m_bViewActive && m_viewList.IsEmpty() )
	{
		//TRACE( "Releasing reference" );
		m_bViewActive = false;
		releaseReference();
	}
}


//----------------------------------------------------------------------------

const int VERSION_022599 = 1;
const int VERSION_090299 = 90299;

bool Port::read( const InStream & input )
{
	if (! Widget::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_090299:
			break;
		case VERSION_022599:
			input >> version;			// note, this version is different from the one above which is a file version
			break;
		}

		// return false, so upstream serialization code will fallback to old format..
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

bool Port::autoLoad()
{
	return( false );
}

void Port::dependencies( DependentArray & dep )
{}

CFrameWnd * Port::createView()
{
	// create a default PropertyEditorView for any port...
	static CMultiDocTemplate docTemplate( IDR_PORT,
							NULL,
							RUNTIME_CLASS(CMDIChildWnd),
							RUNTIME_CLASS(CPropertyEditorView) );

    CMDIChildWnd * pFrame = dynamic_cast<CMDIChildWnd *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

//----------------------------------------------------------------------------

bool Port::locked() const
{
	return m_pFileLock.valid();
}

void Port::setFileLock( File * pFile )
{
	m_pFileLock = pFile;
}

void Port::updateKey()
{
	if ( CResourcerDoc::document() != NULL )
		setKey( CResourcerDoc::document()->portKey( GetPathName() ) );
}

void Port::report( const char * pMessage )
{
	pushMessage( CharString().format(TEXT("%s ; %s"), GetPathName(), pMessage) );
}

//----------------------------------------------------------------------------

Array< ClassKey >					Port::s_Ports;
Hash< ClassKey, HICON >				Port::s_PortIcon;
Tree< CString, Array< ClassKey> >	Port::s_PortImportExt;
Array<CString>						Port::s_PortLibPaths;					// the array of all the directories containing DLL's
Array< Library * >					Port::s_PortLibs;						// array of loaded DLL's

const TCHAR * SECTION = TEXT("Settings");

//----------------------------------------------------------------------------

CResourcerDoc * Port::document()
{
	return CResourcerDoc::document();
}

CharString Port::relative( const char * pFile )
{
	CResourcerDoc * pDoc = document();
	if ( pDoc != NULL )
		return pDoc->relative( pFile );
	else
		return "";
}

void Port::loadPortLibs()
{
	ASSERT( s_PortLibPaths.size() == 0 );

	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	// grab path information from the registry
	int libPathCount = pApp->GetProfileInt( SECTION, TEXT("LibPathCount"), 0 );
	if ( libPathCount > 0 && GetAsyncKeyState( VK_SHIFT) >= 0 )
	{
		CString keyString;
		for(int i=0;i<libPathCount;i++)
		{
			keyString.Format( TEXT("LibPath%d"),i);
			s_PortLibPaths.push( pApp->GetProfileString( SECTION, keyString, TEXT("") ) );
		}
	}
	else
		CChoosePorts().DoModal();

	// load the dll's
	for(int i=0;i<s_PortLibPaths.size();i++)
	{
		Library * pLib = new Library();
		if (! pLib->load( s_PortLibPaths[i] ) )
		{
			MessageBox( NULL, "Failed to load library!", s_PortLibPaths[i], MB_OK );
			delete pLib;

			continue;
		}

		s_PortLibs.push( pLib );
	}
}

void Port::unloadPortLibs()
{
	CString keyString;

	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	pApp->WriteProfileInt( SECTION, TEXT("LibPathCount"), s_PortLibPaths.size() );
	for(int i=0;i<s_PortLibPaths.size();i++)
	{
		keyString.Format(TEXT("LibPath%d"),i);
		pApp->WriteProfileString( SECTION, keyString, s_PortLibPaths[i] );
	}

	s_PortLibPaths.release();

	for(int i=0;i<s_PortLibs.size();i++)
		delete s_PortLibs[i];
	s_PortLibs.release();
}

void Port::addDocTemplate()
{
	CWinApp * pApp = AfxGetApp();
	ASSERT( pApp );

	pApp->AddDocTemplate( new PortTemplate(
		IDR_PRTTYPE,
		NULL,
		NULL,
		NULL) );
}

void Port::registerPort( const ClassKey &key, HICON icon )
{
	s_Ports.push( key );
	s_PortIcon[ key ] = icon;
}

void Port::registerImportExt( const ClassKey &key, const char * extension )
{
	CString upperExtension( extension );
	upperExtension.MakeUpper();

	s_PortImportExt[ upperExtension ].push( key );
}

void Port::unregisterPort( const ClassKey &key )
{
	s_Ports.removeSearch( key );
	s_PortIcon.remove( key );
}

void Port::unregisterImportExt( const char * extension )
{
	CString upperExtension( extension );
	upperExtension.MakeUpper();

	s_PortImportExt.remove( upperExtension );
}

int Port::portCount()
{
	return( s_Ports.size() );
}

ClassKey Port::portKey( int n )
{
	return( s_Ports[ n ] );
}

Factory * Port::portFactory( int n )
{
	return( Factory::findFactory( portKey( n ) ) );
}

CString Port::portName( int n )
{
	return( CString( (const char *)portFactory( n )->className() ) );
}

HICON Port::portIcon( int n )
{
	return( s_PortIcon[ portKey( n ) ] );
}

int Port::portIndex( Port * pPort )
{
	return( s_Ports.search( pPort->factory()->classKey() ) );
}

int	Port::portIndex( const ClassKey & key )
{
	return( s_Ports.search( key ) );
}

int Port::findImportExt( const char * extension )
{
	static CString	lastExtension;
	static int		lastPort = -1;

	if ( lastExtension == extension )
		return( lastPort );

	CString upperExtension( extension );
	upperExtension.MakeUpper();

	Array< ClassKey > & ports = s_PortImportExt[ upperExtension ];
	if ( ports.size() < 1 )
		return -1;

	// only display the dialog if we have more than one type of port that
	// supports importing this extension
	if ( ports.size() > 1 )
	{
		CSelectImportPort selectDialog( upperExtension );
		if ( selectDialog.DoModal() == IDOK )
		{
			lastExtension = extension;
			lastPort = selectDialog.m_SelectedPort;

			return( lastPort );
		}

		return -1;
	}

	// if only one port supports this extension, then use it..
	return( s_Ports.search( ports[ 0 ] ) );
}

//----------------------------------------------------------------------------


bool Port::portInfo( const char * pFileName, PortInfo & info )
{
	CResourcerDoc * pDoc = CResourcerDoc::document();
	if ( pDoc == NULL )
		return NULL;		// no document
	CharString sFileName = CResourcerDoc::document()->resolve( pFileName );
	if ( GetFileAttributes( sFileName ) == 0xffffffff )
		return false;

	// open the file to the object
	FileDisk::Ref pFile = new FileDisk();
	if ( pFile->open( sFileName, FileDisk::READ_WRITE ) )
		info.locked = false;
	else if ( pFile->open( sFileName, FileDisk::READ ) )
		info.locked = true;
	else
		return false;
	if ( pFile->size() >= sizeof( info.classKey ) )
	{
		InStream input( pFile );
		IsTextPort( input );

		input >> info.classKey;	
	}
	pFile->close();

	info.type = s_Ports.search( info.classKey );
	if ( info.type < 0 )
		return false;
	if ( CResourcerDoc::document() != NULL )
		info.widgetKey = pDoc->portKey( sFileName ); 
	else
		info.widgetKey = NULL_WIDGET;

	info.icon = portIcon( info.type );
	info.version = getFileTime( sFileName );

	return true;
}


Port::Ref Port::portLoad( const char * pFileName, bool bLock /*= false*/ )
{
	CResourcerDoc * pDoc = CResourcerDoc::document();
	if ( pDoc == NULL )
		return NULL;		// no document
	CharString sFileName = pDoc->resolve( pFileName );
	if ( sFileName.length() == 0 )
		return NULL;		// failed to resolve filename

	Port::Ref pPort;

	// open the file to the object
	File::Ref pFile = NULL;
	try {
		pFile = new FileDisk( sFileName, 
			bLock ? FileDisk::READ_WRITE : FileDisk::READ );

		// serialize in the object
		InStream input( pFile );		
		IsTextPort( input );

		input >> pPort;
	
		if ( pPort )
		{
			pPort->SetPathName( (CString)sFileName, false );
			pPort->updateKey();		// Update the key only after the Path has been set, the key is generated from the filename of the port...

			pPort->SetTitle( (CString)Path( sFileName ).name() );
			pPort->SetModifiedFlag( false );

			if ( pFile->isWritable() )
				pPort->setFileLock( pFile );
		}
	}
	catch(...)
	{
		return NULL;
	}

	return pPort;
}

Resource::Link Port::portResource( const char * pFileName )
{
	if ( CResourcerDoc::document() != NULL && pFileName[0] != 0 )
		return CResourcerDoc::document()->getResource( pFileName );

	return Resource::Link();
}

void Port::portView( const char * pFileName )
{
	// use the document template to open the port file
	AfxGetApp()->OpenDocumentFile( pFileName );
}

//----------------------------------------------------------------------------

PortTemplate::~PortTemplate()
{
	// This destructor should only be called on program exit..
	//ASSERT( 0 );
}

CDocument* PortTemplate::CreateNewDocument()
{
	CSelectPortType dialog;
	if ( dialog.DoModal() == IDOK && dialog.m_SelectedPort >= 0 )
	{
		Factory *pFactory = Port::portFactory( dialog.m_SelectedPort );

		Port * pDoc = dynamic_cast<Port *>( pFactory->createWidget() );
		ASSERT( pDoc );

		pDoc->SetModifiedFlag();
		pDoc->SetTitle( CString("New ") + pFactory->className() );
		
		AddDocument( pDoc );
		return( pDoc );
	}

	return( NULL );
}

CDocument* PortTemplate::OpenDocumentFile(LPCSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible) {
	return OpenDocumentFile(lpszPathName, bMakeVisible);
}

CDocument* PortTemplate::OpenDocumentFile( LPCTSTR lpszPathName, BOOL bMakeVisible /*= TRUE*/ )
{
	Port::Ref pDocument;
	if ( lpszPathName != NULL )
	{
		// always attempt to lock the port
		if (! (pDocument = Port::portLoad( lpszPathName, true )) )
		{
			if (! (pDocument = Port::portLoad( lpszPathName, false )) )
			{
				MessageBox(NULL,TEXT("Failed to load port!"), lpszPathName, MB_OK );
				return( NULL );
			}
			else
				MessageBox(NULL,TEXT("Failed to lock port; opened read-only"), lpszPathName, MB_OK);
		}
		if ( pDocument && bMakeVisible )
			CreateNewFrame( pDocument, NULL );
		AddDocument( pDocument );
	}
	else
	{
		pDocument = dynamic_cast<Port *>( CreateNewDocument() );
		if ( pDocument && bMakeVisible )
			CreateNewFrame( pDocument, NULL );
	}

	return( pDocument );
}

CFrameWnd* PortTemplate::CreateNewFrame( CDocument* pDoc, CFrameWnd* pOther )
{
	Port * pPort = dynamic_cast<Port *>( pDoc );
	ASSERT( pPort );

	return pPort->createView();
}

//----------------------------------------------------------------------------

dword Port::getFileTime( const char * file )
{
	return( FileDisk::fileDate( file ) );
}

void Port::pushMessage( const char * pMessage )
{
	if ( CResourcerDoc::document() != NULL )
		CResourcerDoc::document()->pushMessage( pMessage );
}

//----------------------------------------------------------------------------
//EOF
