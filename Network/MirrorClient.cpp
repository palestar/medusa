/*
	MirrorClient.cpp

	This client object is used to connect to an MirrorServer
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "File/FileDisk.h"
#include "File/FileMemory.h"
#include "File/FindFile.h"
#include "FileSocket.h"
#include "File/Path.h"
#include "Standard/Time.h"
#include "Standard/ZLIB.h"
#include "Standard/MD5.h"
#include "MirrorClient.h"

//----------------------------------------------------------------------------

const int FILE_TIMEOUT = 3600 * 1000;			// how long to wait to receive 1 file
const int CATALOG_TIMEOUT = 300 * 1000;			// how long to wait to receive the catalog

//----------------------------------------------------------------------------

static CharString FormatBytes( dword bytes )
{
	if ( bytes > 1000000 )
		return CharString().format( "%.1f mb", float( bytes ) / 1000000.0f );

	return CharString().format( "%.1f k", float(bytes) / 1000.0f );
}

//----------------------------------------------------------------------------

MirrorClient::MirrorClient() 
	: m_nServerVersion( 0 ), m_bConnected( false ), m_bLoggedIn( false ), 
	m_bSelfUpdating( false ), m_bCatalogValid( false ), m_nTotalBytes( 0 ), m_nBytes( 0 ),
	m_bLocalCatalog( false ), m_bLocalDirty( false )
{}

MirrorClient::~MirrorClient()
{
	close();
}

//----------------------------------------------------------------------------

void MirrorClient::onConnect()
{
	TRACE( "MirrorClient::onConnect" );

	// wake the open
	m_bConnected = true;
	m_bLoggedIn = false;
	m_bCatalogValid = false;
}

const int CHUNK_SIZE = 1024 * 8;

void MirrorClient::onReceive( byte message, const InStream & input )
{
	switch( message )
	{
	case CLIENT_HELLO:
		{
			dword nJob;
			input >> nJob;
			input >> m_nServerVersion;

			TRACE( CharString().format( "CLIENT_HELLO - nJob = %u, nServerVersion = %u", nJob, m_nServerVersion ) );
			signalJob( nJob, true );
		}
		break;
	case CLIENT_RECV_CRC:
		{
			dword nJob;
			input >> nJob;
			dword nCRC;
			input >> nCRC;

			TRACE( CharString().format( "CLIENT_RECV_CRC - nJob = %u, nCRC = %u", nJob, nCRC ) );
			signalJob( nJob, nCRC );
		}
		break;
	case CLIENT_RECV_CATALOG:
		{
			dword nJob;
			input >> nJob;
			bool bCatalogValid;
			input >> bCatalogValid;

			TRACE( CharString().format( "CLIENT_RECV_CATALOG - nJob = %u, bCatalogValid = %s", nJob, bCatalogValid ? "Yes" : "No" ) );

			if ( bCatalogValid )
			{
				Catalog catalog;
				input >> catalog;

				lock();

				m_Catalog = catalog;
				m_bCatalogValid = true;

				unlock();
			}

			signalJob( nJob, bCatalogValid );
		}
		break;
	case CLIENT_RECV_FILE:
		{
			// receiving a file
			Item item;
			input >> item;

			TRACE( CharString().format( "CLIENT_RECV_FILE - item.name = %s", item.name.cstr() ) );

			if ( m_nTotalBytes < (m_nBytes + item.size) )
				m_nTotalBytes = m_nBytes + item.size;
			onProgress( m_nTotalBytes, m_nBytes );
			onFileProgress( item.name, item.size, 0 );

			// allocate buffer for uncompressed data
			byte * pBuffer = new byte[ item.size ];
			ASSERT( pBuffer );

			bool bReceived = false;
			try {

				dword nReceived = 0;
				while( nReceived < item.size )
				{
					onFileProgress( item.name, item.size, nReceived );
					onProgress( m_nTotalBytes, m_nBytes );

					// get the size of the packet we're receiving
					dword nChunk;
					input >> nChunk;

					input.read( pBuffer + nReceived, nChunk );
					nReceived += nChunk;
					m_nBytes += nChunk;				// update overall progress
				}
				bReceived = nReceived >= item.size;
			}
			catch( FileSocket::FileError )
			{
				onError( CharString().format( "Error while receiving file %s...", (const char *)item.name) );
			}

			if ( bReceived )
			{
				AutoLock lock( &m_Lock );

				// save the file to disk
				CharString sLocalFile( m_MirrorInto + item.name );
				FileDisk::normalizePath( sLocalFile.buffer() );

				// make sure the directory exists
				FileDisk::createDirectory( Path( sLocalFile ).directory() );

				TRACE( CharString().format( "MirrorClient::onReceive, received file = %s, crc = %u, size = %u, localFile = %s", 
					(const char *)item.name, item.crc, item.size, (const char *)sLocalFile) );

				// remove the read-only flag
				FileDisk::lockFile( sLocalFile, false );

				// save the file, note that if self update is true, it will force it to save the file with the .upd extention... 
				// as to avoid partial DLL updates
				bool bSaved = true;
				if( m_bSelfUpdating || !FileDisk::saveFile( sLocalFile, pBuffer, item.size ) )
				{
					// append .upd extention, and attempt to save file again
					sLocalFile += ".upd";
					if ( !FileDisk::saveFile( sLocalFile, pBuffer, item.size) )
					{
						onError( CharString().format( "Failed to save '%s' to disk!", (const char *)sLocalFile ) );
						bSaved = false;
					}
				}

				// update our catalog
				if ( bSaved )
				{
					FileDisk::touchFile( sLocalFile, item.date );

					CharString sKey( item.name );
					normalizeKey( sKey );

					m_Catalog[ sKey ] = item;
					m_LocalCatalog[ sKey ] = item;
				}
			}

			// release allocated memory
			delete [] pBuffer;
		}
		break;
	case CLIENT_FILE_RECEIVED:
		{
			dword nJob;
			input >> nJob;
			bool bJobDone;
			input >> bJobDone;
			Item item;
			input >> item;

			LOG_DEBUG_MED( "MirrorClient", "CLIENT_FILE_RECEIVED - nJob = %u, bJobDone = %s, item.name = %s, item.crc = %u", 
				nJob, bJobDone ? "Yes" : "No", item.name.cstr(), item.crc );

			if ( bJobDone )
			{
				AutoLock lock( &m_Lock );

				CharString sKey( item.name );
				normalizeKey( sKey );

				m_Catalog[ sKey ] = item;
				m_LocalCatalog[ sKey ] = item;
			}

			signalJob( nJob, bJobDone );
		}
		break;
	case CLIENT_FILE_DELETED:
		{
			dword nJob;
			input >> nJob;
			bool bJobDone;
			input >> bJobDone;
			CharString sFile;
			input >> sFile;
			
			TRACE( CharString().format( "CLIENT_FILE_DELETED - nJob = %u, bJobDone = %s, sFile = %s", 
				nJob, bJobDone ? "Yes" : "No", sFile.cstr() ) );

			if ( bJobDone )
			{
				AutoLock lock( &m_Lock );

				CharString sKey( sFile );
				normalizeKey( sKey );

				m_Catalog.remove( sKey );
				m_LocalCatalog.remove( sKey );
			}

			signalJob( nJob, bJobDone );
		}
		break;
	case CLIENT_JOB_DONE:
		{
			dword nJob;
			input >> nJob;
			bool bJobDone;
			input >> bJobDone;

			TRACE( CharString().format( "CLIENT_JOB_DONE - nJob = %u, bJobDone = %s", 
				nJob, bJobDone ? "Yes" : "No") );

			signalJob( nJob, bJobDone );
		}
		break;
	}
}

void MirrorClient::onDisconnect()
{
	TRACE( "MirrorClient::onDisconnect" );

	m_bLoggedIn = false;
	m_bConnected = false;
	m_nServerVersion = 0;

	// did we get disconnected while transfering files?
	if ( m_nBytes < m_nTotalBytes )
		onError( "Lost connection to server!" );
}

//----------------------------------------------------------------------------

bool MirrorClient::open( const char * pAddress, int nPort, const char * pMirrorInto, const char * pCatalog /*= NULL*/, bool bSelfUpdating /*= true*/ )
{
	m_bLoggedIn = false;
	m_bConnected = false;
	m_bSelfUpdating = bSelfUpdating;

	// save root directory
	m_MirrorInto = pMirrorInto;
	FileDisk::normalizePath( m_MirrorInto.buffer() );
	if ( !m_MirrorInto.endsWith( PATH_SEPERATOR ) )
		m_MirrorInto += PATH_SEPERATOR;

	if ( pCatalog == NULL )
		m_sLocalCatalogFile = m_MirrorInto + "catalog.crc";
	else
		m_sLocalCatalogFile = pCatalog;

	// attempt the connection
	if (! Client::open( new Socket(), pAddress, nPort ) )
		return false;

	// say hello to the server
	dword nJob = createJob();
	OutStream output( send( SERVER_HELLO ) );
	output << nJob << version();
	output.flush();

	// wait for hello back
	if ( waitJob( nJob, 15 * 1000 ) < 0 )	// timeout in 15 seconds
		return false;

	//// load the local catalog
	//loadLocalCatalog();
	//// check the local catalog against the actual files found
	//checkCatalog( m_MirrorInto, m_LocalCatalog );

	return m_bConnected;
}

bool MirrorClient::login( const char * pUID, const char * pPW )
{
	if ( !m_bConnected )
		return false;

	lock();

	m_bLoggedIn = false;
	dword nJob = createJob();
	
	OutStream output( send( SERVER_LOGIN ) );
	output << nJob << CharString( pUID ) << CharString( MD5( pPW ).checksum() );
	output.flush();

	unlock();

	if ( waitJob( nJob, 15 * 1000 ) > 0 )
		m_bLoggedIn = true;
	
	return m_bLoggedIn;
}

bool MirrorClient::login( dword nSID )
{
	if ( !m_bConnected )
		return false;

	lock();

	m_bLoggedIn = false;
	dword nJob = createJob();
	
	OutStream output( send( SERVER_LOGIN_SID ) );
	output << nJob << nSID;
	output.flush();

	unlock();

	if ( waitJob( nJob, 15 * 1000 ) > 0 )
		m_bLoggedIn = true;
	
	return m_bLoggedIn;
}

void MirrorClient::close()
{
	// save all changes to the local catalog
	saveLocalCatalog();
	// close the actual socket
	Client::close();
}

dword MirrorClient::syncronize( bool bUpload /*= false*/ )
{
	dword nJobId = 0;

	// create a job with a -1 result, so we can return it's ID on error..
	dword nErrorId = createJob();
	signalJob( nErrorId, -1 );

	// load the local catalog if needed..
	if (! m_bLocalCatalog )
		loadLocalCatalog();

	// make a copy of the local catalog, so we can compare it...
	Catalog OldCatalog = m_LocalCatalog;
	m_bLocalDirty = true;

	// check the local catalog against the actual files found
	onStatus( "Updating local catalog..." );
	checkCatalog( m_MirrorInto, m_LocalCatalog, bUpload );

	dword nCRC = getCRC();
	if ( !bUpload && nCRC != 0 && nCRC == getLocalCRC() )
		return 0;		// nothing has changed... and we are not uploading, so we are in sync..

	// request the remote catalog...
	if ( !getCatalog() )
	{
		onError( "Failed to get remote catalog!" );
		return nErrorId;			// failed to get the catalog
	}
	
	onStatus( "Checking local files..." );
	CatalogIt it = m_LocalCatalog.head();
	while( it.valid() )
	{
		CharString sKey( it.key() );

		Item item = *it;
		it.next();
		
		CatalogIt find = m_Catalog.find( sKey );
		if ( find.valid() )
		{
			if ( (*find).crc == item.crc )
				continue;		// local/remote files the same, continue...
			if (! bUpload )
				continue;		// file modified locally, but we are not uploading...
			
			// check for a file conflict...
			CatalogIt old = OldCatalog.find( sKey );
			if ( old.valid() && (*old).crc != (*find).crc && (*old).crc != item.crc )
			{
				// file modified remotely... 
				int nAction = onFileConflict( item.name );
				if ( nAction == ABORT )
					return 0;
				if ( nAction == USE_REMOTE )
					continue;
			}
			else if ( old.valid() &&  (*old).crc == item.crc )
				continue;		// file modified remotely, let it get downloaded...
		}
		else if (! bUpload )
		{
			// not found in the remote catalog, remove from the local one...
			m_LocalCatalog.remove( sKey );

			// optionally delete the file as well..
			if ( onFileDelete( item.name ) )
				FileDisk::deleteFile( m_MirrorInto + item.name );

			continue;
		}

		if ( bUpload && onFileUpload( item.name ) )
		{
			m_Catalog[ sKey ] = item;

			if ( nJobId != 0 )
			{
				LOG_DEBUG_LOW( "MirrorClient", "Waiting for job %u to complete before put.", nJobId );
				if ( waitJob( nJobId, FILE_TIMEOUT ) <= 0 )
				{
					LOG_DEBUG_LOW( "MirrorClient", "Job %u failed.", nJobId );
					return nErrorId;		
				}
				nJobId = 0;
			}

			if ( (nJobId = putFile( item.name )) == 0 )
				return nErrorId;			// failed to put file
		}
	}

	onStatus( "Checking remote files..." );

	Array< CharString > files;
	it = m_Catalog.head();
	while( it.valid() )
	{
		CharString sKey( it.key() );

		Item & item = *it;
		it.next();
		
		CatalogIt local = m_LocalCatalog.find( sKey );
		if ( local.valid() )
		{
			if ( (*local).crc == item.crc )
				continue;		// file has not be changed, next item...
		}
		else if ( bUpload )
		{
			// local file gone, was the file deleted locally...
			CatalogIt old = OldCatalog.find( sKey );
			if ( old.valid() && onFileDeleteRemote( item.name ) )
			{
				if ( (nJobId = deleteFile( item.name )) == 0 )
					return nErrorId;		// failed to send job...
				m_Catalog.remove( sKey );
				continue;
			}
		}
		else 
		{
			// add to the local catalog
			m_LocalCatalog[ sKey ] = item;
			// make sure we don't already have the correct file.. 
			dword nLocalCRC = checksum( m_MirrorInto + item.name );
			if ( nLocalCRC != 0 && nLocalCRC == item.crc )
				continue;
		}
		
		// file does not exist or has been changed...
		if ( onFileDownload( item.name ) )
		{
			m_nTotalBytes += item.size;			// increment the total bytes by the file size...
			files.push( item.name );
		}
	}

	// save any local catalog changes..
	saveLocalCatalog();

	for(int i=0;i<files.size();i++)
	{
		if ( nJobId != 0 )
		{
			LOG_DEBUG_LOW( "MirrorClient", "Waiting for job %u to complete before get.", nJobId );
			if ( waitJob( nJobId, FILE_TIMEOUT ) <= 0 )
			{
				LOG_DEBUG_LOW( "MirrorClient", "Job %u failed.", nJobId );
				return nErrorId;		
			}
			nJobId = 0;
		}
		if ( (nJobId = getFile( files[i] )) == 0 )
			return nErrorId;			// failed to get file
	}

	return nJobId;
}

dword MirrorClient::getCRC()
{
	lock();

	dword nJob = createJob();
	OutStream output( send( SERVER_SEND_CRC ) );
	output << nJob;
	output.flush();

	unlock();

	int result = waitJob( nJob, FILE_TIMEOUT );
	if ( result < 0 )
		return 0;

	return (dword)result;
}

dword MirrorClient::getLocalCRC()
{
	lock();

	if (! m_bLocalCatalog )
		loadLocalCatalog();

	dword crc = 0;

	CatalogIt it = m_LocalCatalog.head();
	while( it.valid() )
	{
		crc += (*it).crc;
		it.next();
	}

	unlock();

	// clear the high-bit
	crc &= 0x7fffffff;		
	return crc;
}

bool MirrorClient::getCatalog()
{
	if ( !m_bConnected )
		return false;

	onStatus( "Requesting file catalog..." );

	// invalid the catalog first
	m_bCatalogValid = false;

	// send request for catalog
	dword nJob = createJob();
	OutStream output( send( SERVER_SEND_CATALOG ) );
	output << nJob;	
	output.flush();

	// wait up to 2 minutes to receive the catalog
	if ( waitJob( nJob, CATALOG_TIMEOUT ) <= 0 )
		return false;

	return true;
}

dword MirrorClient::getFile( const char * pFile )
{
	if (! m_bConnected  )
		return 0;

	dword nJob = createJob();
	OutStream output( send( SERVER_SEND_FILE ) );
	output << nJob << CharString( pFile );
	output.flush();

	return nJob;
}

dword MirrorClient::getFiles( const Array< CharString > & files )
{
	if (! m_bConnected || files.size() <= 0 )
		return 0;

	// send the file list
	dword nJob = createJob();
	OutStream output( send( SERVER_SEND_FILES ) );
	output << nJob << files;
	output.flush();

	// return the job ID
	return nJob;
}

dword MirrorClient::putFile( const char * pFile )
{
	dword nJob = 0;

	CharString sFilePath( m_MirrorInto + pFile );

	FileDisk fd;
	if ( fd.open( sFilePath ) )
	{
		dword nDataSize = fd.size();
		byte * pData = new byte[ nDataSize ];
		fd.read( pData, nDataSize );
		fd.close();

		Item item;
		item.name = pFile;
		item.size = nDataSize;
		item.crc = checksum( pData, nDataSize );
		item.date = FileDisk::fileDate( sFilePath );

		nJob = createJob();
		OutStream output( send( SERVER_RECV_FILE ) );
		output << nJob << item;
		output.write( pData, nDataSize );
		output.flush();

		delete [] pData;

		TRACE( CharString().format( "MirrorClient::putFile() - nJob = %u, item.name = %s, item.crc = %u", 
			nJob, item.name.cstr(), item.crc ) );

		// go ahead and update the local catalog...
		CharString sKey( pFile );
		normalizeKey( sKey );

		m_LocalCatalog[ sKey ] = item;
	}

	return nJob;
}

dword MirrorClient::putFiles( const Array< CharString > & files )
{
	dword nJob = 0;
	for(int i=0;i<files.size();i++)
		nJob = putFile( files[i] );
	return nJob;
}

dword MirrorClient::deleteFile( const char * pFile )
{
	if (! m_bConnected )
		return 0;

	dword nJob = createJob();
	OutStream output( send( SERVER_DELETE_FILE ) );
	output << nJob << CharString( pFile );
	output.flush();

	// return the job ID
	return nJob;
}

dword MirrorClient::deleteFiles( const Array< CharString > & files )
{
	dword nJob = 0;
	for(int i=0;i<files.size();i++)
		nJob = deleteFile( files[i] );
	return nJob;
}

void MirrorClient::deleteLocalFiles( const Array< CharString > & files )
{
	lock();
	for(int i=0;i<files.size();i++)
	{
		if ( FileDisk::deleteFile( m_MirrorInto + files[i] ) )
		{
			CharString sKey( files[i] );
			normalizeKey( sKey );

			m_LocalCatalog.remove( sKey );
		}
	}
	unlock();
}

//----------------------------------------------------------------------------

void MirrorClient::setLocalCatalog( const char * pFile )
{
	m_sLocalCatalogFile = pFile;
}

void MirrorClient::loadLocalCatalog()
{
	//onStatus( "Loading local catalog..." );

	m_LocalCatalog.release();
	loadCatalog( m_sLocalCatalogFile, m_LocalCatalog );

	m_bLocalCatalog = true;
	m_bLocalDirty = false;
}

void MirrorClient::saveLocalCatalog()
{
	if ( m_bLocalCatalog && m_bLocalDirty )
	{
		//onStatus( "Saving local catalog..." );
		if (! saveCatalog( m_sLocalCatalogFile, m_LocalCatalog ) )
			onError( CharString().format( "Failed to save local catalog (%s)", m_sLocalCatalogFile.cstr()) );

		m_bLocalDirty = false;
	}
}

//----------------------------------------------------------------------------

void MirrorClient::onStatus( const char * pStatus )
{
	TRACE( pStatus );
}

void MirrorClient::onProgress( dword nTotalBytes, dword nBytes )
{
	//TRACE( CharString("MirrorClient::onProgress, bytes = %u, bytesReceived = %u", bytes, bytesReceived ) );
}

void MirrorClient::onFileProgress( const char * pFile, dword nTotalBytes, dword nBytes )
{}

int MirrorClient::onFileConflict( const char * pFile )
{
	// file was changed remotely and locally, return false to keep the local file, or true to get the remote file
	TRACE( CharString().format( "MirrorClient::onFileConflict, pFile = %s", pFile ) );
	return USE_REMOTE;		// default conflicts to the remote file
}

bool MirrorClient::onFileDownload( const char * pFile )
{
	TRACE( CharString().format( "MirrorClient::onFileDownload, pFile = %s", pFile ) );
	// file has changed remotely
	return true;
}

bool MirrorClient::onFileUpload( const char * pFile )
{
	TRACE( CharString().format( "MirrorClient::onFileUpload, pFile = %s", pFile ) );
	// file has changed locally (or new file)
	return false;
}

bool MirrorClient::onFileDelete( const char * pFile )
{
	TRACE( CharString().format( "MirrorClient::onFileDelete, pFile = %s", pFile ) );
	// file was deleted remotely, return true to delete the file locally as well...
	return false;
}

bool MirrorClient::onFileDeleteRemote( const char * pFile )
{
	TRACE( CharString().format( "MirrorClient::onFileDeleteRemote, pFile = %s", pFile ) );
	// file was deleted locally, return true to delete the remote file as well...
	return false;
}

void MirrorClient::onError( const char * pError )
{
	TRACE( CharString().format( "MirrorClient::onError, error = %s", pError ) );
}

//----------------------------------------------------------------------------

dword MirrorClient::version()
{
	return VERSION_20051227;
}

bool MirrorClient::loadCatalog( const char * pFile, Catalog & catalog )
{
	try {
		InStream input( new FileDisk( pFile ) );

		int nVersion;
		input >> nVersion;

		switch( nVersion )
		{
		case VERSION_20051227:
			input >> catalog;
			return true;
		}
	}
	catch( FileDisk::FileError )
	{}

	return false;
}

bool MirrorClient::saveCatalog( const char * pFile, const Catalog & catalog )
{
	bool bSaved = false;

	try {
		OutStream output( new FileDisk( pFile, FileDisk::WRITE ) );
		output << VERSION_20051227;
		output << catalog;
		bSaved = true;
	}
	catch( FileDisk::FileError )
	{}

	// set the catalog file as hidden
	FileDisk::hideFile( pFile, true );
	return bSaved;
}

void MirrorClient::buildCatalog( const char * pRoot, const char * pMask, Catalog & catalog, bool bUploading )
{
	// find all files in this directory, exclude the hidden files
	FindFile ff( pMask );
	CharString sDir( Path( pMask ).directory() );

	int nRootLength = strlen( pRoot );

	// add all files to the catalog 
	int i = 0;
	for(;i<ff.fileCount();i++)
	{
		CharString sLocalFile( sDir + ff.file( i ) );

		Item item;
		item.name = sLocalFile;
		item.name.right( item.name.length() - nRootLength );
		item.date = FileDisk::fileDate( sLocalFile );
		item.size = FileDisk::fileSize( sLocalFile );

		CharString sKey( item.name );
		normalizeKey( sKey );

		if (! bUploading && !catalog.find( sKey ).valid() )
			continue;		// if we are not uploading, then skip any files not already in our catalog..

		CatalogIt find = catalog.find( sKey );
		if ( find.valid() && item.date == (*find).date && item.size == (*find).size )
			item.crc = (*find).crc;
		else
			item.crc = checksum( sLocalFile );

		// add item to the catalog
		catalog[ sKey ] = item;
	}

	// recurse into sub-directories
	for(i=0;i<ff.directoryCount();i++)
	{
		CharString directory( ff.directory( i ) );
		if ( directory[0] != '.' )
			buildCatalog( pRoot, Path( pMask ).directory() + directory + PATH_SEPERATOR + "*", catalog, bUploading );			
	}
}

void MirrorClient::checkCatalog( const char * pRoot, Catalog & catalog, bool bUploading )
{
	CharString sRoot( pRoot );

	// add/update files in catalog...
	buildCatalog( pRoot, sRoot + "*", catalog, bUploading );

	// remove missing files from catalog...
	CatalogIt it = catalog.head();
	while( it.valid() )
	{
		Item item = *it;
		it.next();

		if (! FileDisk::fileExists( sRoot + item.name ) )
		{
			CharString sKey( item.name );
			normalizeKey( sKey );

			catalog.remove( sKey );
		}
	}
}

dword MirrorClient::checksum( const void * pData, dword size )
{
	dword crc = size;
	
	dword dw = size >> 2;
	while( dw-- )
	{
		crc += *((dword *)pData);
		pData = ((dword *)pData) + 1;
	}

	dword remain = size & 0x3;
	while( remain-- )
	{
		crc += *((byte *)pData);
		pData = ((byte *)pData) + 1;
	}

	return crc;
}

dword MirrorClient::checksum( const char * pFile )
{
	FileDisk fd;
	if ( fd.open( pFile ) )
	{
		dword size = fd.size();
		byte * pData = new byte[ size ];

		fd.read( pData, size );
		fd.close();

		dword crc = checksum( pData, size );
		delete [] pData;

		return crc;
	}

	return 0;
}

void MirrorClient::normalizeKey( CharString & a_sKey )
{
	a_sKey.lower();
	a_sKey.replace( '/', '\\' );
}

//----------------------------------------------------------------------------
// EOF
