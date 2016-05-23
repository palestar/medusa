/*
    FileMemory.cpp

    Memory File class
    (c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h" 
#include "Debug/Trace.h" 
#include "File/FileMemory.h"

#include <limits.h>

//----------------------------------------------------------------------------------------

const unsigned int MEMORY_BLOCK_SIZE = 32 * 1024;  // must be power of 2, allocation of memory occurs in these increments
static inline File::Size SizeToBlocks(File::Size size)
{
    return ( (size | (MEMORY_BLOCK_SIZE - 1)) + 1);
}

//----------------------------------------------------------------------------------------

FileMemory::FileMemory() : m_pBytes( NULL ), m_Size( 0 ), m_Pos( 0 ), m_Allocated( false ), m_Blocked( false )
{
	//TRACE( "FileMemory::FileMemory" );
    open();
}

FileMemory::FileMemory(const void * pBytes, Size nBytes, bool copy /*= true*/ ) 
	: m_pBytes( NULL ), m_Size( 0 ), m_Pos( 0 ), m_Allocated( false ), m_Blocked( false )
{
	//TRACE( "FileMemory::FileMemory" );
    ASSERT(pBytes);

    open( pBytes, nBytes, copy );
}

FileMemory::FileMemory(const FileMemory & file )
	: File( file ), m_pBytes( NULL ), m_Size( 0 ), m_Pos( 0 ), m_Allocated( false ), m_Blocked( false )
{
	//TRACE( "FileMemory::FileMemory" );
    open( file.m_pBytes, file.m_Size, file.m_Allocated ? false : true );
	m_Pos = file.m_Pos;
}

FileMemory::FileMemory(const FileMemory &file, bool copy )
	: File( file ), m_pBytes( NULL ), m_Size( 0 ), m_Pos( 0 ), m_Allocated( false ), m_Blocked( false )
{
	//TRACE( "FileMemory::FileMemory" );
    open( file.m_pBytes, file.m_Size, copy );
	m_Pos = file.m_Pos;
}

FileMemory::~FileMemory()
{
	//TRACE( "FileMemory::~FileMemory" );
    close();
}

//----------------------------------------------------------------------------

File * FileMemory::clone() const
{
	return new FileMemory( *this );
}

bool FileMemory::isReadable() const
{
    return true;
}

bool FileMemory::isWritable() const
{
    return true;
}

File::Size FileMemory::size() const
{
    return m_Size;
}

File::Position FileMemory::position() const
{
    return m_Pos;
}

//----------------------------------------------------------------------------

File::Size FileMemory::read( void * pDest, Size nBytes )
{
	if ( !pDest || nBytes == 0 || m_Pos > m_Size )
		return 0;

    if ( m_Pos + nBytes > m_Size )
		nBytes = m_Size - m_Pos;

	if ( nBytes > 0 )
	{
		memcpy( pDest, m_pBytes + m_Pos, nBytes );
		m_Pos += nBytes;
	}

	return nBytes;
}

File::Size FileMemory::write( const void * pSrc, Size nBytes )
{
	if ( !pSrc || nBytes == 0 )
		return 0;

    if (m_Pos + nBytes >= m_Size)
    {
        Size nNewSize = m_Pos + nBytes;
        if (!m_Allocated || !m_Blocked || SizeToBlocks(m_Size) != SizeToBlocks(nNewSize))
        {
            byte *pNewBytes = new byte[ SizeToBlocks( nNewSize ) ];
			if ( pNewBytes == NULL )
			{
				if ( m_Allocated )
					delete [] m_pBytes;
				return 0;
			}
            memcpy(pNewBytes, m_pBytes, m_Size );
 
			if ( m_Allocated )
				delete [] m_pBytes;

            m_pBytes = pNewBytes;
			m_Allocated = true;
			m_Blocked = true;
        }
        m_Size = nNewSize; 
    }

    memcpy(m_pBytes + m_Pos, pSrc, static_cast<unsigned>(nBytes));
    m_Pos += nBytes;

	return nBytes;
}

void FileMemory::flush()
{}

File::Position FileMemory::setPosition( Position position ) 
{
    m_Pos = position;
	return m_Pos;
}

File::Size FileMemory::setSize( Size nNewSize )
{
    if (!m_Allocated || nNewSize != m_Size)
    {
        byte *pNewBytes = new byte[ nNewSize ];
        memcpy(pNewBytes, m_pBytes, m_Size > nNewSize ? nNewSize : m_Size );

		if ( m_Allocated )
			delete [] m_pBytes;

        m_pBytes = pNewBytes;
		m_Allocated = true;
		m_Blocked = false;
    }

    m_Size = nNewSize;
    if (m_Pos > m_Size)
        m_Pos = m_Size;
	return m_Size;
}

File::Size FileMemory::setEOF()
{
    setSize( position() );
	return m_Size;
}

void FileMemory::lock()
{}

void FileMemory::unlock()
{}

//----------------------------------------------------------------------------

void *FileMemory::bytes() const
{
    return( m_pBytes );
}

//----------------------------------------------------------------------------

void FileMemory::open()
{
    close();

    m_Pos = m_Size = 0;
	m_Allocated = true;
	m_Blocked = true;
    m_pBytes = new byte[ SizeToBlocks( m_Size ) ];
}

void FileMemory::open( const void * pBytes, Size nBytes, bool copy /*= true*/ )
{
    close();

    m_Pos = 0;
    m_Size = nBytes;
	
	if ( copy )
	{
		m_Allocated = true;
		m_Blocked = false;
		m_pBytes = new byte[ m_Size ];

		if ( pBytes != NULL )
			memcpy(m_pBytes, pBytes, m_Size );
	}
	else
	{
		m_Allocated = false;
		m_Blocked = false;
		m_pBytes = static_cast<unsigned char *>( const_cast<void *>( pBytes ) );
	}
}

void FileMemory::open( const FileMemory & copy )
{
    open( copy.m_pBytes, copy.m_Size, true );
}

void FileMemory::close()
{
	if ( m_Allocated )
	{
		delete [] m_pBytes;
		m_pBytes = NULL;

		m_Allocated = false;
		m_Blocked = false;
	}
}

void FileMemory::remove( dword nBytes )
{
	if ( nBytes >= m_Size )
	{
		if ( SizeToBlocks( m_Size ) != SizeToBlocks( 0 ) )
		{
			if ( m_Allocated )
				delete [] m_pBytes;
			m_pBytes = new byte[ SizeToBlocks( 0 ) ];
			m_Allocated = true;
			m_Blocked = true;
		}

		m_Pos = m_Size = 0;
		return;
	}

	Size nNewSize = m_Size - nBytes;
    if (m_Blocked && SizeToBlocks(m_Size) != SizeToBlocks(nNewSize))
    {
        byte *pNewBytes = new byte[ SizeToBlocks( nNewSize ) ];
        memcpy(pNewBytes, m_pBytes + nBytes, nNewSize );

		if ( m_Allocated )
			delete [] m_pBytes;

        m_pBytes = pNewBytes;
		m_Allocated = true;
		m_Blocked = true;
    }
	else
	{
		memcpy( m_pBytes, m_pBytes + nBytes, nNewSize );
	}

	// set the new file size
	m_Size = nNewSize;
	// adjust the current file position by the number of bytes removed
	m_Pos = Clamp<dword>( m_Pos - nBytes, 0, m_Size);
}

FileMemory & FileMemory::operator=( const FileMemory & copy )
{
    open( copy.m_pBytes, copy.m_Size, true );
	return *this;
}

//----------------------------------------------------------------------------

const InStream & operator>>( const InStream & input, FileMemory & file )
{
	dword size;
	input >> size;

	file.open( NULL, size );
	input.read( file.bytes(), file.size() );

	return input;
}


const OutStream & operator<<( const OutStream & output, const FileMemory & file )
{
	output << file.size();
	output.write( file.bytes(), file.size() );

	return output;
}

//----------------------------------------------------------------------------------------
// EOF
