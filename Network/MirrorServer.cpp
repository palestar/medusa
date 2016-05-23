/*
	MirrorServer.cpp

	This is the server object for MirrorClient
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "File/Path.h"
#include "Standard/StringHash64.h"
#include "Standard/Time.h"
#include "FileSocket.h"
#include "MirrorServer.h"

//----------------------------------------------------------------------------

const dword		SEND_FILE_CHUNK_SIZE = 1024 * 8;
const char *	CATALOG_FILENAME = "catalog.crc";

//----------------------------------------------------------------------------

MirrorServer::MirrorServer() : m_pUpdateThread( NULL ), m_bCatalogReady( false )
{}

MirrorServer::MirrorServer( Context & context ) : m_pUpdateThread( NULL )
{
	if ( !start( context ) )
		throw Failure();
}

MirrorServer::~MirrorServer()
{
	stop();
}

//----------------------------------------------------------------------------

void MirrorServer::onConnect( dword nClientId )
{
	lock();

	LOG_STATUS( "MirrorServer", "Connect, client = %d, ip = %s", nClientId, clientAddress( nClientId ) );
	m_ClientVersion[ nClientId ] = 0;

	unlock();
}

void MirrorServer::onReceive( dword nClientId, byte message, const InStream & input )
{
	switch( message )
	{
	case MirrorClient::SERVER_HELLO:
		{
			dword nJob;
			input >> nJob;
			dword nVersion;
			input >> nVersion;

			lock();

			LOG_STATUS( "MirrorServer", "Client %u hello, version = %u", nClientId, nVersion );
			// save the client version
			m_ClientVersion[ nClientId ] = nVersion;

			unlock();

			// say hello
			OutStream output( send( nClientId, MirrorClient::CLIENT_HELLO ) );
			output << nJob << MirrorClient::version();
			if ( nVersion < MirrorClient::VERSION_20051227 )
				output << false;		// bVersioNControl, no longer supported...

		}
		break;
	case MirrorClient::SERVER_LOGIN:
		{
			dword nJob;
			input >> nJob;
			CharString sUserId;
			input >> sUserId;
			CharString md5;
			input >> md5;

			bool bLogin = login( nClientId, sUserId, md5 );

			// log a message
			LOG_STATUS( "MirrorServer", "Client %u %slogin as '%s'", nClientId, bLogin ? "" : "failed ", sUserId.cstr() );
			OutStream output( send( nClientId, MirrorClient::CLIENT_JOB_DONE ) );
			output << nJob << bLogin;
		}
		break;
	case MirrorClient::SERVER_LOGIN_SID:
		{
			dword nJob;
			input >> nJob;
			dword nSID;
			input >> nSID;

			bool bLogin = login( nClientId, nSID );

			// log a message
			LOG_STATUS( "MirrorServer", "Client %u %slogin with SID = %u", nClientId, bLogin ? "" : "failed ", nSID );
			OutStream output( send( nClientId, MirrorClient::CLIENT_JOB_DONE ) );
			output << nJob << bLogin;
		}
		break;
	case MirrorClient::SERVER_SEND_CRC:
		{
			dword catalogCRC = 0;

			dword nJob;
			input >> nJob;

			lock();
			CatalogIt it = m_Catalog.head();
			while( it.valid() )
			{
				catalogCRC += (*it).crc;
				it.next();
			}
			unlock();

			// the client determines if an error occured if the value is negative, clear to high bit so it doesn't think an error occured
			catalogCRC &= 0x7fffffff;		
			OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_CRC ) );
			output << nJob << catalogCRC;
		}
		break;
	case MirrorClient::SERVER_SEND_CATALOG:
		{
			dword nJob;
			input >> nJob;

			lock();

			// make a copy of the catalog to send..
			Catalog catalog = m_Catalog;

			LOG_STATUS( "MirrorServer", "Client %u requesting catalog", nClientId );
			if ( clientVersion( nClientId ) < MirrorClient::VERSION_20050508 )
			{
				unlock();

				// legacy support
				OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_CATALOG ) );
				output.setFilter( MirrorClient::FF_DATE );
				output << nJob << catalog;

				break;
			}

			if ( m_bCatalogReady )
			{
				unlock();

				OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_CATALOG ) );
				if ( clientVersion( nClientId ) < MirrorClient::VERSION_20051227 )
					output.setFilter( MirrorClient::FF_DATE );
				output << nJob << true << catalog;
			}
			else
			{
				unlock();

				OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_CATALOG ) );
				output << nJob << false;				// catalog not ready yet
			}
		}
		break;
	case MirrorClient::SERVER_SEND_FILE:
		{
			dword nJob;
			input >> nJob;
			CharString sFile;
			input >> sFile;

			//LOG_STATUS( "MirrorServer", CharString().format("Client %u requested file %s!", nClientId, (const char *)sFile) );
	
			bool bSent = false;

			// make sure the file is in the catalog
			lock();

			CharString sKey( sFile );
			MirrorClient::normalizeKey( sKey );

			CatalogIt find = m_Catalog.find( sKey );
			if ( find.valid() )
			{
				MirrorClient::Item item( *find );
				unlock();

				// attempt to send the file
				bSent = sendFile( nClientId, item );
			}
			else
			{
				LOG_STATUS( "MirrorServer", "Client %u requested file %s not found!", nClientId, sFile.cstr() );
				unlock();
			}

			OutStream output( send( nClientId, MirrorClient::CLIENT_JOB_DONE ) );
			output << nJob << bSent;
		}
		break;
	case MirrorClient::SERVER_SEND_FILES:
		{
			dword nJob;
			input >> nJob;
			Array< CharString > sFiles;
			input >> sFiles;

			//LOG_STATUS( "MirrorServer", CharString().format("Client %u requested %d files!", nClientId, sFiles.size()) );

			bool bSent = false;
			for(int i=0;i<sFiles.size();i++)
			{
				// make sure the file is in the catalog
				lock();

				CharString sKey( sFiles[i] );
				MirrorClient::normalizeKey( sKey );

				CatalogIt find = m_Catalog.find( sKey );
				if ( find.valid() )
				{
					MirrorClient::Item item( *find );
					unlock();

					// attempt to send the file
					if (! (bSent = sendFile( nClientId, item )) )
						break;			// failed to send file
				}
				else
				{
					LOG_WARNING( "MirrorServer", CharString().format("Client %u requested file %s not found!", nClientId, (const char *)sFiles[i]) );
					unlock();
				}
			}

			OutStream output( send( nClientId, MirrorClient::CLIENT_JOB_DONE ) );
			output << nJob << bSent;
		}
		break;
	case MirrorClient::SERVER_RECV_FILE:
		{
			if ( clientVersion( nClientId ) < MirrorClient::VERSION_20051227 )
				const_cast<InStream &>( input ).setFilter( MirrorClient::FF_DATE );

			dword nJob;
			input >> nJob;
			Item item;
			input >> item;

			LOG_STATUS( "MirrorServer", "Client %u upload %s (%u bytes / %8.8X)", 
				nClientId, item.name.cstr(), item.size, item.crc );

			bool bJobDone = false;
			bool bReceived = false;

			// allocate a buffer to download the file into
			byte * pBuffer = new byte[ item.size ];
			ASSERT( pBuffer );
			try {
				// read file into buffer
				input.read( pBuffer, item.size );
				bReceived = true;
			}
			catch( FileSocket::FileError )
			{}

			if ( bReceived )
			{
				AutoLock lock( &m_Lock );
				// receive the file
				bJobDone = receiveFile( nClientId, item, pBuffer );
			}

			// release allocated memory
			delete [] pBuffer;

			// return result
			OutStream output( send( nClientId, MirrorClient::CLIENT_FILE_RECEIVED ) );
			output << nJob << bJobDone << item;
		}
		break;
	case MirrorClient::SERVER_DELETE_FILE:
		{
			dword nJob;
			input >> nJob;
			CharString sFile;
			input >> sFile;

			LOG_STATUS( "MirrorServer", "Client %u deleting file %s", nClientId, sFile.cstr() );
			lock();

			OutStream output( send( nClientId, MirrorClient::CLIENT_FILE_DELETED ) );
			output << nJob << deleteFile( nClientId, sFile ) << sFile;

			unlock();
		}
		break;
	default:
		LOG_STATUS( "MirrorServer", "bad data type %s, removing client!", clientAddress(nClientId) );
		removeClient( nClientId );
		break;
	}
}

void MirrorServer::onDisconnect( dword nClientId )
{
	LOG_STATUS( "MirrorServer", "Disconnect, client %u", nClientId );
	logout( nClientId );

	lock();
	m_ClientVersion.remove( nClientId );
	unlock();
}

//----------------------------------------------------------------------------


bool MirrorServer::start( Context & context )
{
	// save copy of the start context
	m_Context = context;

	lock();

	LOG_STATUS( "MirrorServer", "Starting, mirror = %s, port = %d, maxClients = %d", 
		m_Context.mirror.cstr(), m_Context.port, m_Context.maxClients );

	// load the catalog from disk
	MirrorClient::loadCatalog( m_Context.sCatalog, m_Catalog );
	// start the server
	if (! Server::start( new Socket( "ZLIB" ), m_Context.port, m_Context.maxClients ) )
	{
		LOG_ERROR( "MirrorServer", "Failed to start the server!" );
		unlock();
		return false;
	}

	// create the update thread, which will monitor the directory for changed files
	m_pUpdateThread = new UpdateThread( this );
	m_pUpdateThread->resume();

	unlock();

	return true;
}

void MirrorServer::stop()
{
	LOG_STATUS( "MirrorServer", "Stopping" );

	Server::stop();

	// stop the update thread
	if ( m_pUpdateThread )
	{
		delete m_pUpdateThread;
		m_pUpdateThread = NULL;
	}
}

//----------------------------------------------------------------------------

void MirrorServer::onUpdate()
{}

//----------------------------------------------------------------------------

bool MirrorServer::login( dword nClientId, const char * pUser, const char * pMD5 )
{
	return false;
}

bool MirrorServer::login( dword client, dword nSID )
{
	return false;
}

void MirrorServer::logout( dword nClientId )
{}

bool MirrorServer::canDownload( dword nClientId )
{
	return true;
}

bool MirrorServer::canUpload( dword nClientId )
{
	return false;
}

bool MirrorServer::canDelete( dword nClientId )
{
	return false;
}

//----------------------------------------------------------------------------

MirrorServer::UpdateThread::UpdateThread( MirrorServer * pServer ) : m_pServer( pServer )
{}

int MirrorServer::UpdateThread::run()
{
	m_pServer->updateDemon();
	return 0;
}

//----------------------------------------------------------------------------

class MirrorClientLink : public MirrorClient
{
public:
	// MirrorClient Interface
	virtual int	onFileConflict( const char * pFile )
	{
		LOG_STATUS( "MirrorServer", "File conflict on %s, using local...", pFile );
		return MirrorClient::USE_LOCAL;		// false to use local file, true to use remote file
	}
	virtual bool	onFileDownload( const char * pFile )
	{
		return false;		// we never download, we only upload...
	}
	virtual bool	onFileUpload( const char * pFile )
	{
		LOG_STATUS( "MirrorServer", "Link upload %s...", pFile );
		return true;
	}
	virtual bool	onFileDelete( const char * pFile )
	{
		return false;
	}
	virtual bool	onFileDeleteRemote( const char * pFile )
	{
		LOG_STATUS( "MirrorServer", "Link delete %s...", pFile );
		return true;
	}
	void onStatus( const char * pStatus )
	{
		LOG_STATUS( "MirrorServer", pStatus );
	}
	void onError( const char * pError )
	{
		LOG_ERROR( "MirrorServer", pError );
	}
};

//----------------------------------------------------------------------------

void MirrorServer::updateDemon()
{
	TRACE("MirrorServer::updateDemon started");

	// initialize the last file change so we'll update the catalog immediately...
	dword nLastFileChange = 1;
	dword nLastCatalogUpdate = 1;
	// track changes in the mirror directory
	bool bFileChange = m_FileChange.open( m_Context.mirror );
	
	while( running() )
	{
		Thread::sleep( 1000 );

		// have any files changed
		if ( bFileChange )
		{
			if ( m_FileChange.signaled() )
			{
				LOG_STATUS( "MirrorServer", "File Change detected in %s", m_Context.mirror.cstr() );
				nLastFileChange = Time::seconds();
			}
		}
		else
		{
			// no file change functionality, update the catalog every N minutes then..
			if ( (Time::seconds() - nLastCatalogUpdate) > (30 * 60) )
				nLastFileChange = nLastCatalogUpdate;
		}

		if ( nLastFileChange != 0 && ((nLastFileChange + m_Context.nMinUpdateTime) < Time::seconds()) )
		{
			updateCatalog();
			nLastFileChange = 0;
			nLastCatalogUpdate = Time::seconds();

			// set all links to update...
			for(int i=0;i<m_Context.links.size();i++)
				m_Context.links[i].bUpdate = true;
		}

		// send file updates to other mirror servers
		for(int i=0;i<m_Context.links.size();i++)
		{
			Link & link = m_Context.links[i];
			if (! link.bUpdate )
				continue;

			MirrorClientLink client;
			if ( client.open( link.sAddress, link.nPort, m_Context.mirror, link.sCatalog, false ) )
			{
				if ( client.login( m_Context.sUID, m_Context.sPW) )
				{
					LOG_STATUS( "MirrorServer", "Uploading to %s:%d...", link.sAddress.cstr(), link.nPort );

					dword nJobId = client.syncronize( true );
					if ( nJobId == 0 || client.waitJob( nJobId, 86400 * 1000 ) > 0 )
					{
						LOG_STATUS( "MirrorServer", "Link updated..." );
						link.bUpdate = false;		// last job completed, successfully..
					}
					else
						LOG_ERROR( "MirrorServer", "Failed to synchronize link." );
				}
			}
			client.close();
		}

		// call virtual update function, so another class can inherit
		onUpdate();
	}

	m_FileChange.close();
	TRACE("MirrorServer::updateDemon end");
}

void MirrorServer::updateCatalog()
{
	dword nStart = Time::seconds();
	LOG_STATUS( "MirrorServer", "Starting catalog update..." );

	lock();

	// add new items to catalog
	MirrorClient::checkCatalog( m_Context.mirror, m_Catalog, true );
	MirrorClient::saveCatalog( m_Context.sCatalog, m_Catalog );
	m_bCatalogReady = true;

	// TODO: inform any connected clients that the catalog has been updated...
	LOG_STATUS( "MirrorServer", "Updated catalog, %d files, %u seconds...", 
		m_Catalog.size(), Time::seconds() - nStart );

	unlock();
}

//----------------------------------------------------------------------------

dword MirrorServer::clientVersion( dword nClientId )
{
	dword nVersion = 0;

	lock();

	Hash< dword, dword >::Iterator find = m_ClientVersion.find( nClientId );
	if ( find.valid() )
		nVersion = *find;

	unlock();

	return nVersion;
}

//----------------------------------------------------------------------------

bool MirrorServer::receiveFile( dword nClientId, const Item & item, const byte * pBuffer )
{
	if (! canUpload( nClientId ) )
		return false;

	AutoLock lock( &m_Lock );

	// verify the checksum
	if ( item.crc != MirrorClient::checksum( pBuffer, item.size ) )
	{
		LOG_ERROR( "MirrorServer", CharString().format( "Invalid CRC on received file '%s'...", (const char *)item.name ) );
		return false;
	}

	// save the file to disk
	CharString sLocalFile( m_Context.mirror + item.name );
	CharString sLocalDirectory( Path( sLocalFile ).directory() );

	// make sure the directory exists
	if ( !FileDisk::createDirectory( sLocalDirectory ) )
	{
		LOG_ERROR( "MirrorServer", CharString().format( "Failed to create directory '%s'", (const char *)sLocalDirectory) );
		return false;
	}

	// save the file
	if( !FileDisk::saveFile( sLocalFile, pBuffer, item.size ) )
	{
		LOG_ERROR( "MirrorServer", CharString().format( "Failed to save '%s' to disk!", (const char *)sLocalFile ) );
		return false;
	}
	
	// keep the same date...
	FileDisk::touchFile( sLocalFile, item.date );

	// update the catalog
	CharString sKey( item.name );
	MirrorClient::normalizeKey( sKey );
	m_Catalog[ sKey ] = item;

	return true;
}

bool MirrorServer::sendFile( dword nClientId, const Item & item )
{
	if (! canDownload( nClientId ) )
		return false;

	bool bSent = false;

	// send this file...
	CharString sLocalFile( m_Context.mirror + item.name );
	if ( FileDisk::fileExists( sLocalFile ) )
	{
		FileDisk file;
		if ( file.open( sLocalFile, FileDisk::READ ) )
		{
			// get the latest size of the file
			Item sendItem = item;
			sendItem.size = file.size();

			byte * pBuffer = new byte[ sendItem.size ];
			try {
				// read the file into a buffer for sending
				file.read( pBuffer, sendItem.size  );
			}
			catch( FileDisk::FileError )
			{
				delete [] pBuffer;

				LOG_ERROR( "MirrorServer", CharString().format("Failed to read file %s from disk!", (const char *)sLocalFile) );
				return false;
			}

			//LOG_STATUS( "MirrorServer", CharString().format("Client %u send %s (%u bytes / %8.8X)", nClientId,
			//	(const char *)sendItem.name, sendItem.size, sendItem.crc) );

			dword nClientVersion = clientVersion( nClientId );
			Socket * pSocket = clientSocket( nClientId );

			// create the output stream
			OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_FILE ) );
			if ( nClientVersion < MirrorClient::VERSION_20051227 )
				output.setFilter( MirrorClient::FF_DATE );

			output << sendItem;
			
			// use a try/catch so that we can delete the buffer if the client disconnects
			try {
				// send the file data in chunks
				dword nSent = 0;
				while( nSent < sendItem.size )
				{
					dword nRemain = sendItem.size - nSent;
					dword nSend = nRemain > SEND_FILE_CHUNK_SIZE ? SEND_FILE_CHUNK_SIZE : nRemain;

					// send the number of bytes were sending
					output << nSend;
					// send the file data
					output.write( pBuffer + nSent, nSend );

					nSent += nSend;
				}

				// file sent!
				bSent = nSent >= sendItem.size;
			}
			catch( FileSocket::FileError )
			{
				LOG_STATUS( "MirrorServer", "Client %u disconnected while sending %s...", 
					nClientId, sLocalFile.cstr() );
			}

			// release the local buffer
			delete [] pBuffer;
		}
		else
			LOG_ERROR( "MirrorServer", CharString().format("Failed to open local file %s!", (const char *)sLocalFile) );
	}
	else
		LOG_ERROR( "MirrorServer", CharString().format("Failed to find local file %s!", (const char *)sLocalFile) );

	return bSent;
}

bool MirrorServer::deleteFile( dword nClientId, const char * pFile )
{
	if (! canDelete( nClientId ) )
		return false;

	AutoLock lock( &m_Lock );

	CharString sLocalFile( m_Context.mirror + pFile );
	if (! FileDisk::fileExists( sLocalFile ) )
		return false;
	FileDisk::lockFile( sLocalFile, false );
	if ( ! FileDisk::deleteFile( sLocalFile ) )
		return false;

	CharString sKey( pFile );
	MirrorClient::normalizeKey( sKey );

	m_Catalog.remove( sKey );
	return true;
}

//----------------------------------------------------------------------------
// EOF
