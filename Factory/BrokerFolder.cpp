/*
	BrokerFolder.cpp

	This concrete broker uses a Archive object for storing / loading widgets
	(c)2005 Palestar, Richard Lyle
*/

#define FACTORY_DLL
#include "Debug/Assert.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "BrokerFolder.h"
#include "FactoryTypes.h"

//----------------------------------------------------------------------------

BrokerFolder::BrokerFolder()
{}

BrokerFolder::BrokerFolder( const char * brokerFolder, bool write /*= true*/, bool autoLoad /*= true*/ )
{
	if (! open( brokerFolder, write, autoLoad ) )
		throw Failure();
}

BrokerFolder::~BrokerFolder()
{
	close();
}

//----------------------------------------------------------------------------

dword BrokerFolder::version( const WidgetKey & key ) 
{
	CharString widgetFile( m_BrokerFolder + key.string() + ".wob" );
	if ( ! FileDisk::fileExists( widgetFile ) )
	{
		widgetFile = m_BrokerFolder + key.string() + ".aob";
		if ( ! FileDisk::fileExists( widgetFile ) )
			return 0;
	}

	return( FileDisk::fileDate( widgetFile ) );
}

dword BrokerFolder::size( const WidgetKey & key )
{
	CharString widgetFile( m_BrokerFolder + key.string() + ".wob" );
	if ( ! FileDisk::fileExists( widgetFile ) )
	{
		widgetFile = m_BrokerFolder + key.string() + ".aob";
		if ( ! FileDisk::fileExists( widgetFile ) )
			return 0;
	}
	
	return( FileDisk::fileSize( widgetFile ) );
}

Widget * BrokerFolder::load( const WidgetKey & key )
{
	CharString sWidgetFile( m_BrokerFolder + key.string() + ".wob" );
	
	if ( ! FileDisk::fileExists( sWidgetFile ) )
	{
		sWidgetFile = m_BrokerFolder + key.string() + ".aob";
		if ( ! FileDisk::fileExists( sWidgetFile ) )
			return NULL;
	}

	Widget * pWidget = NULL;
	try {
		InStream input( new FileDisk(sWidgetFile, FileDisk::READ) );
		
		int nVersion = 0;
		input >> nVersion;

		// validate the version number...
		switch( nVersion )
		{
		case SV_20090104:
			input.setVersion( nVersion );
			break;
		default:
			// WOB that is prior to the stream versions...
			input.setVersion( 0 );
			input.setPosition( 0 );
			break;
		}

		input >> pWidget;
	}
	catch( FileDisk::FileError )
	{
		pWidget = NULL;
	}

	return pWidget;
}

bool BrokerFolder::store( Widget * pWidget, dword version, bool autoLoad )
{
	if (! m_Write )
		return false;		// cannot write using this broker

	// generate the widget filename
	CharString sFileName = m_BrokerFolder + pWidget->key().string() + (autoLoad ? ".aob" : ".wob");

	bool bStored = false;
	try {
		int nVersion = SV_CURRENT;

		// write the widget to a file
		OutStream output( new FileDisk( sFileName, FileDisk::WRITE), 0, nVersion );
		output << nVersion;
		output << pWidget;

		bStored = true;
	}
	catch( File::FileError )
	{}

	// update the version
	if ( bStored && version != 0 )
		FileDisk::touchFile( sFileName, version );
	// register the widget
	if ( bStored )
		registerWidget( this, pWidget->key(), version, false, true );		// autoload is always false, we don't want to reload a widget we're storing

	return bStored;
}

bool BrokerFolder::open( const char * brokerFolder, bool bWrite /*= true*/, bool autoLoad /*= true*/ )
{
	close();

	m_BrokerFolder = brokerFolder;
	m_Write = bWrite;

	// find all widgets
	m_Widgets.findFiles( m_BrokerFolder + "*.wob" );
	m_AutoLoadWidgets.findFiles( m_BrokerFolder + "*.aob" );

	// check for any wob or aob files, if none were found return error
	if ( !bWrite && m_Widgets.fileCount() == 0 && m_AutoLoadWidgets.fileCount() == 0 )
		return false;

	// register normal widgets
	int i = 0;
	for(;i<m_Widgets.fileCount();i++)
	{
		CharString	widgetFile( m_Widgets.file( i ) );
		WidgetKey	widgetKey( (const char *)widgetFile );
		dword		widgetVersion( FileDisk::fileDate( m_BrokerFolder + widgetFile ) );

		registerWidget( this, widgetKey, widgetVersion, false, true );
	}

	// reigster autoload widgets
	for(i=0;i<m_AutoLoadWidgets.fileCount();i++)
	{
		CharString	widgetFile( m_AutoLoadWidgets.file( i ) );
		WidgetKey	widgetKey( (const char *)widgetFile );
		dword		widgetVersion( FileDisk::fileDate( m_BrokerFolder + widgetFile ) );

		registerWidget( this, widgetKey, widgetVersion, autoLoad ? true : false, true );
	}

	// load all autoload widgets
	autoLoadWidgets();

	return true;
}

void BrokerFolder::close()
{
	// unregister normal widgets
	int i = 0;
	for(;i<m_Widgets.fileCount();i++)
	{
		CharString	widgetFile( m_Widgets.file( i ) );
		WidgetKey	widgetKey( (const char *)widgetFile );
		dword		widgetVersion( FileDisk::fileDate( m_BrokerFolder + widgetFile ) );

		unregisterWidget( this, widgetKey, widgetVersion );
	}
	// unregister auto-load widgets
	for(i=0;i<m_AutoLoadWidgets.fileCount();i++)
	{
		CharString	widgetFile( m_AutoLoadWidgets.file( i ) );
		WidgetKey	widgetKey( (const char *)widgetFile );
		dword		widgetVersion( FileDisk::fileDate( m_BrokerFolder + widgetFile ) );

		unregisterWidget( this, widgetKey, widgetVersion );
	}

	m_Widgets.release();
	m_AutoLoadWidgets.release();
}

//----------------------------------------------------------------------------
// EOF
