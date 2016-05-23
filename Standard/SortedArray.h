/*
	SortedArray.h

	This template class is used to keep an sorted array of objects. 
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SORTED_ARRAY_H
#define SORTED_ARRAY_H

#include "Standard/Array.h"

//----------------------------------------------------------------------------

/*
	Note: class T must provide the following operators,

	operator=
	operator>
	operator==
*/

template<class T> 
class SortedArray
{
	public:
		// Exceptions
		class		OutOfRange {};

		// Construction
					SortedArray();
					SortedArray( const SortedArray &copy );
		virtual		~SortedArray();

		// Accessors
		const T &	operator[](int n) const;			// accesses the array by index

		int			find( const T & object ) const;		// returns the index of the found item, returns -1 if not found
		int			size() const;						// size of the array

		const Array< T > &
					array() const;						// get the embedded array object

		// Mutators
		int			insert( const T & object );			// inserts an object into the array
		void		removeIndexed( int n );				// removes an item by index
		void		remove( const T & object );			// remove an item

		void		release();

		SortedArray &		
					operator=(const SortedArray & copy );		// assignment operator

	private:
		// Data
		Array< T >
					m_Array;
};

//-------------------------------------------------------------------------------

template<class T>
inline SortedArray<T>::SortedArray()
{}

template<class T>
inline SortedArray<T>::SortedArray( const SortedArray &copy ) : m_Array( copy.m_Array )
{}

template<class T>
inline SortedArray<T>::~SortedArray()
{}

//-------------------------------------------------------------------------------

template<class T>
inline const T & SortedArray<T>::operator[](int n) const			
{ 
	return( m_Array[n] ); 
}

template<class T>
inline int SortedArray<T>::find( const T & object ) const
{
	// standard binary-search
	int lo = 0;
	int hi = m_Array.size();

	while( lo < hi )
	{
		int mid = (lo + hi) >> 1;
		
		if ( m_Array[ mid ] == object )
			return mid;
		else if ( m_Array[ mid ] > object )
			hi = mid;
		else
			lo = mid + 1;
	}

	// item not found, return invalid index
	return -1;
}

template<class T>
inline int SortedArray<T>::size() const
{
	return( m_Array.size() );
}

template<class T>
inline const Array< T > & SortedArray<T>::array() const
{
	return m_Array;
}

//-------------------------------------------------------------------------------

template<class T>
inline int SortedArray<T>::insert( const T & object )
{
	// standard binary-search
	int lo = 0;
	int hi = m_Array.size();

	while( lo < hi )
	{
		int mid = (lo + hi) >> 1;
		
		if ( m_Array[ mid ] == object )
			return -1;		// object already in the array
		else if ( m_Array[ mid ] > object )
			hi = mid;		// look lower
		else
			lo = mid + 1;	// look higher
	}
	
	m_Array.insert( lo, object );
	return( lo );
}

template<class T>
inline void SortedArray<T>::removeIndexed( int n )
{
	m_Array.remove( n );
}

template<class T>
inline void SortedArray<T>::remove( const T & object )
{
	int index = find( object );
	if ( index >= 0 )
		removeIndexed( index );
}

template<class T>
inline void SortedArray<T>::release()
{
	m_Array.release();
}

template<class T>
inline SortedArray<T> & SortedArray<T>::operator=(const SortedArray<T> & copy )
{
	m_Array = copy.m_Array;
	return *this;
}


//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
