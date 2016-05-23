/*
	WorldServerStorage.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "WorldServer.h"
#include "UserStorage.h"
#include "File/FileDisk.h"
#include "File/FindFile.h"

//----------------------------------------------------------------------------

// This function assumes the server is locked
bool WorldServer::inStorage( dword nUserId, const WidgetKey & nKey ) const
{
	UserStorageHash::const_iterator iStorage = m_UserStorage.find( nUserId );
	if ( iStorage == m_UserStorage.end() )
		return false;
	return iStorage->second->isValid( nKey );
}

UserStorage * WorldServer::getStorage( dword nUserId ) const
{
	UserStorageHash::const_iterator iStorage = m_UserStorage.find( nUserId );
	if ( iStorage == m_UserStorage.end() )
		return NULL;
	return iStorage->second;
}

UserStorage::Storage * WorldServer::findStorage( dword nUserId, const WidgetKey & nKey ) const
{
	UserStorage * pUserStorage = getStorage( nUserId );
	if (! pUserStorage )
		return NULL;
	
	return pUserStorage->findStorage( nKey );
}

Noun::Ref WorldServer::getStorageNoun( dword nUserId, const WidgetKey & nKey ) const
{
	UserStorage * pStorage = getStorage( nUserId );
	if (! pStorage )
		return Noun::Ref();
	UserStorage::StorageHash & hash = pStorage->getStorageHash();
	UserStorage::StorageHash::Iterator iStorage = hash.find( nKey );
	if (! iStorage.valid() )
		return Noun::Ref();
	Storage & storage = *iStorage;
	Widget::Ref pUncasted = storage.instance.unwrap();
	return Noun::Ref( WidgetCast<Noun>( pUncasted ) );
}

dword WorldServer::getStorageFactionId( dword nUserId, const WidgetKey & nKey ) const
{
	UserStorage * pStorage = getStorage( nUserId );
	if (! pStorage )
		return 0;
	UserStorage::StorageHash & hash = pStorage->getStorageHash();
	UserStorage::StorageHash::Iterator iStorage = hash.find( nKey );
	if (! iStorage.valid() )
		return 0;

	Storage & storage = *iStorage;
	return storage.nFactionId;
}

Noun * WorldServer::getStorageLocation( dword nUserId, const WidgetKey & nKey ) const
{
	UserStorage * pStorage = getStorage( nUserId );
	if (! pStorage )
		return NULL;
	UserStorage::StorageHash & hash = pStorage->getStorageHash();
	UserStorage::StorageHash::Iterator iStorage = hash.find( nKey );
	if (! iStorage.valid() )
		return NULL;
	Storage & storage = *iStorage;
	return storage.pLocation;
}

//----------------------------------------------------------------------------

bool WorldServer::loadStorage( ClientContext & context )
{
	if (! m_Context.bPersistant )
	{
		m_UserStorage[ context.nUserId ] = new UserStorage( context.nUserId );
		return true;
	}
	if ( m_UserStorage.find( context.nUserId ) != m_UserStorage.end() )
		return true;		// storage is already loaded...

	bool bLoaded = false;

	CharString sStorageFileName;
	sStorageFileName.format( "%s%8.8u.wob", m_Context.sStorage.cstr(), context.nUserId );

	UserStorage::Ref pStorage = new UserStorage();
	if (! pStorage->load( sStorageFileName, context.nUserId ) )
	{
		LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to load or lock user %u storage!", context.nUserId) );
		return false;
	}
	if (! onStorageLoaded( context, pStorage ) )
	{
		LOG_ERROR( "WorldServer", "onStorageLoaded() returned false, rejecting storage load.." );
		return false;
	}
	LOG_STATUS( "WorldServer", "Loaded user %u storage, count = %d, bytes = %d", 
		context.nUserId, pStorage->storageCount(), pStorage->bytes() );
	m_UserStorage[ context.nUserId ] = pStorage;

	return true;
}

bool WorldServer::onStorageLoaded( ClientContext & context, UserStorage * a_pStorage )
{
	return true;
}

bool WorldServer::closeStorage( dword nUserId )
{
	if ( m_Context.bPersistant )
	{
		UserStorageHash::iterator iStorage = m_UserStorage.find( nUserId );
		if ( iStorage != m_UserStorage.end() )
		{
			UserStorage::Ref pStorage = iStorage->second;

			LOG_STATUS( "WorldServer", CharString().format("Closing user %u storage, count = %d, bytes = %d", 
				nUserId, pStorage->storageCount(), pStorage->bytes())  );

			if (! pStorage->save( true ) )
			{
				LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to save user %u storage", nUserId) );
				return false;
			}
		}
	}
	m_UserStorage.erase( nUserId );
	return true;
}

bool WorldServer::clearStorage()
{
	if (! m_Context.bPersistant )
		return false;

	m_UserStorage.clear();

	FindFile ff;
	ff.findFiles( m_Context.sStorage + "*.wob" );

	for(int i=0;i<ff.fileCount();i++)
		FileDisk::deleteFile( m_Context.sStorage + ff.file(i) );

	return true;
}

bool WorldServer::flushStorage( bool a_bForce )
{
	for( UserStorageHash::iterator iStorage = m_UserStorage.begin(); iStorage != m_UserStorage.end(); )
	{
		dword nUserId = iStorage->first;
		++iStorage;

		if (!a_bForce && m_UserClientHash.find( nUserId ) != m_UserClientHash.end() )
			continue;

		if (! closeStorage( nUserId ) )
		{
			LOG_ERROR( "WorldServer", CharString().format("ERROR: Failed to close user storge for user %u!", nUserId) );
			// TODO: not much choice, we really can't abort a shutdown... so this would be bad
			// if this condition happens..
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//EOF
