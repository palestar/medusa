/*
	Buffer.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/Buffer.h"
#include <stdlib.h>
#include <new>

//----------------------------------------------------------------------------

Buffer::Buffer() 
	: m_pBuffer( NULL ), m_nBytes( 0 )
{}

Buffer::Buffer( dword nBytes ) : m_pBuffer( NULL ), m_nBytes( 0 )
{
	if (! allocate( nBytes ) )
		throw std::bad_alloc();
}

Buffer::Buffer( const Buffer & copy )
	: m_pBuffer( NULL ), m_nBytes( 0 )
{
	operator=( copy );
}

Buffer::Buffer( void * pBuffer, dword nBytes, bool bCopy /*= true*/ )
	: m_pBuffer( NULL ), m_nBytes( 0 )
{
	if ( bCopy )
	{
		if (! allocate( nBytes ) )
			throw std::bad_alloc();
		memcpy( m_pBuffer, pBuffer, nBytes );
	}
	else
	{
		m_pBuffer = pBuffer;
		m_nBytes = nBytes;
	}
}

Buffer::~Buffer()
{
	free();
}

//----------------------------------------------------------------------------

void Buffer::set( void * pBuffer, dword nBytes )
{
	free();

	m_pBuffer = pBuffer;
	m_nBytes = nBytes;
}

void * Buffer::detach()
{
	void * pDetach = m_pBuffer;
	m_pBuffer = NULL;
	m_nBytes = 0;

	return pDetach;
}

bool Buffer::allocate( dword nBytes )
{
	free();

	m_nBytes = nBytes;
	m_pBuffer = malloc( m_nBytes );

	return m_pBuffer != NULL;
}

void Buffer::free()
{
	::free( m_pBuffer );
	m_pBuffer = NULL;
	m_nBytes = 0;
}

Buffer & Buffer::operator=( const Buffer & copy )
{
	if (! allocate( copy.bufferSize() ) )
		throw std::bad_alloc();

	memcpy( m_pBuffer, copy.buffer(),m_nBytes );
	return *this;
}

//----------------------------------------------------------------------------
//EOF
