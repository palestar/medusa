/*
	Array.h

	Growable Array template class. The contents of this Array<> class are stored in blocks of arrays, this allows
	growing the array without having to copy the previous contents into a new array. However, because it doesn't
	store the contents in a contiguous block you have to call the buffer() function to get the data contained
	in a contigous buffer.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>		// NULL

//----------------------------------------------------------------------------

template<class T >		
class Array
{
	public:
		// Construction
		Array();
		Array( int size );
		Array( int size, const T * array );
		Array( const Array &copy );

		virtual	~Array();

		// Accessors
		const T &	operator[](int n) const;			// get const item n
		T &			operator[](int n);					// get non-const item n

		int			size() const;						// size of the array
		int			byteSize() const;					// how many bytes are contained in this array
		int			blockCount() const;					// how many blocks
		T *			buffer() const;						// allocates and returns a linear buffer for this array

		const T &	last() const;						// return reference to last item in array
		int			search( const T &object) const;		// linear search

		int			copy( T * pBuffer ) const;			// copy contents to a linear buffer, returns the number of bytes copied

		bool		isValid( int n ) const;				// is this a valid index

		// Mutators
		T &			push();
		T &			push( const T &object );
		T &			pushUnique( const T &object );		// performs a search, and only pushes the object
														// if it's not already there
		void		pop();								// remove the last item in the array

		T &			insert( int n, const T &object );	// insert before n into array
		void		swap( int n, int m );				// swap item n with item m
		void		move( int n, int m );				// move item n before item m, shifting other items down in aray
		void		remove( int n );					// remove indexed item from the array
		void		removeSwap( int n );				// remove item and replace it with the last item in the array (faster)
		bool		removeSearch( const T &object);		// find item, then remove it from the array
		bool		removeSearchSwap( const T &object);	// find item, then remove it from the array using removeSwap()
		void		release();							// release the entire array

		void		allocate( int size );				// set the size, destroy's existing contents
		void		allocate( int size, const T * array );			
		void		realloc( int nSize );				// set the new size of the array, keeps any existing values
		void		grow( int size );					// increase the size
		void		shrink( int size );					// decrease the size

		void		qsort();							// quick sort the array
		void		qsort( int (* pCompare)(T e1, T e2) );	// quick sort this array using the provided compare function

		Array &		operator=(const Array & copy );

	private:
		// Types
		enum {
			BLOCK_SHIFT = 7,
			BLOCK_SIZE = 1 << BLOCK_SHIFT,
			BLOCK_MASK = BLOCK_SIZE - 1,
		};

		// Data
		int			m_Size;								// array size
		int			m_BlockCount;						// number of blocks allocated
		T **		m_pBlocks;							// the array data

		// Mutators
		void		rqsort( int d, int h, int (* pCompare)(T e1, T e2) );
		void		growArray( int newSize );
		void		cullArray();

		// Helpers
		static int	DefaultCompare( T e1, T e2 );
};

//-------------------------------------------------------------------------------

template<class T >
inline Array<T>::Array() : m_pBlocks( NULL ), m_Size(0), m_BlockCount( 0 )
{
}

template<class T>
inline Array<T>::Array( int size ) : m_pBlocks( NULL ), m_Size(0), m_BlockCount( 0 )
{
	allocate( size );
}

template<class T>
inline Array<T>::Array( int size, const T * pArray) : m_pBlocks( NULL ), m_Size(0), m_BlockCount( 0 )
{
	allocate( size, pArray );
}

template<class T>
inline Array<T>::Array( const Array &copy )  : m_pBlocks( NULL ), m_Size(0), m_BlockCount( 0 )
{
	allocate( copy.size() );
	for(int i=0;i<copy.size();i++)
		(*this)[i] = copy[i];
}

template<class T>
inline Array<T>::~Array()
{
	release();
}

//-------------------------------------------------------------------------------

template<class T>
inline const T & Array<T>::operator[](int n) const
{ 
	return m_pBlocks[ n >> BLOCK_SHIFT ][ n & BLOCK_MASK ]; 
}

template<class T>
inline T & Array<T>::operator[](int n)			
{ 
	return m_pBlocks[ n >> BLOCK_SHIFT ][ n & BLOCK_MASK ]; 
}

template<class T>
inline int Array<T>::size() const
{ 
	return m_Size; 
}

template<class T>
inline int Array<T>::byteSize() const
{ 
	return m_Size * sizeof(T); 
}

template<class T>
inline int Array<T>::blockCount() const
{
	return m_BlockCount;
}

template<class T>
inline T * Array<T>::buffer() const
{
	T * pBuffer = new T[ m_Size ];
	copy( pBuffer );

	return pBuffer;
}

template<class T>
inline const T & Array<T>::last() const						
{ 
	return (*this)[ m_Size - 1];
}

template<class T>
inline int Array<T>::search( const T &object ) const
{
	for(int i=0;i<m_Size;i++)
		if ( (*this)[i] == object)
			return( i );

	return( -1 );
}

template<class T>
inline int Array<T>::copy( T * pBuffer ) const
{
	for(int i=0;i<m_Size;i++)
		pBuffer[i] = (*this)[i];

	return m_Size * sizeof(T);
}

template<class T>
inline bool Array<T>::isValid( int n ) const
{
	return n >= 0 && n < m_Size;
}

//-------------------------------------------------------------------------------

template<class T>
inline T & Array<T>::push()
{
	growArray( m_Size + 1 );
	m_Size++;

	return (*this)[ m_Size - 1 ];
}

template<class T>
inline T & Array<T>::push( const T &object )
{
	T & newObject = push();
	newObject = object;

	return newObject;
}

template<class T>
inline T & Array<T>::pushUnique( const T &object )
{
	int found = search( object );
	if ( found >= 0 )
		return (*this)[ found ];

	T & newObject = push();
	newObject = object;

	return newObject;
}

template<class T>
inline void	Array<T>::pop()								
{ 
	remove( m_Size - 1); 
}

template<class T>
inline T & Array<T>::insert( int n, const T &object )
{
	growArray( m_Size + 1 );

	for(int i=m_Size;i>n;i--)
		(*this)[ i ] = (*this)[ i - 1 ];

	T & newObject = (*this)[ n ];
	newObject = object;
	m_Size++;

	return newObject;
}

template<class T>
inline void Array<T>::swap( int n, int m )
{
	T swap = (*this)[ n ];
	(*this)[ n ] = (*this)[ m ];
	(*this)[ m ] = swap;
}

template<class T>
inline void Array<T>::move( int n, int m )
{
	T move = (*this)[ n ];
	// remove item n from array
	remove( n );
	// insert item into array before m
	insert( m, move );
}

template<class T>
inline void Array<T>::remove( int n )
{
	for(int i=n;i<(m_Size - 1);i++)
		(*this)[i] = (*this)[i + 1];

	m_Size--;
	(*this)[ m_Size ] = T();

	cullArray();
}

template<class T>
inline void Array<T>::removeSwap( int n )
{
	if ( n < (m_Size - 1))
		(*this)[ n ] = (*this)[ m_Size - 1];

	m_Size--;
	(*this)[ m_Size ] = T();

	cullArray();
}

template<class T>
inline bool Array<T>::removeSearch( const T &object)
{
	int foundIndex = search( object );
	if ( foundIndex >= 0 )
	{
		remove( foundIndex );
		return true;
	}

	return false;
}

template<class T>
inline bool Array<T>::removeSearchSwap( const T &object)
{
	int foundIndex = search( object );
	if ( foundIndex >= 0 )
	{
		removeSwap( foundIndex );
		return true;
	}

	return false;
}

template<class T>
inline void Array<T>::release()
{
	if ( m_pBlocks != NULL )
	{
		for(int i=0;i<blockCount();i++)
			delete [] m_pBlocks[i];
		
		delete [] m_pBlocks;
		m_pBlocks = NULL;
	}
	m_Size = 0;
	m_BlockCount = 0;
}

template<class T>
inline void Array<T>::allocate( int size )
{
	release();

	growArray( size );
	m_Size = size;
}

template<class T>
inline void Array<T>::allocate( int size, const T * array )
{
	release();

	growArray( size );
	m_Size = size;

	for(int i=0;i<m_Size;i++)
		(*this)[ i ] = array[ i ];
}

template<class T>
inline void Array<T>::realloc( int nSize )
{
	grow( nSize - m_Size );
}

template<class T>
inline void Array<T>::grow( int size )
{
	if ( size > 0 )
	{
		growArray( m_Size + size );
		m_Size += size;
	}
	else if ( size < 0 )
	{
		// shrink the array in-fact
		shrink( -size );
	}
}

template<class T>
inline void Array<T>::shrink( int size )
{
	if ( size > 0 )
	{
		m_Size -= size;
		cullArray();
	}
	else if ( size < 0 )
		grow( -size );
}

template<class T>
inline void Array<T>::qsort()
{
	qsort( DefaultCompare );
}

template<class T>
inline void	Array<T>::qsort( int (* pCompare)(T e1, T e2) )
{
	if ( m_Size > 1 )
	{
		int low = 0;
		int high = m_Size - 1;

		rqsort( low, high, pCompare );
	}
}

template<class T>
inline Array<T> & Array<T>::operator=(const Array<T> & copy )
{
	allocate( copy.m_Size );
	for(int i=0;i<m_Size;i++)
		(*this)[i] = copy[i];

	return *this;
}

//-------------------------------------------------------------------------------

template<class T>
void Array<T>::rqsort( int d, int h, int (* pCompare)(T e1, T e2) )
{
	int i = h;
	int j = d;

	T str = operator[]( (d + h) / 2 );

	do {

		while( pCompare( operator[]( j ), str ) < 0 )
			j++;
		while( pCompare( operator[]( i ), str ) > 0 )
			i--;

		if ( i >= j ) 
		{
			if ( i != j ) 
			{
				T swap = operator[]( i );
				operator[]( i ) = operator[]( j );
				operator[]( j ) = swap;
			}

			i--;
			j++;
		}

	} while (j <= i);

	if (d < i) 
		rqsort( d, i, pCompare );
	if (j < h) 
		rqsort( j, h, pCompare );
}

template<class T>
void Array<T>::growArray( int newSize )
{
	int blocksNeeded = (newSize + (BLOCK_SIZE - 1)) / BLOCK_SIZE;

	if ( blocksNeeded > m_BlockCount )
	{
		T ** newBlocks = new T*[ blocksNeeded ];

		// copy over old blocks pointers
		int i = 0;
		for(;i<m_BlockCount;i++)
			newBlocks[i] = m_pBlocks[i];
		// allocate any new blocks
		for(i=m_BlockCount;i<blocksNeeded;i++)
			newBlocks[i] = new T[ BLOCK_SIZE ];

		delete [] m_pBlocks;
		m_pBlocks = newBlocks;
		m_BlockCount = blocksNeeded;
	}
}

template<class T>
void Array<T>::cullArray()
{
	int blocksNeeded = (m_Size + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
	if ( blocksNeeded < m_BlockCount )
	{
		if ( blocksNeeded > 0 )
		{
			T ** newBlocks = new T*[ blocksNeeded ];

			// copy over old block pointers
			int i =0;
			for(;i<blocksNeeded;i++)
				newBlocks[i] = m_pBlocks[i];
			// free up no longer needed blocks
			for(i=blocksNeeded;i<m_BlockCount;i++)
				delete [] m_pBlocks[i];

			delete [] m_pBlocks;
			m_pBlocks = newBlocks;
			m_BlockCount = blocksNeeded;
		}
		else
			release();
	}
}

template<class T>
int Array<T>::DefaultCompare( T e1, T e2 )
{
	return e1 - e2;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
