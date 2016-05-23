/*
	HashArray.h

	This template class is access like an array, however it uses both the Array<> and Hash<> 
	template classes to hold the data allowing for quick sequential access of the hash elements.

	- Best used when you need to easily enumerate all the data, and still have the ability 
	to quickly search for an object by key.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef HASHARRAY_H
#define HASHARRAY_H

#include "Standard/Array.h"
#include "Standard/Hash.h"



//----------------------------------------------------------------------------

// K is for the key
// O is the object contained

template<class K,class O, unsigned int tableSize = 32> 
class HashArray
{
	public:
		typedef typename Hash<K,O,tableSize>::Iterator		
						Iterator;

		// Accessor
		int				size() const;						// get total number of elements
		K				key( int n ) const;					// get key n

		bool			isValid( int n ) const;				// validate element n
		const O &		operator[]( int n ) const;			// get element n

		Iterator		find( const K &key ) const;			// find leaf, returns 0 if leaf not found

		const Array< K > &	
						array() const;						// get the array object
		const Hash< K, O, tableSize > &
						hash() const;						// get the hash object

		// Mutators
		O &				operator[]( int n );				// get element n

		bool			insert( const K & key,
							const O & object );				// add new element by key, overwrites if key is already in array and returns false, otherwise returns true
		O &				insert( const K & key );			// add new element by key, returns existing item if it already exists
		bool			remove( int n );					// remove object n
		bool			removeByKey( const K & key );		// remove object by key, slower than remove() because array must be searched for the key

		void			release();							// release all data

		// needed for serialization
		Array< K > &	array();							// get the array object

		Hash< K, O, tableSize > &
						hash();								// get the hash object


	private:
		// Data
		Array< K >					
						m_Array;
		Hash< K, O, tableSize >		
						m_Hash;
};

//----------------------------------------------------------------------------

template<class K,class O, unsigned int tableSize>
inline int HashArray<K,O,tableSize>::size() const
{
	return m_Array.size();
}

template<class K,class O, unsigned int tableSize>
inline K HashArray<K,O,tableSize>::key( int n ) const
{
	return m_Array[ n ];
}

template<class K,class O, unsigned int tableSize>
inline bool HashArray<K,O,tableSize>::isValid( int n ) const
{
	return m_Array.isValid( n );
}

template<class K,class O, unsigned int tableSize>
inline const O & HashArray<K,O,tableSize>::operator[]( int n ) const
{
	return m_Hash[ m_Array[ n ] ];
}

template<class K,class O, unsigned int tableSize>
inline typename HashArray<K,O,tableSize>::Iterator HashArray<K,O,tableSize>::find( const K &key ) const
{
	return m_Hash.find( key );
}

template<class K,class O, unsigned int tableSize>
inline const Array< K > & HashArray<K,O,tableSize>::array() const
{
	return m_Array;
}

template<class K,class O, unsigned int tableSize>
inline const Hash< K, O, tableSize > & HashArray<K,O,tableSize>::hash() const
{
	return m_Hash;
}

template<class K,class O, unsigned int tableSize>
inline O & HashArray<K,O,tableSize>::operator[]( int n )
{
	return m_Hash[ m_Array[ n ] ];
}

template<class K,class O, unsigned int tableSize>
inline bool HashArray<K,O,tableSize>::insert( const K & key, const O & object )
{
	Iterator replace = m_Hash.find( key );
	if ( replace.valid() )
	{
		(*replace) = object;		// update existing element
		return false;
	}

	// new element
	m_Array.push( key );
	m_Hash.insert( key, object );
	return true;
}

template<class K,class O, unsigned int tableSize>
inline O & HashArray<K,O,tableSize>::insert( const K & key )
{
	Iterator iReplace = m_Hash.find( key );
	if ( iReplace.valid() )
		return *iReplace;

	// new element
	m_Array.push( key );
	return m_Hash[ key ];
}

template<class K,class O, unsigned int tableSize>
inline bool HashArray<K,O,tableSize>::remove( int n )
{
	if( m_Array.isValid( n ) && m_Hash.remove( m_Array[ n ] ) )
	{
		m_Array.removeSwap( n );
		return true;
	}

	// failed to find the element
	return false;
}

template<class K,class O, unsigned int tableSize>
inline bool HashArray<K,O,tableSize>::removeByKey( const K & key )
{
	if ( m_Hash.remove( key ) )
	{
		m_Array.removeSearchSwap( key );
		return true;
	}

	return false;
}

template<class K,class O, unsigned int tableSize>
inline void HashArray<K,O,tableSize>::release()
{
	m_Array.release();
	m_Hash.release();
}

template<class K,class O, unsigned int tableSize>
inline Array< K > & HashArray<K,O,tableSize>::array() 
{
	return m_Array;
}

template<class K,class O, unsigned int tableSize>
inline Hash< K, O, tableSize > & HashArray<K,O,tableSize>::hash() 
{
	return m_Hash;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
