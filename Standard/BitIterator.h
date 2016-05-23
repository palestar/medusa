/*
	BitIterator.h
	BitIterator object used to read / write bits into a buffer. This is used by the binary serializer object.

	(c)2006 Palestar, Richard Lyle
*/

#ifndef BITITERATOR_H
#define BITITERATOR_H

#include <string.h>

#include "Types.h"
#include "String.h"
#include "Buffer.h"
#include "Color.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

#define BITCOUNT( TYPE )			(sizeof(TYPE) * 8)

// Handy macro to implement byte based streaming of a datatype...
#define IMPLEMENT_STREAMING( OBJECT )																	\
	inline BitIterator & operator>>( BitIterator & input, OBJECT & object )								\
	{																									\
		input.ReadBytes( &object, sizeof( OBJECT ) );													\
		return input;																					\
	}																									\
	inline BitIterator & operator<<( BitIterator & output, const OBJECT & object)						\
	{																									\
		output.WriteBytes( &object, sizeof( OBJECT ) );													\
		return output;																					\
	}

//---------------------------------------------------------------------------------------------------

class DLL BitIterator
{
public:
	// Construction
	BitIterator();
	BitIterator( void * pStart, void * pEnd, int nOffset = 0, int nAdd = 0 );
	BitIterator( void * pStart, int nSize, int nOffset = 0, int nAdd = 0 );
	BitIterator( const BitIterator & copy );

	// Accessors
	bool			IsValid() const;									// is the iterator at a valid position

	byte *			Start() const;
	byte *			End() const;
	byte *			It() const;											// get the current byte position
	int				Offset() const;									// get the current bit in the current byte

	Buffer			GetBuffer() const;								// get the bits from the start to the current position

	// Mutators
	bool			Set( void * pStart, void * pEnd, 
						int nOffset = 0, 
						int nAdd = 0 );								// initialize the bit iterator
	void			Rewind();											

	bool			SetIt( byte * pIt );								// sets the current byte position, must be between start and end
	void			SetOffset( int nOffset );						// sets the current bit in the current byte

	void			Inc();
	void			Add( unsigned int n );							// adds n bits to the current position
	void			Sub( unsigned int n );							// subtracts n bits

	int				Read( bool & bBit );								// gets 1 bit
	int				Read( void * pDst, int nBits,					// read n bits into buffer at pDst
						bool bAligned );								// if bAligned is true, then buffer is aligned so memcpy() can be used
	int				ReadBytes( void * pDst,
						int nBytes );

	int				Write( bool bBit );								// set the next bit, returns number of bits written
	int				Write( const void * pSrc,						// write bits, returns the number of bits written
						int nBits, bool bAligned );				
	int				WriteBytes( const void * pSrc,				// write bytes, returns the number of bytes written
						int nBytes );								

	BitIterator &	operator=( const BitIterator & copy );

	// skip bits
	BitIterator &	operator+=(int n );				
	BitIterator &	operator-=(int n );				
	BitIterator &	operator++(int);				
	BitIterator &	operator--(int);				

private:
	// Data
	byte *			m_pStart;											// start of the buffer
	byte *			m_pEnd;												// end of the buffer
	byte *			m_pIt;												// current byte pointer
	int				m_nOffset;											// current bit in byte pointer
};

//---------------------------------------------------------------------------------------------------

inline bool BitIterator::IsValid() const
{
	return m_pIt != NULL && m_pIt >= m_pStart && m_pIt <= m_pEnd;
}

inline byte * BitIterator::Start() const
{
	return m_pStart;
}

inline byte * BitIterator::End() const
{
	return m_pEnd;
}

inline byte * BitIterator::It() const
{
	return m_pIt;
}

inline int BitIterator::Offset() const
{
	return m_nOffset;
}

inline Buffer BitIterator::GetBuffer() const
{
	int nBytes = (int)(m_pIt - m_pStart);
	if ( m_nOffset != 0 )
		nBytes++;

	return Buffer( m_pStart, nBytes );
}

//----------------------------------------------------------------------------

inline bool BitIterator::SetIt( byte * pIt )
{
	m_pIt = pIt;
	return IsValid();
}

inline void BitIterator::SetOffset( int nOffset )
{
	m_nOffset = nOffset & 0x7;
}

inline void BitIterator::Inc()
{
	m_nOffset++;
	if ( m_nOffset > 7 )
	{
		m_pIt++;
		m_nOffset = 0;
	}
}

inline void BitIterator::Add( unsigned int n )
{
	int nBits = m_nOffset + n;
	m_pIt += nBits >> 3;					// / 8
	m_nOffset = nBits & 0x7;			// % 8
}

inline void BitIterator::Sub( unsigned int n )
{
	int nBits = m_nOffset - n;
	m_pIt -= nBits < 0 ? (((-nBits) >> 3) + 1) : 0;
	m_nOffset = nBits < 0 ? ((nBits & 0x7) + 8) : nBits;
}

inline int BitIterator::Read( bool & bBit )
{
	if ( IsValid() )
	{
		// get the bit
		bBit = (*m_pIt & (1 << m_nOffset)) != 0;
		// next bit
		Inc();
		return 1;
	}

	return 0;
}

inline int BitIterator::Write( bool bBit )
{
	if (! IsValid() )
		return 0;

	byte nBit = 1 << m_nOffset;
	if ( bBit )
		*m_pIt |= nBit;
	else
		*m_pIt &= ~nBit;
	Inc();
	return 1;
}

inline BitIterator & BitIterator::operator=( const BitIterator & copy )
{
	m_pStart = copy.m_pStart;
	m_pEnd = copy.m_pEnd;
	m_pIt = copy.m_pIt;
	m_nOffset = copy.m_nOffset;
	return *this;
}


inline BitIterator & BitIterator::operator+=(int n)
{
	Add( n );
	return *this;
}

inline BitIterator & BitIterator::operator-=(int n)
{
	Sub( n );
	return *this;
}

inline BitIterator & BitIterator::operator++(int)
{
	Add( 1 );
	return *this;
}

inline BitIterator & BitIterator::operator--(int)
{
	Sub( 1 );
	return *this;
}

inline BitIterator operator+(const BitIterator & b, int n )
{
	return BitIterator( b ) += n;
}

inline BitIterator operator-(const BitIterator & b,int n )
{
	return BitIterator( b ) -= n;
}

//-------------------------------------------------------------------------------

IMPLEMENT_STREAMING( u8 );
IMPLEMENT_STREAMING( u16 );
IMPLEMENT_STREAMING( u32 );
IMPLEMENT_STREAMING( u64 );
IMPLEMENT_STREAMING( s8 );
IMPLEMENT_STREAMING( s16 );
IMPLEMENT_STREAMING( s32 );
IMPLEMENT_STREAMING( s64 );
IMPLEMENT_STREAMING( f32 );
IMPLEMENT_STREAMING( f64 );
IMPLEMENT_STREAMING( unsigned long );		// dword
IMPLEMENT_STREAMING( long );
IMPLEMENT_STREAMING( Color );

inline BitIterator & operator>>( BitIterator & input, bool & boolean )
{																						
	input.Read( boolean );										
	return input;																	
}																						

inline BitIterator & operator<<( BitIterator & output, const bool & boolean )
{																									
	output.Write( boolean );												
	return output;																				
}

//---------------------------------------------------------------------------------------------------

inline BitIterator & operator>>( BitIterator & input, WideString & string )
{
	unsigned short nLen = 0;
	input >> nLen;

	string.allocate( nLen );
	input.ReadBytes( string.buffer(), sizeof(wchar_t) * nLen );

	return input;
}

inline BitIterator & operator<<( BitIterator & output, const WideString & string )
{
	unsigned short nLen = (unsigned short)string.length();
	output << nLen;
	output.WriteBytes( string.buffer(), sizeof(wchar_t) * nLen );
	return output;
}

inline BitIterator & operator>>( BitIterator & input, CharString & string )
{
	unsigned short nLen = 0;
	input >> nLen;

	string.allocate( nLen );
	input.ReadBytes( string.buffer(), sizeof(char) * nLen );

	return input;
}

inline BitIterator & operator<<( BitIterator & output, const CharString & string )
{
	unsigned short nLen = (unsigned short)string.length();
	output << nLen;
	output.WriteBytes( string.buffer(), sizeof(char) * nLen );
	return output;
}
//----------------------------------------------------------------------------

inline BitIterator& operator<<( BitIterator & output, const char * pString )
{
	output.WriteBytes( pString, (int)(strlen( pString ) * sizeof(char)) );
	return output;
}

inline BitIterator& operator<<( BitIterator & output, const wchar * pString )
{
	output.WriteBytes( pString, (int)(wcslen( pString ) * sizeof(wchar)) );
	return output;
}

//---------------------------------------------------------------------------------------------------

#include "Array.h"

template<class T>
inline BitIterator & operator>>( BitIterator & input, Array<T> & vector )
{
	int nSize = 0;
	input >> nSize;

	vector.allocate( nSize );
	for(int i=0;i<nSize;i++)
		input >> vector[i];
	return input;
}

template<class T>
inline BitIterator & operator<<( BitIterator & output, const Array<T> & vector )
{
	int nSize = vector.size();
	output << nSize;

	for(int i=0;i<nSize;i++)
		output << vector[i];

	return output;
}


//---------------------------------------------------------------------------------------------------

#include "List.h"

template<class T>
inline BitIterator & operator>>( BitIterator & input, List<T> & li )
{
	int nSize = 0;
	input >> nSize;

	li.release();
	for(int i=0;i<nSize;i++)
	{
		T object;
		input >> object;

		li.insert( object );
	}
	return input;
}

template<class T>
inline BitIterator & operator<<( BitIterator & output, const List<T> & li )
{
	int nSize = li.size();
	output << nSize;

	typename List<T>::Iterator i = li.head();
    for(;i.valid(); i++ )
		output << *i;

	return output;
}


//---------------------------------------------------------------------------------------------------

#include "Tree.h"

template<class K,class T>
inline BitIterator & operator<<( BitIterator & output, const Tree<K,T> & tree )
{
	output << tree.size();

	typename Tree<K,T>::Iterator i = tree.head();
	for(;i.valid(); i++)
		output << i.key() << (*i);

	return output;
}

template<class K,class T>
inline BitIterator & operator>>( BitIterator & input, Tree<K,T> & tree )
{
	int nSize = 0;
	input >> nSize;

	tree.release();
	for(int i=0;i<nSize;i++)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		tree[ key ] = object;
	}

	return input;
}

//---------------------------------------------------------------------------------------------------

#include "Hash.h"

template<class K,class T>
inline BitIterator & operator<<( BitIterator & output, const Hash<K,T> & hash )
{
	output << hash.size();

	typename Hash<K,T>::Iterator i = hash.head();
	for(;i.valid(); i++)
		output << i.key() << (*i);

	return output;
}

template<class K,class T>
inline BitIterator & operator>>( BitIterator & input, Hash<K,T> & hash )
{
	int nSize = 0;
	input >> nSize;

	hash.release();
	for(int i=0;i<nSize;i++)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		hash[ key ] = object;
	}

	return input;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
