/*
	BrokerArchive.cpp

	This concrete broker uses a Archive object for storing / loading widgets
	(c)2005 Palestar, Richard Lyle
*/

#define FACTORY_DLL

#include "File/FileArchive.h"
#include "BrokerArchive.h"
#include "Widget.h"
#include "Factory.h"

//----------------------------------------------------------------------------

const dword FLAG_NONE = 0x0;
const dword FLAG_AUTOLOAD = 0x1;

//----------------------------------------------------------------------------

BrokerArchive::BrokerArchive()
{}

BrokerArchive::BrokerArchive( const char * archiveFile, bool write /*= true*/, bool autoLoad /*= true*/ )
{
	if (! open( archiveFile, write, autoLoad ) )
		throw Failure();
}

BrokerArchive::~BrokerArchive()
{
	close();
}

//----------------------------------------------------------------------------

dword BrokerArchive::version( const WidgetKey & key )
{
	int fileIndex = m_Archive.file( key );
	if ( fileIndex >= 0 )
		return( m_Archive.fileVersion( fileIndex ) );

	return( 0 );
}

dword BrokerArchive::size( const WidgetKey & key )
{
	int fileIndex = m_Archive.file( key );
	if ( fileIndex >= 0 )
		return( m_Archive.fileSize( fileIndex ) );

	return( 0 );
}

Widget * BrokerArchive::load( const WidgetKey & key )
{
	Widget * pWidget = NULL;
	try {
		InStream input( new FileArchive( &m_Archive, key ) );
		input >> pWidget;
	}
	catch( File::FileError )
	{}
	
	return pWidget;
}

bool BrokerArchive::store( Widget * pWidget, dword version, bool autoLoad )
{
	bool bStored = false;

	try {
		FileArchive::Ref pFile = new FileArchive( &m_Archive, pWidget->key() );

		// set the widget version
		pFile->setVersion( version );
		// set the autoload flag
		if ( autoLoad )
			pFile->setFlags( FLAG_AUTOLOAD );

		// write the widget
		OutStream output( pFile );
		output << pWidget;
		output.file()->setEOF();

		bStored = true;
	}
	catch( File::FileError )
	{}

	// register the stored widget
	if ( bStored )
		registerWidget( this, pWidget->key(), version, false, true );

	return bStored;
}

bool BrokerArchive::open( const char * archiveFile, bool write /*= true*/, bool autoLoad /*= true*/ )
{
	if (! m_Archive.open( archiveFile, write ? Archive::READ_WRITE : Archive::READ ) )
		return false;

	// register all the widgets contained in the archive
	for(int i=0;i<m_Archive.fileCount();i++)
	{
		registerWidget( this, m_Archive.fileKey( i ), m_Archive.fileVersion( i ),
			(m_Archive.fileFlags( i ) & FLAG_AUTOLOAD) && autoLoad, true );
	}

	// load all autoload widgets
	autoLoadWidgets();

	return true;
}

void BrokerArchive::close()
{
	// unregister all the widgets
	for(int i=0;i<m_Archive.fileCount();i++)
		unregisterWidget( this, m_Archive.fileKey( i ), m_Archive.fileVersion( i ) );

	m_Archive.close();
}

Archive & BrokerArchive::archive()
{
	return( m_Archive );
}

//----------------------------------------------------------------------------
// EOF
