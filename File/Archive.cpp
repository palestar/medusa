/*
	Archive.cpp

	This object is used to store / retrieve data ... it stores it's data in a single file
	which is segmented into sectors just like a hard drive.

	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "File/Archive.h"

//----------------------------------------------------------------------------

const dword SECTOR_FREE = 0xffffffff;
const dword SECTOR_LAST = 0xfffffffe;
const dword	ARCHIVE_ID = 'ARCH';

//----------------------------------------------------------------------------

Archive::Archive() : m_Open( false ), m_pFile( new FileDisk() )
{}

Archive::Archive( const char * archivePath, Access access ) : m_Open( false ), m_pFile( new FileDisk() )
{
	if (! open( archivePath, access ) )
		throw Failure();
}

Archive::~Archive()
{
	close();
}

//----------------------------------------------------------------------------

Archive::Access Archive::access() const
{
	return( m_Access );
}

int Archive::fileCount() const
{
	return( m_Files.size() );
}

WidgetKey Archive::fileKey( int n ) const
{
	return( m_Files[ n ].key );
}

dword Archive::fileVersion( int n ) const
{
	return( m_Files[ n ].version );
}

dword Archive::fileFlags( int n ) const
{
	return( m_Files[ n ].flags );
}

dword Archive::fileSize( int n ) const
{
	return( m_Files[ n ].size );
}

dword Archive::fileSector( int n ) const
{
	return( m_Files[ n ].sector );
}

int Archive::file( const WidgetKey & key ) const
{
	FileHashIt find = m_FileHash.find( key );
	if ( find.valid() )
		return( *find );

	return -1;
}

//----------------------------------------------------------------------------

bool Archive::open( const char * archivePath, Access access )
{
	lock();

	// close this archive if previously open
	close();

	// set the access
	m_Access = access;
	// attempt to open the file
	if (! m_pFile->open( archivePath, access == READ ? FileDisk::READ : FileDisk::READ_WRITE ) || !readTable() )
	{
		unlock();
		return false;	// failed
	}

	m_Open = true;

	unlock();

	// success
	return true;
}

void Archive::close()
{
	lock();
	if ( m_Open )
	{
		// close any open file handles
		while(m_Handles.size() > 0)
			close( m_Handles[0] );

		// close the actual file
		m_pFile->close();
		m_Open = false;

		m_Files.release();
		m_FileHash.release();
		m_SectorMap.release();
	}
	unlock();
}

//----------------------------------------------------------------------------

Archive::Handle * Archive::open( const WidgetKey & key )
{
	ASSERT( m_Open );

	lock();

	int fileIndex = file( key );
	if ( fileIndex < 0 )
	{
		if ( m_Access == READ )
		{
			unlock();
			return NULL;
		}

		fileIndex = m_Files.size();

		// allocate a new entry
		File & createFile = m_Files.push();
		createFile.key = key;
		createFile.version = 0;
		createFile.flags = 0;
		createFile.size = 0;
		createFile.sector = allocateSector();
		
		// add new entry to the hash table
		m_FileHash[ key ] = fileIndex;

		writeTable();
	}
	
	File & openFile = m_Files[ fileIndex ];

	// create the handle
	Handle * pHandle = new Handle;
	memset( pHandle, 0, sizeof(Handle));
	pHandle->key = openFile.key;
	pHandle->sector = openFile.sector;
	pHandle->size = openFile.size;
	pHandle->users = 1;

	// keep track of handle
	m_Handles.push( pHandle );

	// load in the first sector
	readSector( pHandle );

	unlock();

	return pHandle;
}

Archive::Handle * Archive::duplicate( Handle * pDupHandle )
{
	ASSERT( m_Open && pDupHandle );
	pDupHandle->users++;

	return( pDupHandle );
}

bool Archive::remove( const WidgetKey & key )
{
	ASSERT( m_Open && m_Access == READ_WRITE );
	lock();

	// you cannot remove a file while it's open
	if ( fileOpen( key ) )
	{
		unlock();
		return false;
	}

	int fileIndex = file( key );
	if ( fileIndex < 0 )
	{
		unlock();
		return false;
	}

	File & removeFile = m_Files[ fileIndex ];

	// deallocate the sectors for this file
	dword sector = removeFile.sector;
	while( sector != SECTOR_LAST )
	{
		dword next = m_SectorMap[ sector ];
		m_SectorMap[ sector ] = SECTOR_FREE;

		sector = next;
	}
	
	// remove from file table
	m_Files.remove( fileIndex );
	// rebuild the hash
	updateHash();
	// write the new table
	writeTable();

	unlock();

	// file removed
	return true;
}

dword Archive::read( Handle * pHandle, void * pBuffer, dword nBytes )
{
	if (! m_Open || !pHandle || !pBuffer )
		return 0;
	
	dword nBytesRead = 0;
	while( nBytesRead < nBytes )
	{
		dword nRemain = ARCHIVE_SECTOR_SIZE - pHandle->offset;		// number of bytes remaing in buffer
		dword nRead = nBytes > nRemain ? nRemain :nBytes;			// number of bytes to copy from this sector
		if ( (pHandle->position + nRead) > pHandle->size )
			nRead = pHandle->size - pHandle->position;
		if ( nRead <= 0 )
			break;

		// copy the bytes
		memcpy( pBuffer, pHandle->buffer + pHandle->offset, nRead );

		// advance the pointers
		pHandle->offset += nRead;
		pHandle->position += nRead;

		nBytesRead += nRead;
		pBuffer = ((byte *)pBuffer) + nRead;

		// do we need to load the next sector
		if ( nRemain == nRead )
		{
			lock();

			// was the last sector modified
			if ( pHandle->modified )
			{
				writeSector( pHandle );
				pHandle->modified = false;
			}

			dword nextSector = m_SectorMap[ pHandle->sector ];
			if ( nextSector == SECTOR_LAST )
				break;	// already at last sector??!!

			pHandle->sector = nextSector;
			pHandle->offset = 0;

			readSector( pHandle );
			
			unlock();
		}

		// continue reading from next sector if bytes > 0
	}

	// done with read
	return nBytesRead;
}

dword Archive::write( Handle * pHandle, const void * pBuffer, dword nBytes )
{
	if (! m_Open || !pHandle || !pBuffer || m_Access == READ )
		return 0;

	dword nBytesWritten = 0;
	while( nBytesWritten < nBytes )
	{
		dword remain = ARCHIVE_SECTOR_SIZE - pHandle->offset;	// number of bytes remaing in buffer
		dword write = nBytes > remain ? remain : nBytes;			// number of bytes to copy from this sector

		// copy the bytes
		memcpy( pHandle->buffer + pHandle->offset, pBuffer, write );
		pHandle->modified = true;

		// advance the pointers
		pHandle->offset += write;
		pHandle->position += write;

		if ( pHandle->position > pHandle->size )
			pHandle->size = pHandle->position;

		nBytesWritten += write;
		pBuffer = ((byte *)pBuffer) + write;

		// do we need to load the next sector
		if ( remain == write )
		{
			lock();

			// was the last sector modified
			if ( pHandle->modified )
			{
				writeSector( pHandle );
				pHandle->modified = false;
			}

			dword sector = pHandle->sector;
			dword nextSector = m_SectorMap[ sector ];
			ASSERT( nextSector != SECTOR_FREE );

			if ( nextSector == SECTOR_LAST )
			{
				nextSector = allocateSector();
				m_SectorMap[ sector ] = nextSector;
			}

			pHandle->sector = nextSector;
			pHandle->offset = 0;

			readSector( pHandle );
			
			unlock();
		}
	}

	return nBytesWritten;
}

void Archive::close( Handle * pHandle )
{
	ASSERT( m_Open && pHandle );

	// decrement user count
	pHandle->users--;
	if ( pHandle->users > 0 )
		return;

	lock();

	int handleIndex = m_Handles.search( pHandle );
	if ( handleIndex < 0 )
		throw Failure();				// invalid handle was passed in, we didn't find it in our open file list

	// flush any written data
	if ( pHandle->modified )
	{
		writeSector( pHandle );			// flush to disk any modified data
		pHandle->modified = false;
	}
	
	int fileIndex = file( pHandle->key );
	if ( fileIndex < 0 )
		throw Failure();

	// update the file table
	File & closeFile = m_Files[ fileIndex ];

	// update the table
	if ( pHandle->size != closeFile.size )
	{
		closeFile.size = pHandle->size;
		// the size of the file has changed, update the table
		writeTable();
	}

	// remove the handle
	m_Handles.remove( handleIndex );
	// release the handle
	delete pHandle;

	unlock();
}

dword Archive::setEOF( Handle * pHandle )
{
	if (! m_Open || !pHandle || m_Access == READ )
		return 0;

	lock();

	// set the file size
	pHandle->size = pHandle->position;

	dword sector = pHandle->sector;
	if ( m_SectorMap[ sector ] != SECTOR_LAST )
	{
		dword freeSector = m_SectorMap[ sector ];
		// mark the current as the last sector
		m_SectorMap[ sector ] = SECTOR_LAST;
		// deallocate sectors beyond the current
		while( freeSector != SECTOR_LAST )
		{
			dword nextSector = m_SectorMap[ freeSector ];
			m_SectorMap[ freeSector ] = SECTOR_FREE;
			freeSector = nextSector;
		}
		//m_SectorMap[ freeSector ] = SECTOR_FREE;

		// since were updating the table, go ahead and update m_Files
		int fileIndex = file( pHandle->key );
		if ( fileIndex < 0 )
			throw Failure();
		m_Files[ fileIndex ].size = pHandle->size;

		// update the table
		writeTable();
	}

	unlock();

	return pHandle->size;
}

dword Archive::setPosition( Handle * pHandle, dword position )
{
	if (! m_Open || !pHandle )
		return 0;
	if ( m_Access == READ && position >= pHandle->size )
		return 0;	// trying to set the position past the EOF on a read-only archive

	lock();

	// is the current sector data modified, if so then flush the data to disk
	if ( pHandle->modified )
	{
		writeSector( pHandle );
		pHandle->modified = false;
	}

	// set the position
	pHandle->position = position;
	if ( position > pHandle->size )
		pHandle->size = position;

	// set the correct sector and offset
	int fileIndex = file( pHandle->key );
	if( fileIndex < 0 )
		throw Failure();

	dword sector = m_Files[ fileIndex ].sector;
	while( position > ARCHIVE_SECTOR_SIZE )
	{
		dword nextSector = m_SectorMap[ sector ];
		ASSERT( nextSector != SECTOR_FREE );

		if ( nextSector == SECTOR_LAST )
		{
			nextSector = allocateSector();
			m_SectorMap[ sector ] = nextSector;
		}

		position -= ARCHIVE_SECTOR_SIZE;
		sector = nextSector;
	}

	pHandle->sector = sector;
	pHandle->offset = position;

	// read the sector data
	readSector( pHandle );

	unlock();

	return position;
}

void Archive::setVersion( Handle * pHandle, dword version )
{
	ASSERT( pHandle && m_Access == READ_WRITE );

	lock();
	
	int fileIndex = file( pHandle->key );
	ASSERT( fileIndex >= 0 );

	m_Files[ fileIndex ].version = version;
	writeTable();

	unlock();
}

void Archive::setFlags( Handle * pHandle, dword flags )
{
	ASSERT( pHandle && m_Access == READ_WRITE );

	lock();

	int fileIndex = file( pHandle->key );
	ASSERT( fileIndex >= 0 );

	m_Files[ fileIndex ].flags = flags;
	writeTable();

	unlock();
}

void Archive::lock()
{
	m_Lock.lock();
}

void Archive::unlock()
{
	m_Lock.unlock();
}

//----------------------------------------------------------------------------

struct ArchiveHeader
{
	dword		id;			// == ARCHIVE_ID
	dword		sectors;	// number of sectors in this archive
};

const int SECTOR_BLOCKS = 128;		// allocate in .25 mb blocks

//----------------------------------------------------------------------------

dword Archive::allocateSector()
{
	int i = 0;
	for(;i<m_SectorMap.size();i++)
		if ( m_SectorMap[ i ] == SECTOR_FREE )
		{
			m_SectorMap[ i ] = SECTOR_LAST;
			return i;
		} 

	// allocate a new set of sector blocks
	int sector = m_SectorMap.size();
	m_SectorMap.grow( SECTOR_BLOCKS );

	m_SectorMap[ sector ] = SECTOR_LAST;
	for(i=sector + 1;i<m_SectorMap.size();i++)
		m_SectorMap[ i ] = SECTOR_FREE;

	// write the table
	writeTable();

	return sector;
}

void Archive::writeSector( Handle * pHandle )
{
	m_pFile->setPosition( sizeof(ArchiveHeader) + (pHandle->sector * ARCHIVE_SECTOR_SIZE) );
	m_pFile->write( pHandle->buffer, sizeof(pHandle->buffer) );
}

void Archive::readSector( Handle * pHandle )
{
	m_pFile->setPosition( sizeof(ArchiveHeader) + (pHandle->sector * ARCHIVE_SECTOR_SIZE) );
	m_pFile->read( pHandle->buffer, sizeof(pHandle->buffer) );
}

bool Archive::readTable()
{
	if ( m_pFile->size() == 0 )
	{
		// really bad news if the following exception get's thrown
		if ( m_SectorMap.size() != 0 )
			throw Failure();
		// new archive file, initialize the header
		if ( m_Access == READ )
			return false;
		
		// write a pristine table to disk
		writeTable();

		return true;
	}


	// read in the header
	ArchiveHeader header;
	m_pFile->setPosition( 0 );
	m_pFile->read( &header, sizeof(header) );

	if (header.id != ARCHIVE_ID )
		return false;

	// read in the table
	m_pFile->setPosition( sizeof(ArchiveHeader) + header.sectors * ARCHIVE_SECTOR_SIZE );

	InStream input( m_pFile );
	input >> m_Files;
	input >> m_SectorMap;

	// build the file hash
	updateHash();

	return true;
}

void Archive::writeTable()
{
	// update the header
	ArchiveHeader header;
	header.id = ARCHIVE_ID;
	header.sectors = m_SectorMap.size();

	m_pFile->setPosition( 0 );
	m_pFile->write( &header, sizeof(header) );

	// write the table
	m_pFile->setPosition( sizeof(ArchiveHeader) + header.sectors * ARCHIVE_SECTOR_SIZE );

	OutStream output( m_pFile );
	output << m_Files;
	output << m_SectorMap;

	// set the end of file
	output.file()->setEOF();
}

void Archive::updateHash()
{
	m_FileHash.release();
	for(int i=0;i<m_Files.size();i++)
		m_FileHash[ m_Files[i].key ] = i;
}

bool Archive::fileOpen( const WidgetKey & key )
{
	for(int i=0;i<m_Handles.size();i++)
		if ( m_Handles[i]->key == key )
			return true;

	return false;
}

//----------------------------------------------------------------------------
// EOF
