/**
	@file Dictionary.h
	@brief Dictionary object is attached to a Stream object, it is used by serialization functions
	as a storage object to reduce common data into a CompactNumber ID.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 1/13/2011 8:55:24 PM
*/

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "Standard/Types.h"
#include "Standard/Reference.h"
#include "Standard/CharString.h"
#include <map>

#include "MedusaDll.h"		// include last always

//---------------------------------------------------------------------------------------------------

class InStream;
class OutStream;

class DLL Dictionary : public Referenced
{
public:
	//! Types
	typedef Reference< Dictionary >			Ref;

	//! Base class for a storage object, so we can clear it up..
	class Storage : public Referenced
	{
	public:
		typedef Reference< Storage >			Ref;

		// delcare virtual destructor..
		virtual ~Storage() 
		{}
	};

	//! template class for a type storage object, this is what actually holds the data for a certain data type..
	template<typename T>
	class TypedStorage : public Storage
	{
	public:
		//! Construction
		TypedStorage();

		bool			findID( const T & a_Data, u32 & a_nID );	// find a ID given the given data, returns false if not found
		u32				addData( const T & a_Data );				// add data to this storge, returns the ID assigned
		T *				findData( u32 a_nID );						// returns NULL if data isn't found by the given ID
		void			clearData( u32 a_nID );

		void			clear();
		void			initDataMap();								// reinitialize the m_DataMap from m_IDMap

		//! Types
		typedef std::map< T, u32 >							IDMap;			// maps a given value to it's ID
		typedef std::map< u32, typename IDMap::iterator >	DataMap;		// maps an ID to the value

		//! Data
		u32				m_nNextID;
		IDMap			m_IDMap;
		DataMap			m_DataMap;
	};

	//! Construction
	Dictionary();
	
	//! Find storage by the given ID
	Storage *			findStorage( const char * a_pID ) const;
	void				setStorage( const char * a_pID, Storage * a_pStorage );
	void				clearStorage( const char * a_pID );

	//! Returns a TypeStorage object for the given type, will create and attached the storage if needed.
	template<typename T>
	TypedStorage<T> *	getStorage( bool bCreate = false );

protected:
	//! Types
	typedef std::map< CharString, Storage::Ref >		StorageMap;

	//! Data
	StorageMap		m_StorageMap;
};

//---------------------------------------------------------------------------------------------------

inline Dictionary::Storage * Dictionary::findStorage( const char * a_pID ) const
{
	StorageMap::const_iterator iStorage = m_StorageMap.find( a_pID );
	if ( iStorage != m_StorageMap.end() )
		return iStorage->second;
	return NULL;
}

inline void Dictionary::setStorage( const char * a_pID, Storage * a_pStorage )
{
	m_StorageMap[ a_pID ] = a_pStorage;
}

inline void	Dictionary::clearStorage( const char * a_pID )
{
	m_StorageMap.erase( a_pID );
}

template<typename T>
inline Dictionary::TypedStorage<T> * Dictionary::getStorage(bool bCreate /*= false*/ )
{
	StorageMap::iterator iStorage = m_StorageMap.find( typeid(T).name() );
	if ( iStorage != m_StorageMap.end() )
		return (TypedStorage<T> *)iStorage->second.pointer();

	if (! bCreate )
		return NULL;

	TypedStorage<T> * pStorage = new TypedStorage<T>();
	m_StorageMap[ typeid(T).name() ] = pStorage;

	return pStorage;
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline Dictionary::TypedStorage<T>::TypedStorage() : m_nNextID( 0 )
{}

template<typename T>
inline bool Dictionary::TypedStorage<T>::findID( const T & a_Data, u32 & a_nID )
{
	typename IDMap::iterator iData = m_IDMap.find( a_Data );
	if ( iData != m_IDMap.end() )
	{
		a_nID = iData->second;
		return true;
	}

	return false;
}

template<typename T>
inline u32 Dictionary::TypedStorage<T>::addData( const T & a_Data )
{
	u32 nID = m_nNextID++;
	typename IDMap::iterator iData = m_IDMap.insert( std::pair< T, u32 >( a_Data, nID ) ).first;
	m_DataMap[ nID ] = iData;

	return nID;
}

template<typename T>
inline T * Dictionary::TypedStorage<T>::findData( u32 a_nID )
{
	typename DataMap::iterator iData = m_DataMap.find( a_nID );
	if ( iData != m_DataMap.end() )
		return const_cast<T *>( &(iData->second->first) );

	return NULL;
}

template<typename T>
inline void Dictionary::TypedStorage<T>::clearData( u32 a_nID )
{
	typename DataMap::iterator iData = m_DataMap.find( a_nID );
	if ( iData != m_DataMap.end() )
	{
		m_IDMap.erase( iData->second );
		m_DataMap.erase( iData );
	}
}

template<typename T>
inline void Dictionary::TypedStorage<T>::clear()
{
	m_DataMap.clear();
	m_IDMap.clear();
	m_nNextID = 0;
}

template<typename T>
inline void Dictionary::TypedStorage<T>::initDataMap()
{
	m_DataMap.clear();
	for( typename IDMap::iterator iData = m_IDMap.begin(); iData != m_IDMap.end(); ++iData )
		m_DataMap[ iData->second ] = iData;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF
