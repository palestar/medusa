/*
	UserStorage.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/


#include "UserStorage.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( UserStorage, Widget );

BEGIN_PROPERTY_LIST( UserStorage, Widget )
	ADD_PROPERTY( m_sFileName );
	ADD_TRANSMIT_PROPERTY( m_nUserId );
	ADD_TRANSMIT_PROPERTY( m_Storage );
	ADD_TRANSMIT_PROPERTY( m_pGroup );
END_PROPERTY_LIST();

UserStorage::UserStorage() : m_nUserId( 0 )
{}

UserStorage::UserStorage( dword nUserId ) : m_nUserId( nUserId )
{}

UserStorage::~UserStorage()
{
	save(true);
}

//---------------------------------------------------------------------------------------------------

void UserStorage::postReceive()
{
	// clear any dictionary references from the instances, they are saved/loaded without any dictionaries..
	for(int i=0;i<storageCount();++i)
		getStorage(i).instance.clearDictionary();
}

bool UserStorage::load( const char * a_pFileName, dword a_nUserID )
{
	if ( m_pFileDisk ) 
		return false;

	bool bLoaded = false;

	m_sFileName = a_pFileName;
	m_nUserId = a_nUserID;

	FileDisk::Ref pFile = new FileDisk();
	if (! pFile->open( m_sFileName, FileDisk::READ_WRITE ) )
		return false;

	if ( pFile->size() > 0 )
	{
		bool bLoaded = false;
		try {
			InStream input( pFile );

			int nVersion = 0;
			input >> nVersion;

			// validate the version number...
			switch( nVersion )
			{
			case SV_20090104:
				input.setVersion( nVersion );
				break;
			default:
				// file that is prior to the stream versions...
				input.setVersion( 0 );
				input.setPosition( 0 );
				break;
			}
			input >> *this;
			m_nUserId = a_nUserID;

			bLoaded = true;
		}
		catch( FileDisk::FileError )
		{}

		if (! bLoaded )
		{
			TRACE( CharString().format( "UserStorage::load() - Failed to load %s", a_pFileName ) );
			return false;
		}
	}

	m_pFileDisk = pFile;
	return true;
}

bool UserStorage::save(bool a_bCloseFile)
{
	if (! m_pFileDisk && m_nUserId != 0 )
	{
		// attempt to re-lock the file so we can save..
		FileDisk::Ref pFile = new FileDisk();
		if (! pFile->open( m_sFileName, FileDisk::READ_WRITE ) )
		{
			LOG_ERROR( "UserStorage", "Failed to open user storage %s for read/write.", m_sFileName.cstr() );
			return false;
		}
		m_pFileDisk = pFile;
	}

	if (! m_pFileDisk )
		return false;

	LOG_STATUS( "UserStorage", "Saving active user %u storage, count = %d, bytes = %d, Closing = %s", 
		m_nUserId, storageCount(), bytes(), a_bCloseFile ? "Yes" : "No" );

	m_pFileDisk->setPosition( 0 );

	bool bStored = false;
	try {
		int nVersion = SV_CURRENT;

		OutStream output( m_pFileDisk );
		output << nVersion;
		output << *this;

		bStored = true;
	}
	catch( FileDisk::FileError )
	{}

	if (! bStored )
	{
		LOG_ERROR( "UserStorage", "UserStorage::save() - Failed to save %s", m_sFileName.cstr() );
		return false;
	}

	if(a_bCloseFile)
	{
		m_sFileName = "";
		m_pFileDisk->setEOF();
		m_pFileDisk->close();
		m_pFileDisk = NULL;
		m_nUserId = 0;
		m_Storage.release();
	}

	return true;
}

//----------------------------------------------------------------------------
//EOF
