/*
	Stream.cpp

	Input and Output streams
	(c)2005 Palestar, Richard Lyle
*/

#include "File/Stream.h"
#include "Debug/Assert.h"
#include "Debug/Trace.h"

#include <stdarg.h>

//-------------------------------------------------------------------------------

Stream::Stream( File * pFile, dword a_nFilter /*= 0*/, int a_nVersion /*= SV_CURRENT*/, Dictionary * a_pDict /*= NULL*/ ) : 
	m_pFile( pFile ), m_nFilter( a_nFilter ), m_nVersion( a_nVersion ), m_pDictionary( a_pDict )
{}

Stream::Stream( const Stream & copy ) : 
	m_pFile( copy.m_pFile ), m_nFilter( copy.m_nFilter ), m_nVersion( copy.m_nVersion ), m_pDictionary( copy.m_pDictionary )
{}

Stream::~Stream()
{}

//----------------------------------------------------------------------------

void Stream::setFile( File * pFile )
{
	m_pFile = pFile;
}

dword Stream::setFilter( dword nFilter )
{
	dword nPrevious = m_nFilter;
	m_nFilter = nFilter;
	return nPrevious;
}

dword Stream::clearFilter( dword nClear )
{
	dword nPrevious = m_nFilter;
	m_nFilter &= ~nClear;
	return nPrevious;
}

dword Stream::addFilter( dword nAdd )
{
	dword nPrevious = m_nFilter;
	m_nFilter |= nAdd;
	return nPrevious;
}

void Stream::setVersion( int a_nVersion )
{
	m_nVersion = a_nVersion;
}

void Stream::setDictionary( Dictionary * a_pDictionary )
{
	m_pDictionary = a_pDictionary;
}

//------------------------------------------------------------------------------------

InStream::InStream( File * pFile, dword a_nFilter /*= 0*/, int a_nVersion /*= SV_CURRENT*/, Dictionary * a_pDict /*= NULL*/ ) 
	: Stream( pFile, a_nFilter, a_nVersion, a_pDict ), m_pLineBuffer( NULL ), m_nLineBufferMax( 0 )
{
	if (m_pFile.valid() && !m_pFile->isReadable() )
		throw File::FileError();
}

InStream::InStream( const InStream & copy ) : Stream( copy ), m_pLineBuffer( NULL ), m_nLineBufferMax( 0 )
{
	if (m_pFile.valid() && !m_pFile->isReadable() )
		throw File::FileError();
}

InStream::~InStream()
{
	if ( m_pLineBuffer != NULL )
		delete [] m_pLineBuffer;
}

//------------------------------------------------------------------------------------

int InStream::bytes() const
{
	if ( m_pFile.valid() )
		return m_pFile->size() - m_pFile->position();
	return 0;
}

void InStream::read( void * pDst, int nBytes ) const
{
	if (! m_pFile.valid() )
		throw File::FileError();
	if ( m_pFile->read( pDst, nBytes ) != nBytes )
		throw File::FileError();
}

const char * InStream::readLine( bool a_bEscape /*= false*/ ) const
{
	if (! m_pFile.valid() )
		throw File::FileError();
	
	int i = 0;
	while( true )
	{
		char c;
		if ( m_pFile->read( &c, sizeof(char) ) != sizeof(char) )
			throw File::FileError();
		if ( c == '\n' )
			break;
		if ( a_bEscape && c == '\\' )
		{
			char c2;
			if ( m_pFile->read( &c2, sizeof(char) ) != sizeof(char) )
				throw File::FileError();

			if ( c2 == 'n' )
				c = '\n';
			else if ( c2 == 'r' )
				c = '\r';
			else if ( c2 == 't' )
				c = '\t';
			else if ( c2 == '[' )
				c = '[';
			else if ( c2 == ']' )
				c = ']';
			else if ( c2 == '{' )
				c = '{';
			else if ( c2 == '}' )
				c = '}';
			else if ( c2 != '\\' )
				throw File::FileError();		// unsupported escape sequence!
		}

		if ( (i+1) >= m_nLineBufferMax )		// add 1 for the ending null terminator we need to add at the end..
		{
			int nPreviousBufferMax = m_nLineBufferMax;
			if ( m_nLineBufferMax < 32 )
				m_nLineBufferMax = 32;		// start the buffer out a minimum size
			else
				m_nLineBufferMax *= 2;		// double the buffer size each time we need to reallocate

			char * pNewBuffer = new char[ m_nLineBufferMax ];
			if ( pNewBuffer == NULL )
				throw File::FileError();		// memory allocation failed!
			if ( nPreviousBufferMax > 0 )
				memcpy( pNewBuffer, m_pLineBuffer, nPreviousBufferMax );

			delete [] m_pLineBuffer;
			m_pLineBuffer = pNewBuffer;
		}
		m_pLineBuffer[i++] = c;
	}

	// NULL terminate the string..
	m_pLineBuffer[i] = 0;

	return m_pLineBuffer;
}

static inline u8 HexNibble( u8 c )
{
	if ( c >= '0' && c <= '9' )
		return c - '0';
	else if ( c >= 'A' && c <= 'F' )
		return (c - 'A') + 10;
	else if ( c >= 'a' && c <= 'f' )
		return (c - 'a') + 10;
	else
		throw File::FileError();			// invalid character causes file exception!
}

void InStream::readHexLine( void * pDst, int bytes ) const
{
	const char * pLine = readLine();
	for(int i=0;i<bytes;++i)
	{
		u8 d = HexNibble( *pLine++ ) << 4;
		d |= HexNibble( *pLine++ );
		((u8 *)pDst)[i] = d;
	}
}

//------------------------------------------------------------------------------------

OutStream::OutStream( File * pFile, dword a_nFilter /*= 0*/, int a_nVersion /*= SV_CURRENT*/, Dictionary * a_pDict /*= NULL*/ ) : 
	Stream( pFile, a_nFilter, a_nVersion, a_pDict ), m_pFormatBuffer( NULL ), m_nFormatBufferMax( 0 )
{
	if( m_pFile.valid() )
	{
		m_pFile->lock();
		if (! m_pFile->isWritable() )
			throw File::FileError();
	}
}

OutStream::OutStream( File * pFile, byte * pWrite, dword bytes, dword a_nFilter /*= 0*/, int a_nVersion /*= SV_CURRENT*/, Dictionary * a_pDict /*= NULL*/ ) : 
	Stream( pFile, a_nFilter, a_nVersion, a_pDict ), m_pFormatBuffer( NULL ), m_nFormatBufferMax( 0 )
{
	if( m_pFile.valid() )
	{
		m_pFile->lock();
		if (! m_pFile->isWritable() )
			throw File::FileError();
		m_pFile->write( pWrite, bytes );
	}
}

OutStream::OutStream( const OutStream & copy ) : Stream( copy ), m_pFormatBuffer( NULL ), m_nFormatBufferMax( 0 )
{
	if( m_pFile.valid() )
	{
		m_pFile->lock();
		if (! m_pFile->isWritable() )
			throw File::FileError();
	}
}

OutStream::~OutStream()
{
	if( m_pFile.valid() )
	{
		m_pFile->flush();
		m_pFile->unlock();
	}
	if ( m_pFormatBuffer != NULL )
		delete [] m_pFormatBuffer;
}

//------------------------------------------------------------------------------------

void OutStream::pipe( const InStream & input ) const
{
	int nBytes = input.bytes();
	if ( nBytes > 0 )
	{
		byte * pBuffer = new byte[ nBytes ];
		input.read( pBuffer, nBytes );
		write( pBuffer, nBytes );

		delete [] pBuffer;
	}
}

void OutStream::write( const void * pSrc, int nBytes ) const
{
	if ( m_pFile.valid() )
	{
		if ( m_pFile->write( pSrc, nBytes ) != nBytes )
			throw File::FileError();
	}
}

void OutStream::writeLine( const char * pLine, bool a_bEscape /*= false*/ ) const
{
	if ( m_pFile.valid() )
	{
		int nLength = strlen( pLine );

		char * pEscapedLine = NULL;
		if ( a_bEscape )
		{
			pEscapedLine = escapeString( pLine );
			pLine = pEscapedLine;
			nLength = strlen( pLine );
		}

		if ( nLength > 0 && m_pFile->write( pLine, nLength ) != nLength )
			throw File::FileError();

		if ( pEscapedLine != NULL )
			delete [] pEscapedLine;

		char newline = '\n';
		if ( m_pFile->write( &newline, sizeof(char)) != sizeof(char) )
			throw File::FileError();
	}
}

static inline u8 NibbleHex( u8 c )
{
	if ( c >= 0 && c <= 9 )
		return c + '0';
	else if ( c >= 10 && c <= 15 )
		return (c - 10) + 'A';
	else
		throw File::FileError();			// invalid nibble causes file exception!
}

void OutStream::writeHexLine( const void * pSrc, int bytes ) const
{
	char * pHexString = new char[ (bytes * 2) + 1 ];
	for(int i=0;i<bytes;++i)
	{
		u8 d = ((u8 *)pSrc)[i];
		pHexString[ (i*2) ] = NibbleHex( d >> 4 );
		pHexString[ (i*2)+1 ] = NibbleHex( d & 0xf );
	}
	pHexString[ bytes * 2 ] = 0;		// null terminate the string
	writeLine( pHexString );

	delete [] pHexString;
}

void OutStream::writeFormatted( const char * pFormat, ... ) const
{
	if ( m_pFormatBuffer == NULL )
	{
		m_nFormatBufferMax = 1024;
		m_pFormatBuffer = new char[ m_nFormatBufferMax ];
	}

	va_list args;
	va_start( args, pFormat );

#if defined(_WIN32)
	while( vsprintf_s( m_pFormatBuffer, m_nFormatBufferMax, pFormat, args ) < 0 )
#else
	while( vsnprintf( m_pFormatBuffer, m_nFormatBufferMax, pFormat, args ) > m_nFormatBufferMax )
#endif
	{
		delete [] m_pFormatBuffer;
		// failed, increase our buffer size..
		m_nFormatBufferMax *= 2;
		m_pFormatBuffer = new char[ m_nFormatBufferMax ];
	}

	va_end( args );

	writeLine( m_pFormatBuffer );
}

void OutStream::flush() const
{
	if ( m_pFile.valid() )
		m_pFile->flush();
}

//---------------------------------------------------------------------------------------------------

char * OutStream::escapeString( const char * a_pIn )
{
	int nLength = strlen( a_pIn );
	char * pEscapedLine = new char[ (nLength * 2) + 1 ];
	
	char * pOut = pEscapedLine;
	for(int i=0;i<nLength;++i)
	{
		char c = a_pIn[i];
		if ( c == '\n' )
		{
			*pOut++ = '\\';
			*pOut++ = 'n';
		}
		else if ( c == '\r' )
		{
			*pOut++ = '\\';
			*pOut++ = 'r';
		}
		else if ( c == '\t' )
		{
			*pOut++ = '\\';
			*pOut++ = 't';
		}
		else if ( c == '\\' )
		{
			*pOut++ = '\\';
			*pOut++ = '\\';
		}
		else if ( c == '[' )	
		{
			*pOut++ = '\\';
			*pOut++ = '[';
		}
		else if ( c == ']' )
		{
			*pOut++ = '\\';
			*pOut++ = ']';
		}
		else if ( c == '{' )	
		{
			*pOut++ = '\\';
			*pOut++ = '{';
		}
		else if ( c == '}' )
		{
			*pOut++ = '\\';
			*pOut++ = '}';
		}
		else
			*pOut++ = c;
	}

	*pOut = 0;

	return pEscapedLine;
}

//-------------------------------------------------------------------------------
// EOF
