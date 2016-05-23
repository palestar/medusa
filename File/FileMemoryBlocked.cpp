/*
	FileMemoryBlocked.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h" 
#include "Debug/Trace.h" 
#include "File/FileMemoryBlocked.h"

#include <limits.h>

//----------------------------------------------------------------------------------------

FileMemoryBlocked::FileMemoryBlocked() : m_Pos( 0 ), m_Size( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    open();
}

FileMemoryBlocked::FileMemoryBlocked(const void * pBytes, Size nBytes ) 
	: m_Pos( 0 ), m_Size( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    open( pBytes, nBytes );
}

FileMemoryBlocked::FileMemoryBlocked( const FileMemoryBlocked & copy )
	: File(), m_Pos( 0 ), m_Size( 0 ), m_nBlocks( 0 ), m_pBlocks( NULL )
{
    open( copy );
}

FileMemoryBlocked::~FileMemoryBlocked()
{
    close();
}

//----------------------------------------------------------------------------

File * FileMemoryBlocked::clone() const
{
	return new FileMemoryBlocked( *this );
}

bool FileMemoryBlocked::isReadable() const
{
    return true;
}

bool FileMemoryBlocked::isWritable() const
{
    return true;
}

File::Size FileMemoryBlocked::size() const
{
    return m_Size;
}

File::Position FileMemoryBlocked::position() const
{
    return m_Pos;
}

//----------------------------------------------------------------------------

File::Size FileMemoryBlocked::read( void * pBuffer, Size nBytes )
{
	if ( pBuffer == NULL || m_Pos > m_Size )
		return 0;

    if ( (m_Pos + nBytes) > m_Size )
		nBytes = m_Size - m_Pos;

	Size nBytesRead = 0;
	while( nBytesRead < nBytes )
	{
		int nBlock = m_Pos / BLOCK_SIZE;
		int nOffset = m_Pos % BLOCK_SIZE;
		int nRemain = BLOCK_SIZE - nOffset;

		ASSERT( nBlock < m_nBlocks );
		ASSERT( m_pBlocks != NULL && m_pBlocks[ nBlock ] != NULL );

		int nCopy = Min<int>( nBytes - nBytesRead, nRemain );
		memcpy( pBuffer, m_pBlocks[ nBlock ] + nOffset, nCopy );

		// update the current file position
		m_Pos += nCopy;
		nBytesRead += nCopy;
		pBuffer = (byte *)pBuffer + nCopy;
	}

	return nBytesRead;
}

File::Size FileMemoryBlocked::write( const void * pBuffer, Size nBytes )
{
	if ( pBuffer == NULL )
		return 0;
	// grow the file size only if needed
    if ( (m_Pos + nBytes) >= m_Size)
		setSize( m_Pos + nBytes );

	Size nBytesWritten = 0;
 	while( nBytesWritten < nBytes )
	{
		int nBlock = m_Pos / BLOCK_SIZE;
		int nOffset = m_Pos % BLOCK_SIZE;
		int nRemain = BLOCK_SIZE - nOffset;

		ASSERT( nBlock < m_nBlocks );
		ASSERT( m_pBlocks != NULL && m_pBlocks[ nBlock ] != NULL );

		int nCopy = Min<int>( nBytes - nBytesWritten, nRemain );
		memcpy(m_pBlocks[ nBlock ] + nOffset, pBuffer, nCopy );

		// update the current file position
		m_Pos += nCopy;
		nBytesWritten += nCopy;

		pBuffer = (byte *)pBuffer + nCopy;
	}

	return nBytesWritten;
}

void FileMemoryBlocked::flush()
{}

File::Position FileMemoryBlocked::setPosition( Position nPos ) 
{
    m_Pos = nPos;
	return m_Pos;
}

File::Size FileMemoryBlocked::setSize(Size nNewSize)
{
	int nBlocksNeeded = (nNewSize + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    if ( nBlocksNeeded > m_nBlocks )
    {
		// size is growing
		byte ** pNewBlocks = (byte **)malloc( sizeof(byte *) * nBlocksNeeded );
		for(int i=0;i<nBlocksNeeded;i++)
		{
			if ( i >= m_nBlocks )
				pNewBlocks[ i ] = (byte *)malloc( BLOCK_SIZE );			// new block
			else
				pNewBlocks[ i ] = m_pBlocks[ i ];				// copy of existing block
		}

		free( m_pBlocks );
		m_pBlocks = pNewBlocks;
		m_nBlocks = nBlocksNeeded;
	}
	else if ( nBlocksNeeded < m_nBlocks )
	{
		// size is shrinking, are we shrinking to a smaller size or zero
		if ( nBlocksNeeded > 0 )
		{
			int i = 0;
			byte ** pNewBlocks = (byte **)malloc( sizeof(byte *) * nBlocksNeeded );
			for(;i<nBlocksNeeded;i++)
				pNewBlocks[ i ] = m_pBlocks[ i ];

			// release the remaining blocks
			for(;i<m_nBlocks;i++)
				free( m_pBlocks[ i ] );

			free( m_pBlocks );
			m_pBlocks = pNewBlocks;
			m_nBlocks = nBlocksNeeded;
		}
		else if ( m_nBlocks > 0 )
		{
			// blocks is zero, release everything
			int i = 0;
			for(;i<m_nBlocks;i++)
				free( m_pBlocks[ i ] );

			free( m_pBlocks );
			m_pBlocks = NULL;
			m_nBlocks = 0;
		}
	}

    m_Size = nNewSize; 
    if (m_Pos > m_Size)
        m_Pos = m_Size;
	return m_Size;
}

File::Size FileMemoryBlocked::setEOF()
{
    setSize( position() );
	return m_Size;
}

void FileMemoryBlocked::lock()
{}

void FileMemoryBlocked::unlock()
{}

//----------------------------------------------------------------------------

void FileMemoryBlocked::open()
{
    close();
}

void FileMemoryBlocked::open( const void * pBytes, Size nBytes )
{
	// release everything
    close();
	// copy over the bytes
	write( pBytes, nBytes );
	rewind();
}

void FileMemoryBlocked::open( const FileMemoryBlocked & copy )
{
	setSize( copy.size() );

	ASSERT( m_nBlocks == copy.m_nBlocks );
	for(int i=0;i<m_nBlocks;i++)
		memcpy( m_pBlocks[i], copy.m_pBlocks[i], BLOCK_SIZE );

	m_Pos = copy.m_Pos;
}

void FileMemoryBlocked::close()
{
	setSize( 0 );
}

void FileMemoryBlocked::pipe( FileMemoryBlocked & in, Size nBytes )
{
	if ( nBytes > INT_MAX )
		throw FileError();

	// grow the file size only if needed
    if ( (m_Pos + nBytes) >= m_Size)
		setSize( m_Pos + nBytes );

 	while( nBytes > 0 )
	{
		int nBlock = m_Pos / BLOCK_SIZE;
		int nOffset = m_Pos % BLOCK_SIZE;
		int nRemain = BLOCK_SIZE - nOffset;

		ASSERT( nBlock < m_nBlocks );
		ASSERT( m_pBlocks != NULL && m_pBlocks[ nBlock ] != NULL );

		int nCopy = Min<int>( nBytes, nRemain );
		in.read( m_pBlocks[ nBlock ] + nOffset, nCopy );

		// update the current file position
		m_Pos += nCopy;
		nBytes -= nCopy;
	}
}

FileMemoryBlocked & FileMemoryBlocked::operator=( const FileMemoryBlocked & copy )
{
    open( copy );
	return *this;
}

//----------------------------------------------------------------------------------------
// EOF
