/*
	Archive.h

	This object is used to store / retrieve data ... it stores it's data in a single file
	which is segmented into sectors just like a hard drive.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "Standard/WidgetKey.h"
#include "Standard/CriticalSection.h"
#include "Standard/Array.h"
#include "Standard/Hash.h"
#include "File/Stream.h"
#include "File/FileDisk.h"
#include "Factory/WidgetKey.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning(disable:4251)

class DLL Archive
{
public:
	// Exceptions
	class Failure {};
	// Types
	enum {
		ARCHIVE_SECTOR_SIZE = 2048,
	};
	enum Access
	{
		READ,
		READ_WRITE,
	};
	struct Handle
	{
		WidgetKey		key;		// key of the file record
		dword			sector;		// current sector in the buffer
		dword			offset;		// offset within the sector
		dword			position;	// position within the file
		dword			size;		// size of the file

		bool			modified;	// has the buffer been modified
		byte			buffer[ ARCHIVE_SECTOR_SIZE ];

		int				users;		
	};
	struct File
	{
		WidgetKey		key;		// record key
		dword			version;	// version of record
		dword			flags;		// user flags
		dword			sector;		// first sector of the file
		dword			size;		// size of record
	};

	// Construction
	Archive();
	Archive( const char * archivePath, Access access );
	virtual	~Archive();

	// Accessors
	Access		access() const;					// access

	int			fileCount() const;				// number of files stored in this archive
	WidgetKey	fileKey( int n ) const;			// get the id of record n
	dword		fileVersion( int n ) const;		// get the version of record n
	dword		fileFlags( int n ) const;		// get the file flags
	dword		fileSize( int n ) const;		// get the record size
	dword		fileSector( int n ) const;		// first sector of record

	int			file( const WidgetKey & key ) const;	// find file index from key

	// Mutators
	bool		open( const char * archivePath, Access access );
	void		close();

	Handle *	open( const WidgetKey & key );			// open / create a file
	Handle *	duplicate( Handle * pDupHandle );		// duplicate handle
	bool		remove( const WidgetKey & key );		// remove a file

	dword		read( Handle * pHandle, 
					void * pBuffer, dword bytes );		// read data from file
	dword		write( Handle * pHandle, 
					const void * pBuffer, dword bytes );// write data to file
	void		close( Handle * pHandle );				// close a file

	dword		setEOF( Handle * pHandle );
	dword		setPosition( Handle * pHandle, dword position );
	void		setVersion( Handle * pHandle, dword version );
	void		setFlags( Handle * pHandle, dword flags );

	void		lock();
	void		unlock();

private:
	// Types
	typedef Hash< WidgetKey, int >	FileHash;
	typedef FileHash::Iterator		FileHashIt;

	// Data
	bool		m_Open;				// is this archive open
	Access		m_Access;			// the acesss
	FileDisk::Ref
				m_pFile;			// the file
	CriticalSection
				m_Lock;	

	Array< File >
				m_Files;			// file
	FileHash	m_FileHash;			// file lookup
	Array< dword >	
				m_SectorMap;		// sector map
	Array< Handle * >
				m_Handles;			// open file handles

	// Mutators
	dword		allocateSector();	// get a available sector, or create one
	void		writeSector( Handle * pHandle );
	void		readSector( Handle * pHandle );

	bool		readTable();		// read the latest table from disk
	void		writeTable();		// write the table to disk
	void		updateHash();

	bool		fileOpen( const WidgetKey & key );
};

IMPLEMENT_RAW_STREAMING( Archive::File );

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
