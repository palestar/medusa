/*
	UserStorage.h

	This widget object is used to hold a players storage objects
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef USERSTORAGE_H
#define USERSTORAGE_H

#include "File/FileDisk.h"
#include "Factory/FactoryTypes.h"
#include "Factory/WidgetWrap.h"
#include "Noun.h"
#include "Group.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL UserStorage : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	struct Storage
	{
		// Construction
		Storage();
		Storage &				operator=( const Storage & copy );

		// Data
		int						nType;							// application specific type
		dword					nFactionId;						// application specific faction data..
		dword					nTime;							// time when stored
		Noun::wRef				pLocation;						// if not NULL, then location of this item
		CharString				sServer;						// server this item is located on..
		int						nPort;
		WidgetWrap				instance;						// instance for this object
	};

	typedef Reference< UserStorage >		Ref;
	typedef HashArray< WidgetKey, Storage >	StorageHash;

	// Construction
	UserStorage();
	UserStorage( dword nUserId );
	virtual ~UserStorage();

	// Accessors
	const CharString &	filename() const;
	FileDisk *			fileDisk() const;
	dword				userId() const;
	bool				isValid( const WidgetKey & nKey ) const;
	int					storageCount() const;
	WidgetKey			storageKey( int n ) const;
	const Storage &		storage( int n ) const;
	const Storage &		storageByKey( const WidgetKey & nKey ) const;
	Storage *			findStorage( const WidgetKey & nKey ) const;

	int					bytes() const;
	Group *				group() const;

	// Mutators
	void				postReceive();											// this should be invoked after this object is received from the server
	bool				load( const char * a_pFileName, dword a_nUserID );		// locks and loads the user storage from disk	
	bool				save(bool a_bCloseFile);								// saves the storage to disk and unlocks if done

	StorageHash &		getStorageHash();
	Storage &			getStorage( int i );
	bool				add( const WidgetKey & nKey, const Storage & copy );
	bool				remove( const WidgetKey & nKey );

	void				setGroup( Group * a_pGroup );

private:
	// Data
	CharString			m_sFileName;		// filename of the storage file
	dword				m_nUserId;
	StorageHash			m_Storage;
	Group::Ref			m_pGroup;			// users current group, if any...

	// non-serialized
	FileDisk::Ref		m_pFileDisk;		// our open handle to the file, if this is NULL then we have no lock
};

//----------------------------------------------------------------------------

inline const CharString & UserStorage::filename() const
{
	return m_sFileName;
}

inline FileDisk * UserStorage::fileDisk() const
{
	return m_pFileDisk;
}

inline dword UserStorage::userId() const
{
	return m_nUserId;
}

inline bool UserStorage::isValid( const WidgetKey & nKey ) const
{
	return m_Storage.find( nKey ).valid();
}

inline int UserStorage::storageCount() const
{
	return m_Storage.size();
}

inline WidgetKey UserStorage::storageKey( int n ) const
{
	return m_Storage.key( n );
}

inline const UserStorage::Storage & UserStorage::storage( int n ) const
{
	return m_Storage[ n ];
}

inline const UserStorage::Storage & UserStorage::storageByKey( const WidgetKey & nKey ) const
{
	return m_Storage.hash()[ nKey ];
}

inline UserStorage::Storage * UserStorage::findStorage( const WidgetKey & nKey ) const
{
	StorageHash::Iterator iFind = m_Storage.hash().find( nKey );
	if (! iFind.valid() )
		return NULL;

	return const_cast<UserStorage::Storage *>( &(*iFind) );
}

inline int UserStorage::bytes() const
{
	int nBytes = 0;
	for(int i=0;i<m_Storage.size();i++)
		nBytes += m_Storage[ i ].instance.size();

	return nBytes;
}

inline Group * UserStorage::group() const
{
	return m_pGroup;
}

//---------------------------------------------------------------------------------------------------

inline UserStorage::StorageHash & UserStorage::getStorageHash()
{
	return m_Storage;
}

inline UserStorage::Storage & UserStorage::getStorage( int i )
{
	return m_Storage[ i ];
}

inline bool UserStorage::add( const WidgetKey & nKey, const Storage & copy )
{
	m_Storage.insert( nKey, copy );
	return true;
}

inline bool UserStorage::remove( const WidgetKey & nKey )
{
	return m_Storage.removeByKey( nKey );
}

inline void UserStorage::setGroup( Group * a_pGroup )
{
	m_pGroup = a_pGroup;
}

//---------------------------------------------------------------------------------------------------

inline UserStorage::Storage::Storage() : nType( 0 ), nFactionId( 0 ), nTime( 0 ), nPort( 0 )
{}

inline UserStorage::Storage & UserStorage::Storage::operator=( const Storage & copy )
{
	nType = copy.nType;
	nFactionId = copy.nFactionId;
	nTime = copy.nTime;
	pLocation = copy.pLocation;
	sServer = copy.sServer;
	nPort = copy.nPort;
	instance = copy.instance;
	return *this;
}

const int STORAGE_VERSION_1 = 20080929;
const int STORAGE_VERSION_2 = 20090928;
const int STORAGE_VERSION_3 = 20100705;

inline const InStream & operator>>( const InStream & input, UserStorage::Storage & storage )
{
	int nVersion;
	input >> nVersion;

	switch( nVersion )
	{
	case STORAGE_VERSION_3:
		input >> storage.nType;
		input >> storage.nFactionId;
		input >> storage.nTime;
		input >> storage.pLocation;
		input >> storage.sServer;
		input >> storage.nPort;
		input >> storage.instance;
		break;
	case STORAGE_VERSION_2:
		input >> storage.nType;
		input >> storage.nFactionId;
		input >> storage.pLocation;
		input >> storage.sServer;
		input >> storage.nPort;
		input >> storage.instance;
		break;
	case STORAGE_VERSION_1:
		input >> storage.nType;
		input >> storage.nFactionId;
		input >> storage.pLocation;
		input >> storage.instance;
		break;
	default:
		storage.nType = nVersion;
		input >> storage.pLocation;
		input >> storage.instance;
		break;
	}
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const UserStorage::Storage & storage )
{
	output << STORAGE_VERSION_3;
	output << storage.nType;
	output << storage.nFactionId;
	output << storage.nTime;
	output << storage.pLocation;
	output << storage.sServer;
	output << storage.nPort;
	output << storage.instance;
	return output;
}


//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
