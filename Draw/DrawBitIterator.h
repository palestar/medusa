/*
	DrawBitIterator.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef DRAWBITITERATOR_H
#define DRAWBITITERATOR_H

#include "Standard/Types.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL DrawBitIterator
{
public:
	// Construction
	DrawBitIterator( byte * pBytes, int nOffset = 0, int nAdd = 0 );
	DrawBitIterator( const DrawBitIterator & copy, int nAdd = 0 );

	// Mutators
	void			set( byte * pBytes, 
						int nOffset = 0, 
						int nAdd = 0 );								// initialize the bit iterator

	bool			getBit();										// gets 1 bit
	dword			getBits( int n );								// get n bits, up to 32 bits
	void			putBit( bool bBit );							// set the next bit
	void			putBits( int n, dword nBits );					// set the next n bits to nValue, up to 32 bits

	DrawBitIterator &	operator=( const DrawBitIterator & copy );

	// skip bits
	DrawBitIterator &	operator+=(int n );				
	DrawBitIterator &	operator-=(int n );				
	DrawBitIterator &	operator++(int);				
	DrawBitIterator &	operator--(int);				

private:
	byte *			m_pBytes;			// bit pointer
	int				m_nOffset;			// current bit in byte pointer
};

//----------------------------------------------------------------------------

inline DrawBitIterator::DrawBitIterator( byte * pBytes, int nOffset /*= 0*/, int nAdd /*= 0*/ ) : m_pBytes( 0 ), m_nOffset( 0 )
{
	set( pBytes, nOffset, nAdd );
}

inline DrawBitIterator::DrawBitIterator( const DrawBitIterator & copy, int nAdd /*= 0*/ ) : m_pBytes( 0 ), m_nOffset( 0 )
{
	set( copy.m_pBytes, copy.m_nOffset, nAdd );
}

//----------------------------------------------------------------------------

inline void DrawBitIterator::set( byte * pBytes, int nOffset /*= 0*/, int nAdd /*= 0*/ )
{
	int nBits = m_nOffset + nAdd;
	m_pBytes = pBytes + (nBits / 8);
	m_nOffset = nBits % 8;
}

inline bool DrawBitIterator::getBit()
{
	// get the bit
	bool bBit = (*m_pBytes & (0x80 >> m_nOffset)) != 0;

	// next bit
	m_nOffset++;
	if ( m_nOffset > 7 )
	{
		m_pBytes++;
		m_nOffset = 0;
	}

	return bBit;
}

inline dword DrawBitIterator::getBits( int n )
{
	if ( n < 1 )
		return 0;
	if ( n > 32 )
		n = 32;

	dword nBits = 0;
	for(int i=0;i<n;i++)
	{
		nBits <<= 1;
		if ( getBit() )
			nBits |= 1;
	}

	return nBits;
}

inline void DrawBitIterator::putBit( bool bBit )
{
	byte nBit = 128 >> m_nOffset;
	if ( bBit )
		*m_pBytes |= nBit;
	else
		*m_pBytes &= ~nBit;

	// next bit
	m_nOffset++;
	if ( m_nOffset > 7 )
	{
		m_pBytes++;
		m_nOffset = 0;
	}
}

inline void DrawBitIterator::putBits( int n, dword nBits )
{
	for(int i=0;i<n;i++)
		putBit( (nBits & (1 << i)) != 0 );
}

inline DrawBitIterator & DrawBitIterator::operator=( const DrawBitIterator & copy )
{
	m_pBytes = copy.m_pBytes;
	m_nOffset = copy.m_nOffset;
	return *this;
}


inline DrawBitIterator & DrawBitIterator::operator+=(int n)
{
	int nBits = m_nOffset + n;
	m_pBytes += nBits / 8;
	m_nOffset = nBits % 8;
	return *this;
}

inline DrawBitIterator & DrawBitIterator::operator-=(int n)
{
	int nBits = m_nOffset - n;
	m_pBytes -= nBits < 0 ? ((-nBits / 8) + 1) : 0;
	m_nOffset = nBits < 0 ? ((nBits % 8) + 8) : nBits;
	return *this;
}

inline DrawBitIterator & DrawBitIterator::operator++(int)
{
	return operator+=( 1 );
}

inline DrawBitIterator & DrawBitIterator::operator--(int)
{
	return operator-=( 1 );
}

//----------------------------------------------------------------------------

inline DrawBitIterator operator+(const DrawBitIterator & b, int n )
{
	return DrawBitIterator( b, n );
}

inline DrawBitIterator operator-(const DrawBitIterator & b,int n )
{
	return DrawBitIterator( b, -n );
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
