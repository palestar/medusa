/*
	FileSocket.cpp

	File interface for a socket object, this allows for using Stream object on a socket
	connection. 

	(c)2005 Palestar, Richard Lyle
*/

#include "FileSocket.h"
#include "Standard/Thread.h"
#include "Debug/Log.h"

//-------------------------------------------------------------------------------

FileSocket::FileSocket() : m_pSocket( NULL )
{}

FileSocket::FileSocket( Socket *pSocket ) : m_pSocket( pSocket )
{}

FileSocket::FileSocket( const FileSocket & copy) : m_pSocket( copy.m_pSocket ), File()
{}

FileSocket::~FileSocket()
{
	//flush();
}

//-------------------------------------------------------------------------------

File * FileSocket::clone() const
{
	return( new FileSocket( *this ) );
}

bool FileSocket::isReadable() const
{
	return true;
}

bool FileSocket::isWritable() const
{
	return true;
}

File::Position FileSocket::position() const
{
	return 0;
}

File::Size FileSocket::size() const
{
	if ( m_pSocket != NULL )
		return m_pSocket->receivedBytes();

	return 0;
}

//-------------------------------------------------------------------------------

#pragma warning( disable:4244 )

File::Size FileSocket::read( void * pBuffer, Size nBytes )
{
	if ( m_pSocket != NULL )
	{
		int nBytesRecv = 0;
		while( nBytes > 0 )
		{
			int n = m_pSocket->receive( pBuffer, nBytes );
			if ( n < 0 )
				break;
			
			nBytes -= n;
			nBytesRecv += n;
			pBuffer = ((byte *)pBuffer) + n;
			
			if ( nBytes <= 0 )
				break;

			LOG_DEBUG_HIGH( "FileSocket", "Blocking on FileSocket::read(), %u bytes", nBytes );

			// sleep then update the socket, this is a blocking read..
			Thread::sleep( 10 );
			m_pSocket->update();
		}

		return nBytesRecv;
	}
	return 0;
}

File::Size FileSocket::write( const void * pBuffer, Size nBytes )
{
	// if we have a valid socket, then attempt to send the data, do not worry 
	// if the data fails to send, any disconnection will be handled by read()
	if ( m_pSocket != NULL )
		m_pSocket->send( pBuffer, nBytes);
	return nBytes;
}

void FileSocket::flush()
{
	if ( m_pSocket != NULL )
		m_pSocket->flush();
}

File::Position FileSocket::setPosition( Position position )
{
	return 0;
}

File::Size FileSocket::setSize( Size newSize)
{
	return 0;
}

File::Size FileSocket::setEOF()
{
	return 0;
}

void FileSocket::lock()
{
	if ( m_pSocket != NULL )
		m_pSocket->lock();
}

void FileSocket::unlock()
{
	if ( m_pSocket != NULL )
		m_pSocket->unlock();
}

//---------------------------------------------------------------------------------------------------

void FileSocket::setSocket( Socket * a_pSocket )
{
	m_pSocket = a_pSocket;
}

//-------------------------------------------------------------------------------
// EOF
