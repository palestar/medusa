/*
	BrokerClient.cpp

	This object is used to connect to a broker server
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Standard/StringHash64.h"
#include "File/FileArchive.h"
#include "File/Stream.h"
#include "FileSocket.h"
#include "BrokerClient.h"

//----------------------------------------------------------------------------

BrokerClient::BrokerClient()
{}

BrokerClient::BrokerClient( Context & context )
{
	if (! open( context ) )
		throw Failure();
}

BrokerClient::~BrokerClient()
{
	close();
}

//-------------------------------------------------------------------------------

Widget * BrokerClient::load( const WidgetKey & key )
{
	AutoLock lock( &m_Lock );

	// check the local archive, if stored there then load it
	int fileIndex = m_Archive.file( key );
	if ( fileIndex >= 0 )
	{
		Widget * pWidget = NULL;

		InStream input( new FileArchive( &m_Archive, key ) );
		input >> pWidget;

		return pWidget;
	}

	// file is stored remotely, send request to server to send the widget
	try {
		OutStream output( send( SERVER_REQUEST_WIDGET ) );
		output << key;
	}
	catch( FileSocket::FileError )
	{
		return NULL;
	}

	// unlock the client
	lock.release();
	// wait until a widget has been received
	m_Received.wait();

	// call recursively
	return load( key );
}

bool BrokerClient::store( Widget * pWidget, dword version, bool autoLoad )
{
	// cannot store widgets to a remote server, YET!
	ASSERT( 0 );
	return false;
}

//-------------------------------------------------------------------------------

bool BrokerClient::open( Context & context )
{
	// open the local archive
	if (! m_Archive.open( context.localArchive, Archive::READ_WRITE ) )
		return false;
	// connect to the BrokerServer
	if (! Client::open( context.socket, context.serverAddress, context.serverPort ) )
		return false;

	return true;
}

void BrokerClient::close()
{
	// close the connection
	Client::close();
	// close the archive
	m_Archive.close();
}

//-------------------------------------------------------------------------------

/*
	This uses pretty standard security measures, the password is never sent in the open
	over the network... 

	1. The server sends us a randomly generated key
	2. We hash that random key with our password and send the result back
	3. The server does the same thing, and compares the received key, if invalid it disconnects
*/

void BrokerClient::onConnect()
{}

void BrokerClient::onReceive( byte message, const InStream & input )
{
	AutoLock lock( &m_Lock );
	switch( message )
	{
	case CLIENT_RECEIVE_FILES:
		{
			// unregister the previous file list
			unregisterFiles();
			// receive the file list
			input >> m_Files;
			// register the new files
			registerFiles();
		}
		break;
	case CLIENT_RECEIVE_WIDGET:
		{
			// load the version then the widget
			//
			Widget * pWidget = NULL;
			input >> pWidget;

			ASSERT( pWidget );

			// find the file index
			int fileIndex = m_FileHash[ pWidget->key() ];
			ASSERT( fileIndex >= 0 );

			// store the received widget in the archive object
			//
			FileArchive::Ref pFile = new FileArchive( &m_Archive, pWidget->key() );
			pFile->setVersion( m_Files[ fileIndex ].version );

			OutStream output( pFile );
			output << pWidget;

			pFile->setEOF();

			// release the widget
			delete pWidget;
		}
		break;
	}

	// wake any waiting thread
	m_Received.signal();
}

void BrokerClient::onDisconnect()
{
	unregisterFiles();
}

//----------------------------------------------------------------------------

void BrokerClient::registerFiles()
{
	lock();

	// generate the hash table
	m_FileHash.release();
	int i = 0;
	for(;i<m_Files.size();i++)
		m_FileHash[ m_Files[i].key ] = i;

	// register all the files sent to us by the server
	for(i=0;i<m_Files.size();i++)
		registerWidget( this, m_Files[i].key, m_Files[i].version, m_Files[i].autoLoad, false );

	// remove any files in the archive that have been removed from the server
	Array< WidgetKey > removeList;
	for(i=0;i<m_Archive.fileCount();i++)
	{
		if ( ! m_FileHash.find( m_Archive.fileKey( i ) ).valid() )
			removeList.push( m_Archive.fileKey( i ) );
	}

	// remove any files from the archive for which the server has a newer version
	for(i=0;i<m_Files.size();i++)
	{
		int fileIndex = m_Archive.file( m_Files[i].key );
		if ( fileIndex >= 0 )
			if ( m_Archive.fileVersion( fileIndex ) != m_Files[i].version )
				removeList.push( m_Files[i].key );
	}

	// remove all listed files from the archive
	for(i=0;i<removeList.size();i++)
	{
		TRACE( CharString().format("BrokerClient, removing %s widget from local archive", (const char *)removeList[i].string()) );
		m_Archive.remove( removeList[i] );
	}

	// auto-load all widgets
	autoLoadWidgets();

	unlock();
}

void BrokerClient::unregisterFiles()
{
	lock();

	// unregister all the widgets
	for(int i=0;i<m_Files.size();i++)
		unregisterWidget( this, m_Files[i].key, m_Files[i].version );

	m_Files.release();
	m_FileHash.release();

	unlock();
}

//----------------------------------------------------------------------------
// EOF
