/*
	MirrorServerVersionControl.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define NETWORK_DLL
#include "Standard/Time.h"
#include "File/Path.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"
#include "Network/FileSocket.h"
#include "Network/MirrorServer.h"

#include <time.h>
#include <stdio.h>



//----------------------------------------------------------------------------

static char *	CURRENT_USER = "CurrentUser";
static char *	FIRST_VERSION = "FirstVersion";
static char *	LAST_VERSION = "LastVersion";
static char *	HISTORY_COUNT = "HistoryCount";
static char *	LABELS = "Labels";
static char *	LABEL_COUNT = "LabelCount";
static char *	DELETED = "Deleted";
static char *	DELETED_COUNT = "DeletedCount";

static char *	CONTROL_FILE = "control.ini";

//----------------------------------------------------------------------------

// this function returns true if the specified file is locked by the client
bool MirrorServer::isFileLocked( dword nClientId, const char * pFile )
{
	bool bLocked = false;

	lock();

	CharString sLocalFile = m_Context.mirror + pFile;
	if ( FileDisk::fileExists( sLocalFile ) )
	{
		if ( FileDisk::fileLocked( sLocalFile ) )
		{
			if ( nClientId != 0 )
			{
				// check for the read-only flag on the file, if found then it's locked by someone
				if ( getKeyValue( pFile, CURRENT_USER, "" ) ^= clientUser( nClientId ) )
					bLocked = true;		// user has the file locked!
			}
			else
				bLocked = true;
		}
	}

	unlock();

	return bLocked;
}

void MirrorServer::appendHistory( const char * pFile, const char * pHistory, dword nTime /*= 0*/ )
{
	if ( nTime == 0 )
		nTime = Time::seconds();

	dword nHistoryCount = getKeyValue( pFile, HISTORY_COUNT, (dword)0 );
	putKeyValue( pFile, HISTORY_COUNT, nHistoryCount + 1 );
	putKeyValue( pFile, CharString().format("History%d", nHistoryCount), 
		CharString().format("%s : %s", (const char *)Time::format( nTime, "%c" ), pHistory ) );
}

//----------------------------------------------------------------------------

bool MirrorServer::checkoutFile( dword nClientId, const MirrorClient::Item & item )
{
	bool bSuccess = false;

	lock();

	if ( canLock( nClientId ) )
	{
		if ( lockFile( nClientId, item.name ) )
		{
			// if files are different, then resend the current file to the client
			CharString sLocalFile = m_Context.mirror + item.name;
			if ( MirrorClient::checksum( sLocalFile ) != item.crc )
				if (! sendFile( nClientId, item ) )
					log("Failed to send file!");

			// add history entry for this file
			appendHistory( item.name, CharString().format("%s : Checked Out", (const char *)clientUser( nClientId ) ) );
			bSuccess = true;
		}
		else
			log("Failed to lock file!");
	}
	else
		log("canLock() returned false");

	unlock();

	return bSuccess;
}

bool MirrorServer::lockFile( dword nClientId, const char * pFile )
{
	bool bLocked = false;

	lock();

	CharString sLocalFile = m_Context.mirror + pFile;
	if ( FileDisk::fileExists( sLocalFile ) )
	{
		// check for the read-only flag on the file, if found then it's locked by someone
		if (! FileDisk::fileLocked( sLocalFile ) )
		{
			// set this user as the current user for the file
			putKeyValue( pFile, CURRENT_USER, clientUser( nClientId ) );
			// turn on the read-only flag now, which locks the file
			FileDisk::lockFile( sLocalFile, true );

			bLocked = true;
		}
		else if ( getKeyValue( pFile, CURRENT_USER, "" ) ^= clientUser( nClientId ) )
			bLocked = true;
		else
			log("File already locked!");
	}
	else
		log("File not found!");

	unlock();

	return bLocked;
}

bool MirrorServer::checkinFile( dword nClientId, const Item & item, const byte * pBuffer, bool bKeepLocked )
{
	bool bSuccess = false;
	if ( canLock( nClientId ) )
	{
		if ( !FileDisk::fileExists( m_Context.mirror + item.name ) || isFileLocked( nClientId, item.name) )
		{
			if ( receiveFile( nClientId, item, pBuffer ) )
			{
				if (! bKeepLocked )
				{
					if ( unlockFile( nClientId, item.name ) )
					{
						appendHistory( item.name, CharString().format("%s : Checked In", (const char *)clientUser( nClientId ) ) );
						bSuccess = true;
					}
					else
						log("Failed to unlock file!");
				}
				else
				{
					// do not unlock the file, keep it locked
					bSuccess = true;
				}
			}
			else
				log("Failed to receive file!");
		}
		else
			log("File not locked!");
	}
	else
		log("Invalid client rights!");

	return bSuccess;
}

bool MirrorServer::unlockFile( dword nClientId, const char * pFile )
{
	bool bSuccess = false;

	// check for the read-only flag on the file, if found then it's locked by someone
	CharString sLocalFile = m_Context.mirror + pFile;
	if ( FileDisk::fileExists( sLocalFile ) )
	{
		if ( isFileLocked( 0, pFile ) )
		{
			// get the userId of the person who has the file locked from the hidden INI file
			if ( !(getKeyValue( pFile,  CURRENT_USER, "" ) ^= clientUser( nClientId )) )
				appendHistory( pFile, CharString().format("%s : Forced unlock", (const char *)clientUser( nClientId ) ) );

			// clear the current user
			putKeyValue( pFile, CURRENT_USER, "" );
			// turn off the read-only flag now
			FileDisk::lockFile( sLocalFile, false );
			bSuccess = true;
		}
		else
			log("File not locked!");
	}
	else
		log("File not found!");

	// file has no read-only flag, it can be locked
	return bSuccess;
}

bool MirrorServer::uncheckFile( dword nClientId, const char * pFile, dword nCRC )
{
	bool bSuccess = false;

	if ( canLock( nClientId ) )
	{
		if ( isFileLocked( 0, pFile ) && unlockFile( nClientId, pFile ) )
		{
			CharString sLocalFile = m_Context.mirror + pFile;
			dword nLocalCRC = MirrorClient::checksum( sLocalFile );

			// if files are different, then resend the current file to the client
			if ( MirrorClient::checksum( sLocalFile ) != nCRC )
			{
				Item item;
				item.name = pFile;
				item.name.lower();
				item.crc = nLocalCRC;
				item.size = FileDisk::fileSize( sLocalFile );

				sendFile( nClientId, item );
			}

			// add history entry for this file
			appendHistory( pFile, CharString().format("%s : Undo Check Out", (const char *)clientUser( nClientId ) ) );
			bSuccess = true;
		}
		else
			log("Failed to unlock file!");
	}
	else
		log("Invalid rights!");

	return bSuccess;
}

bool MirrorServer::deleteFile( dword nClientId, const char * pFile )
{
	bool bDeleted = false;

	if ( canDelete( nClientId ) )
	{
		CharString sLocalFile = m_Context.mirror + pFile;

		if (! FileDisk::fileExists( sLocalFile ) )
		{
			// file doesn't exist locally, so assume the user is deleteing a local file for them...
			bDeleted = true;
		}
		else
		{
			log( CharString().format("Client %u delete file %s", nClientId, pFile ) );
			// don't actually delete the file, just hide the file
			if ( FileDisk::hideFile( sLocalFile, true ) )
			{
				// turn off the read-only flag if on..
				FileDisk::lockFile( sLocalFile, false );
				// clear the current user if any
				putKeyValue( pFile, CURRENT_USER, "" );
				// remove the file from the catalog
				m_Catalog.remove( CharString( pFile ).lower() );

				// add the file to the deleted list
				int nDeletedCount = getKeyValue( DELETED, DELETED_COUNT, (dword)0 );
				putKeyValue( DELETED, DELETED_COUNT, nDeletedCount + 1 );
				putKeyValue( DELETED, CharString().format("Deleted%d", nDeletedCount), pFile );

				// store the deleted index into the file's version control
				putKeyValue( pFile, DELETED, nDeletedCount );
				// return success
				bDeleted = true;
			}
		}
	}
	else
		log("Invalid rights!");

	return bDeleted;
}

bool MirrorServer::undeleteFile( dword nClientId, const char * pFile )
{
	bool bUndeleted = false;

	if (! canDelete( nClientId ) )
		return false;

	CharString sLocalFile = m_Context.mirror + pFile;
	if (! FileDisk::fileExists( sLocalFile ) )
		log( "Deleted file doesn't exist!" );

	// unhide the file
	if (! FileDisk::hideFile( sLocalFile, false ) )
		log( "Failed to unhide the file!" );

	// clear the deleted slot in the control file
	int nDeletedIndex = getKeyValue( pFile,  DELETED, (dword)0 );
	if ( nDeletedIndex >= 0 )
	{
		putKeyValue( pFile, DELETED, (dword)-1 );
		putKeyValue( DELETED, CharString().format("Deleted%d", nDeletedIndex), "" );
		bUndeleted = true;

		log( CharString().format("Client %u undeleted file %s", nClientId, pFile) );
	}

	return bUndeleted;
}

dword MirrorServer::findLabel( const char * pLabel )
{
	int nLabelCount = getKeyValue( LABELS, LABEL_COUNT, (dword)0 );
	for(int i=0;i<nLabelCount;i++)
		if ( getKeyValue( LABELS, CharString().format("Label%d", i), "" ) ^= pLabel )
			return getKeyValue( LABELS, CharString().format("LabelTime%d", i), (dword)0);

	return 0;
}


bool MirrorServer::labelFiles( dword nClientId, const char * pLabel )
{
	if (! canLabel( nClientId ) )
		return false;

	int nLabelCount = getKeyValue( LABELS, LABEL_COUNT, (dword)0 );
	putKeyValue( LABELS, LABEL_COUNT, nLabelCount + 1 );
	putKeyValue( LABELS, CharString().format("Label%d", nLabelCount), pLabel );
	putKeyValue( LABELS, CharString().format("LabelTime%d", nLabelCount), Time::seconds() );

	return true;
}

bool MirrorServer::rollbackFiles( dword nClientId, const char * pLabel )
{
	if (! canRollback( nClientId ) )
		return false;
	dword nLabelTime = findLabel( pLabel );
	if ( nLabelTime == 0 )
		return false;		// failed to find label

	return rollbackFiles( nClientId, nLabelTime, m_Context.mirror + "*.*" );
}

bool MirrorServer::rollbackFiles( dword nClientId, dword nTime, const char * pMask )
{
	int i;

	CharString sBase = Path( pMask ).directory();

	// find all files in this directory, including hidden files..
	FindFile ff( pMask );
	for(i=0;i<ff.fileCount();i++)
	{
		CharString sLocalFile( sBase + ff.file( i ) );
		sLocalFile.lower();

		CharString sFile = sLocalFile;
		sFile.right( sFile.length() - m_Context.mirror.length() );

		rollbackFile( nClientId, nTime, sFile );
	}

	for(i=0;i<ff.directoryCount();i++)
	{
		CharString sDirectory( ff.directory( i ) );
		if ( sDirectory[0] != '.' )
			rollbackFiles( nClientId, nTime, sBase + sDirectory + "\\*.*" );	
	}

	return true;
}

bool MirrorServer::rollbackFile( dword nClientId, const char * pLabel, const char * pFile )
{
	if (! canRollback( nClientId ) )
		return false;

	dword nLabelTime = findLabel( pLabel );
	if ( nLabelTime == 0 )
		return false;		// failed to find label

	return rollbackFile( nClientId, nLabelTime, pFile );
}

bool MirrorServer::rollbackFile( dword nClientId, dword nTime, const char * pFile )
{
	if (! canRollback( nClientId ) )
		return false;
	CharString sLocalFile = m_Context.mirror + pFile;
	if (! FileDisk::fileExists( sLocalFile ) )
		return false;

	//log( CharString().format("MirrorServer::rollbackFile() - nTime = %s / %u, pFile = %s", Time::format( nTime, "%c" ), nTime, pFile) );

	bool bSuccess = false;
	if ( FileDisk::fileDate( sLocalFile ) > nTime )
	{
		int nVersion = getKeyValue( pFile,  LAST_VERSION, (dword)0 );
		while( nVersion >= 0 && !bSuccess )
		{
			CharString sVersionFile = sLocalFile + CharString().format(".%d", nVersion);
			dword nVersionDate = FileDisk::fileDate( sVersionFile );

			//log( CharString().format( "MirrorServer::rollbackFile() - %s -> %s / %u", sVersionFile, Time::format( nVersionDate, "%c" ), nVersionDate ) );
			if ( nVersionDate != 0 && nVersionDate <= nTime )
			{
				int nNextVersion = getKeyValue( pFile,  LAST_VERSION, (dword)0 );
				putKeyValue( pFile, LAST_VERSION, nNextVersion + 1 );
				putKeyValue( pFile, CURRENT_USER, "" );			// force the file to be unchecked

				CharString sNextVersion = sLocalFile + CharString().format(".%d", nNextVersion);
				
				// go ahead and version the current file
				dword nVersionTime = Time::seconds();
				FileDisk::lockFile( sLocalFile, false );
				if (! FileDisk::renameFile( sLocalFile, sNextVersion ) )
					log( CharString().format("Failed to rename %s to %s!", (const char *)sLocalFile, (const char *)sNextVersion) );

				appendHistory( pFile, CharString().format("%s : Version %d", 
					(const char *)clientUser( nClientId ), nNextVersion), nVersionTime );
				FileDisk::lockFile( sNextVersion, false );						// clear the read-only flag
				FileDisk::hideFile( sNextVersion, true );						// make sure the file is hidden
				FileDisk::touchFile( sNextVersion, nVersionTime );

				// make the older version the current file
				if (! FileDisk::copyFile( sVersionFile, sLocalFile ) )
				{
					log( CharString().format("Failed to copy %s to %s", 
						(const char *)sVersionFile, (const char *)sLocalFile) );
				}

				// set the time of the next version 1 second ahead in time, so we can rollback to the previous
				// version if needed
				nVersionTime++;

				FileDisk::lockFile( sLocalFile, false );						// clear the read-only flag
				FileDisk::hideFile( sLocalFile, false );						// unhide the file
				FileDisk::touchFile( sLocalFile, nVersionTime );				// set the time on the file

				appendHistory( pFile, CharString().format("%s : Rolled back to version %d", 
					(const char *)clientUser( nClientId ), nVersion), nVersionTime );

				// update the catalog
				Item newItem;
				newItem.name = CharString( pFile ).lower();
				newItem.crc = MirrorClient::checksum( sLocalFile );
				newItem.size = FileDisk::fileSize( sLocalFile );
				m_Catalog[ newItem.name ] = newItem;

				// send the file back to the client
				if ( nClientId != 0 )
					sendFile( nClientId, newItem );

				bSuccess = true;
			}
			else
			{
				// next version
				nVersion--;
			}
		}
	}
	else
	{
		// current version of the file is newer than the time provided... leave the file alone
		bSuccess = true;
	}

	return bSuccess;
}

// convert a string date into a time_t value... format is 06/30/04 01:33:24 
static dword StringTime( const char * pTime )
{
	tm t;

	// zero out the structure
	memset( &t, 0, sizeof(t) );
	t.tm_isdst = -1;

	// parse the parts of the date
	sscanf( pTime, "%d/%d/%d %d:%d:%d", &t.tm_mon, &t.tm_mday, 
		&t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec );

	// months are 0 based, so subtract 1
	t.tm_mon = t.tm_mon - 1;
	// adjust the year
	t.tm_year = t.tm_year + 2000 - 1900;

	return mktime( &t );
}

static int SortHistory( CharString t1, CharString t2 )
{
	return StringTime( t2 ) - StringTime( t1 );
}

void MirrorServer::sendHistory( dword nClientId, dword nJob, const char * pFile )
{
	Array< CharString > history;

	lock();

	log( CharString().format("Client %u requested history for file %s", nClientId, pFile) );
	if ( canUpload( nClientId ) )
	{
		CharString sLocalFile( m_Context.mirror + pFile );
		if ( FileDisk::fileExists( sLocalFile ) )
		{
			int nHistoryCount = getKeyValue( pFile,  HISTORY_COUNT, (dword)0 );
			for(int i=0;i<nHistoryCount;i++)
				history.push( getKeyValue( pFile,  CharString().format("History%d", i), "" ) );

			// get the labels, and append them into the history for this file
			int nLabelCount = getKeyValue( LABELS, LABEL_COUNT, (dword)0 );
			for(int k=0;k<nLabelCount;k++)
			{
				dword nLabelTime = getKeyValue( LABELS, CharString().format("LabelTime%d", k), (dword)0);

				CharString sLabel;
				sLabel.format( "%s : %s", (const char *)Time::format( nLabelTime, "%c" ), 
					(const char *)getKeyValue( LABELS, CharString().format("Label%d",k), "" ) );

				history.push( sLabel );
			}

			// sort history array by date now
			history.qsort( SortHistory );
		}
	}

	unlock();

	OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_HISTORY ) );
	output << nJob << history;
}

void MirrorServer::sendLockedFiles( dword nClientId, dword nJob )
{
	Array< MirrorClient::LockedFile > locked;

	lock();
	log( CharString().format("Client %u requesting locked files", nClientId) );

	if ( canDownload( nClientId ) )
	{
		// enumerate entire catalog... push locked files into the array to be sent onto the client
		CatalogIt it = m_Catalog.head();
		while( it.valid() )
		{
			MirrorClient::Item & item = *it;
			it++;

			if ( isFileLocked( 0, item.name ) )
			{
				MirrorClient::LockedFile file;
				file.file = item.name;
				file.user = getKeyValue( item.name, CURRENT_USER, "" );
				file.bLocked = file.user ^= clientUser( nClientId );

				locked.push( file );
			}
		}
	}
	unlock();

	OutStream output( send( nClientId, MirrorClient::CLIENT_RECV_LOCKED ) );
	output << nJob << locked;
}

//----------------------------------------------------------------------------

void MirrorServer::loadVersionControl()
{
	if (! m_Context.bVersionControl )
		return;

	char * pText = FileDisk::loadTextFile( m_Context.mirror + CONTROL_FILE );
	if ( pText == NULL )
		return;		// no file yet, just ignore the error

	lock();

	m_SectionTree.release();
	m_nSectionTreeModified = 0;

	int nSectionCount = 0;
	int nKeyCount = 0;

	char * pSection = NULL;
	char * pParse = pText;
	char * pEnd = pText + strlen( pText );
	while( pParse < pEnd )
	{
		// find the end of the line
		int nLength = strcspn( pParse, "\r\n" );
		if ( nLength > 0 && (pParse + nLength) <= pEnd )
		{
			// null terminate the line
			pParse[ nLength ] = 0;
			
			// is this a section definition
			if ( pParse[ 0 ] == '[' )
			{
				// find the ending ]
				char * pBracket = strchr( pParse, ']' );
				if ( pBracket != NULL )
				{
					pSection = pParse + 1;
					*pBracket = 0;

					nSectionCount++;
				}
			}
			else if ( pSection != NULL )
			{
				// find the = operator
				char * pEqual = strchr( pParse, '=' );
				if ( pEqual != NULL )
				{
					char * pKey = pParse;
					char * pValue = pEqual + 1;
					*pEqual = 0;

					m_SectionTree[ pSection ][ pKey ] = pValue;

					nKeyCount++;
				}
			}
		}
		
		// skip past the \r and \n characters
		pParse += nLength + 1;
	}

	// lastly, free the allocated memory
	Core::free( pText );

	log( CharString().format("Version control file loaded, %d sections, %d keys", nSectionCount, nKeyCount) );

	unlock();
}

void MirrorServer::saveVersionControl(bool bForce /*= false*/ )
{
	if (! m_Context.bVersionControl )
		return;
	if ( m_nSectionTreeModified == 0 )
		return;		// tree not modified
	if ( !bForce && ((m_nSectionTreeModified + 30) > Time::seconds()) )
		return;		// tree modified too recently, wait some time before trying to save...

	log( "Saving version control information..." );

	lock();

	try {
		FileDisk fd( m_Context.mirror + CONTROL_FILE, FileDisk::WRITE );

		SectionTree::Iterator section = m_SectionTree.head();
		while( section.valid() )
		{
			// write out the section to the file
			CharString sSection;
			sSection.format( "[%s]\r\n", (const char *)section.key() );

			fd.write( sSection, sSection.length() );

			KeyValueTree::Iterator key = (*section).head();
			while( key.valid() )
			{
				CharString sKeyValue;
				sKeyValue.format("%s=%s\r\n", (const char *)key.key(), (const char *)(*key));

				fd.write( sKeyValue, sKeyValue.length() );

				key.next();
			}

			section.next();
		}

		// done, close the file
		fd.close();

	}
	catch( FileDisk::FileError )
	{
		log( "Error occured while trying to save version control information to disk!" );
	}

	m_nSectionTreeModified = 0;

	unlock();
}

CharString MirrorServer::getKeyValue( const char * pSection, const char * pKey, const char * pDefault )
{
	KeyValueTree::Iterator it = m_SectionTree[ pSection ].find( pKey );
	if ( it.valid() )
		return *it;
	return pDefault;
}

dword MirrorServer::getKeyValue( const char * pSection, const char * pKey, dword nDefault )
{
	KeyValueTree::Iterator it = m_SectionTree[ pSection ].find( pKey );
	if ( it.valid() )
		return strtoul( *it, NULL, 10 );
	return nDefault;
}

void MirrorServer::putKeyValue( const char * pSection, const char * pKey, const char * pValue )
{
	m_SectionTree[ pSection ][ pKey ] = pValue;
	m_nSectionTreeModified = Time::seconds();
}

void MirrorServer::putKeyValue( const char * pSection, const char * pKey, dword nValue )
{
	m_SectionTree[ pSection ][ pKey ] = CharString().format("%u", nValue);
	m_nSectionTreeModified = Time::seconds();
}

//----------------------------------------------------------------------------
//EOF
