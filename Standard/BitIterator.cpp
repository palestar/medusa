/*
	BitIterator.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "BitIterator.h"

//---------------------------------------------------------------------------------------------------

BitIterator::BitIterator() : m_pStart( NULL ), m_pEnd( NULL ), m_pIt( NULL ), m_nOffset( 0 )
{}

BitIterator::BitIterator( void * pStart, void * pEnd, int nOffset /*= 0*/, int nAdd /*= 0*/ ) 
	: m_pStart( NULL ), m_pEnd( NULL ), m_pIt( NULL ), m_nOffset( 0 )
{
	Set( pStart, pEnd, nOffset, nAdd );
}

BitIterator::BitIterator( void * pStart, int nSize, int nOffset /*= 0*/, int nAdd /*= 0*/ )
	: m_pStart( NULL ), m_pEnd( NULL ), m_pIt( NULL ), m_nOffset( 0 )
{
	Set( pStart, (byte *)pStart + nSize, nOffset, nAdd );
}

BitIterator::BitIterator( const BitIterator & copy ) 
	: m_pStart( copy.m_pStart ), m_pEnd( copy.m_pEnd ), m_pIt( copy.m_pIt ), m_nOffset( copy.m_nOffset )
{}

//---------------------------------------------------------------------------------------------------

bool BitIterator::Set( void * pStart, void * pEnd, int nOffset /*= 0*/, int nAdd /*= 0*/ )
{
	if ( pStart == NULL || pEnd == NULL || pStart > pEnd )
		return false;			// start cannot be AFTER end

	m_pStart = (byte *)pStart;
	m_pEnd = (byte *)pEnd;

	int nBits = nOffset + nAdd;
	m_pIt = m_pStart + (nBits / 8);
	m_nOffset = nBits % 8;

	return IsValid();
}

void BitIterator::Rewind()
{
	m_pIt = m_pStart;
	m_nOffset = 0;
}

int BitIterator::Read( void * pDst, int n, bool bAligned )
{
	if ( pDst == NULL || n == 0 || !IsValid() )
		return 0;

	int nBits = 0;
	if ( bAligned )
	{
		// if we are not byte aligned already, then force the current read offset to a byte aligned position, so we can use memcpy()
		if ( m_nOffset > 0 )
		{
			m_pIt++;
			m_nOffset = 0;
			if (! IsValid() )
				return 0;								// past end of buffer
		}

		int nBytes = n / 8;
		if ( (m_pIt + nBytes) > (m_pEnd + 1) )
			nBytes = (m_pEnd - m_pIt) + 1;		// trying to read too many bytes, how many can we actually read

		if ( nBytes > 0 )
		{
			memcpy( pDst, m_pIt, nBytes );
			pDst = (byte *)pDst + nBytes;

			m_pIt += nBytes;
			nBits += nBytes * 8;
		}

		if (! IsValid() )
			return nBits;

		// now fall through, and let the code below handle any remaining bits...
	}

	int nDstOffset = 0;
	while( nBits < n )
	{
		nBits++;
		if ( (*m_pIt & (1 << m_nOffset)) != 0 )
			*((byte *)pDst) |= (1 << nDstOffset);
		else
			*((byte *)pDst) &= ~(1 << nDstOffset);

		m_nOffset++;
		if ( m_nOffset > 7 )
		{
			m_pIt++;
			m_nOffset = 0;

			if (! IsValid() )
				return nBits;								// out of buffer, return bits read..
		}

		nDstOffset++;
		if ( nDstOffset > 7 )
		{
			pDst = (byte *)pDst + 1;
			nDstOffset = 0;
		}
	}

	return nBits;
}

int BitIterator::ReadBytes( void * pDst, int nBytes )
{
	return Read( pDst, nBytes * 8, true ) / 8;
}

int BitIterator::Write( const void * pSrc, int n, bool bAligned )
{
	if ( pSrc == NULL || n == 0 || !IsValid() )
		return 0;

	int nBits = 0;
	if ( bAligned )
	{
		// if we are not byte aligned already, then force the current offset to a byte aligned position, so we can use memcpy()
		if ( m_nOffset > 0 )
		{
			m_pIt++;
			m_nOffset = 0;
			if (! IsValid() )
				return 0;								// past end of buffer
		}

		int nBytes = n / 8;
		if ( (m_pIt + nBytes) > (m_pEnd + 1) )
			nBytes = (m_pEnd - m_pIt) + 1;		// trying to read too many bytes, how many can we actually read

		if ( nBytes > 0 )
		{
			memcpy( m_pIt, pSrc, nBytes );
			pSrc = (byte *)pSrc + nBytes;
			m_pIt += nBytes;

			nBits += nBytes * 8;
		}

		if (! IsValid() )
			return nBits;

		// now fall through, and let the code below handle any remaining bits...
	}

	int nSrcOffset = 0;
	while( nBits < n )
	{
		nBits++;
		if ( (*((byte *)pSrc) & (1 << nSrcOffset)) != 0 )
			*m_pIt |= (1 << m_nOffset);
		else
			*m_pIt &= ~(1 << m_nOffset);

		m_nOffset++;
		if ( m_nOffset > 7 )
		{
			m_pIt++;
			m_nOffset = 0;

			if (! IsValid() )
				return nBits;								// out of buffer, return bits read..
		}

		nSrcOffset++;
		if ( nSrcOffset > 7 )
		{
			pSrc = (byte *)pSrc + 1;
			nSrcOffset = 0;
		}
	}

	return nBits;
}

int BitIterator::WriteBytes( const void * pSrc, int nBytes )
{
	return Write( pSrc, nBytes * 8, true ) / 8;
}

//---------------------------------------------------------------------------------------------------
// EOF

