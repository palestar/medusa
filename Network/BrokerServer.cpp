/*
	BrokerServer.cpp

	This server provides access to a broker file over a socket
	(c)2005 Palestar, Richard Lyle
*/

#include "File/FileArchive.h"
#include "BrokerServer.h"

//----------------------------------------------------------------------------

BrokerServer::BrokerServer( const char * archiveFile, Socket * pSocket, int serverPort, int maxClients )
{
	if ( ! start( archiveFile, pSocket, serverPort, maxClients ) )
		throw Failure();
}

BrokerServer::~BrokerServer()
{
	stop();
}

//-------------------------------------------------------------------------------

const dword FLAG_AUTOLOAD = 0x1;	// see BrokerArchive.cpp

void BrokerServer::onConnect( dword client )
{
	// build a file list 
	Array< BrokerClient::File >	m_Files( m_Archive.fileCount() );
	for(int i=0;i<m_Archive.fileCount();i++)
	{
		BrokerClient::File & file = m_Files[ i ];
		file.key = m_Archive.fileKey( i );
		file.version = m_Archive.fileVersion( i );
		file.size = m_Archive.fileSize( i );
		file.autoLoad = (m_Archive.fileFlags( i ) & FLAG_AUTOLOAD) ? true : false;
	}

	// send the client the file list
	OutStream output( send( client, BrokerClient::CLIENT_RECEIVE_FILES ) );
	output << m_Files;
}

#pragma warning(disable:4244)

void BrokerServer::onReceive( dword client, byte message, const InStream & input )
{
	switch( message )
	{
	case BrokerClient::SERVER_REQUEST_WIDGET:
		{
			// get the widget key
			WidgetKey key;
			input >> key;

			lock();

			// load the widget into a buffer, and send it to the client
			FileArchive file;
			if ( file.open( &m_Archive, key ) )
			{
				// allocate a buffer to read the widget into
				dword fileSize = file.size();
				byte * pBuffer = new byte[ fileSize ];
				
				// read the widget into this buffer
				file.read( pBuffer, fileSize );

				// send the data to the client
				OutStream output = send( client, BrokerClient::CLIENT_RECEIVE_WIDGET );
				output.write( pBuffer, fileSize );

				delete pBuffer;
			}
			else
			{
				TRACE( "Invalid widget key received from BrokerClient, closing connection" );
				removeClient( client );
			}

			unlock();
		}
		break;
	default:
		TRACE( "Unknow message received from client, disconnecting" );
		removeClient( client );
		break;
	}
}

//-------------------------------------------------------------------------------

bool BrokerServer::start( const char * archivePath, Socket * pSocket, int serverPort, int maxClients )
{
	if ( ! m_Archive.open( archivePath, Archive::READ ) )
		return false;
	if ( ! Server::start( pSocket, serverPort, maxClients ) )
		return false;

	return true;
}

void BrokerServer::stop()
{
	Server::stop();
	m_Archive.close();
}

//----------------------------------------------------------------------------
// EOF
