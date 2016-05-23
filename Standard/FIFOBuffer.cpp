/*
	FIFOBuffer.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/Limits.h"
#include "Standard/FIFOBuffer.h"

#include <memory.h>
#include <stdlib.h>		// NULL

//----------------------------------------------------------------------------------------

FIFOBuffer::FIFOBuffer() : m_nBottom( 0 ), m_nTop( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    initialize();
}

FIFOBuffer::FIFOBuffer(const void * pBytes, int nBytes ) 
	: m_nBottom( 0 ), m_nTop( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    initialize( pBytes, nBytes );
}

FIFOBuffer::FIFOBuffer(const FIFOBuffer & copy, int nBytes )
	: m_nBottom( 0 ), m_nTop( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    initialize( copy, nBytes );
}

FIFOBuffer::FIFOBuffer(const FIFOBuffer & copy )
	: m_nBottom( 0 ), m_nTop( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    initialize( copy );
}

FIFOBuffer::~FIFOBuffer()
{
    release();
}

//----------------------------------------------------------------------------

int FIFOBuffer::size() const
{
    return m_nTop - m_nBottom;
}

int FIFOBuffer::contiguousSize() const
{
	int nMax = m_nTop - m_nBottom;
	int nOffset = m_nBottom % BLOCK_SIZE;
	int nRemain = BLOCK_SIZE - nOffset;

	return Min( nRemain, nMax );
}

void * FIFOBuffer::contiguousBytes() const
{
	int nBlock = m_nBottom / BLOCK_SIZE;
	int nOffset = m_nBottom % BLOCK_SIZE;

	return m_pBlocks[ nBlock ] + nOffset;
}

int FIFOBuffer::peek( void * a_pBuffer, int a_nBytes, int a_nOffset /*= 0*/ ) const
{
	if ( a_pBuffer == NULL || a_nBytes <= 0 )
		return 0;			// invalid parameters or 0 bytes

	int nAvailable = size() - a_nOffset;
    if ( a_nBytes > nAvailable )
		a_nBytes = nAvailable;		// number of bytes exceed the number available, reduce the count automatically..

	int nBytesCopied = 0;
	int nPos = m_nBottom + a_nOffset;
	while( a_nBytes > 0 )
	{
		int nBlock = nPos / BLOCK_SIZE;
		int nOffset = nPos % BLOCK_SIZE;
		int nRemain = BLOCK_SIZE - nOffset;

		int nCopy = Min<int>( a_nBytes, nRemain );
		memcpy( a_pBuffer, m_pBlocks[ nBlock ] + nOffset, nCopy );

		nPos += nCopy;
		nBytesCopied += nCopy;
		a_nBytes -= nCopy;
		a_pBuffer = ((byte *)a_pBuffer) + nCopy;
	}

	return nBytesCopied;
}

//----------------------------------------------------------------------------

int FIFOBuffer::pop( int nBytes )
{
    if ( nBytes > size() )
		nBytes = size();		// number of bytes exceed the number available, reduce the count automatically..

	m_nBottom += nBytes;
	if ( m_nBottom >= BLOCK_SIZE )
	{
		if ( m_nBottom < m_nTop )
		{
			// move blocks from the front to the back.. 
			int nFirstBlock = m_nBottom / BLOCK_SIZE;
			for(int i=0;i<nFirstBlock;++i)
			{
				// shift all blocks down by 1..
				byte * pMoveBack = m_pBlocks[0];
				for(int j=1;j<m_nBlocks;++j)
					m_pBlocks[j-1] = m_pBlocks[j];
				m_pBlocks[ m_nBlocks - 1 ] = pMoveBack;
			}

			// adjust the top and bottom now by the number of blocks removed
			m_nBottom -= (nFirstBlock * BLOCK_SIZE);
			m_nTop -= (nFirstBlock * BLOCK_SIZE);
		}
		else
		{
			// set bottom & top back to the start of the buffer chain..
			m_nBottom = m_nTop = 0;
		}
	}

	return nBytes;
}

int FIFOBuffer::pop( void * pBuffer, int nBytes )
{
	return pop( peek( pBuffer, nBytes ) );
}

int FIFOBuffer::push( const void * pBuffer, int nBytes )
{
	if ( pBuffer == NULL || nBytes <= 0  )
		return 0;		// invalid parameters or 0 bytes

	// grow the file size only if needed
	grow( nBytes );

	int nBytesCopied = 0;
 	while( nBytes > 0 )
	{
		int nBlock = m_nTop / BLOCK_SIZE;
		int nOffset = m_nTop % BLOCK_SIZE;
		int nRemain = BLOCK_SIZE - nOffset;

		int nCopy = Min<int>( nBytes, nRemain );
		memcpy(m_pBlocks[ nBlock ] + nOffset, pBuffer, nCopy );

		m_nTop += nCopy;
		nBytesCopied += nCopy;
		nBytes -= nCopy;
		pBuffer = (byte *)pBuffer + nCopy;
	}

	return nBytesCopied;
}

int FIFOBuffer::push( const FIFOBuffer & in, int nBytes )
{
	if ( nBytes <= 0 )
		return 0;		// nothing to push...
	if ( nBytes > in.size() )
		nBytes = in.size();

	byte * pBuffer = (byte *)malloc( nBytes );
	in.peek( pBuffer, nBytes );

	push( pBuffer, nBytes );
	free( pBuffer );

	return nBytes;
}

int FIFOBuffer::pipe( FIFOBuffer & buffer, int nBytes )
{
	return buffer.pop( push( buffer, nBytes ) );
}

int FIFOBuffer::flush()
{
	int nBytes = m_nTop - m_nBottom;
	release();

	return nBytes;
}

//----------------------------------------------------------------------------

void FIFOBuffer::initialize()
{
    release();
}

void FIFOBuffer::initialize( const void * pBytes, int nBytes )
{
    release();
	push( pBytes, nBytes );
}

void FIFOBuffer::initialize( const FIFOBuffer & copy, int nBytes )
{
	release();
	push( copy, nBytes );
}

void FIFOBuffer::initialize( const FIFOBuffer & copy )
{
	release();
	push( copy, copy.size() );
}

void FIFOBuffer::release()
{
	for(int i=0;i<m_nBlocks;i++)
		free( m_pBlocks[ i ] );

	free( m_pBlocks );
	m_pBlocks = NULL;
	m_nBlocks = 0;
	m_nTop = 0;
	m_nBottom = 0;
}


FIFOBuffer & FIFOBuffer::operator=( const FIFOBuffer & copy )
{
    initialize( copy );
	return *this;
}

//------------------------------------------------------------------------------------------------------

void FIFOBuffer::grow( int nBytes )
{
	// grow the file size only if needed
	int nBlocks = ((m_nTop + nBytes) + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    if ( nBlocks > m_nBlocks )
	{
		byte ** pNewBlocks = (byte **)malloc( sizeof(byte *) * nBlocks );
		int i=0;
		for(;i<m_nBlocks;i++)
			pNewBlocks[i] = m_pBlocks[i];
		for(;i<nBlocks;i++)
			pNewBlocks[i] = (byte *)malloc( BLOCK_SIZE );

		free( m_pBlocks );
		m_pBlocks = pNewBlocks;
		m_nBlocks = nBlocks;
	}
}

//----------------------------------------------------------------------------------------
// EOF
