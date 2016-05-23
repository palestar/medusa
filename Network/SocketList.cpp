/**
	@file SocketList.cpp

	(c)2012 Palestar Inc
	@author Richard Lyle @date 8/17/2012 12:41:23 AM
*/

#include "Debug/Log.h"
#include "SocketList.h"
#include <algorithm>

//! define to non-zero to enable the use of select for checking the sockets..
#define ENABLE_SELECT			1

#if defined(WIN32)

#define FD_SETSIZE				1024
#include <winsock2.h>

typedef int				socklen_t;

//! Prevent windows.h pain..
#undef RGB
#define ERRNO			(WSAGetLastError())

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

typedef int				SOCKET;

#define INVALID_SOCKET			(-1)
#define SOCKET_ERROR            (-1)
#define ERRNO			(errno)

#endif

//---------------------------------------------------------------------------------------------------

SocketList::SocketList() : 
	m_bDirty( false ), 
	m_nNFDS( 0 ), 
	m_pReadFD( NULL ), 
	m_pWriteFD( NULL ),
	m_pErrorFD( NULL ),
	m_nBytesSent( 0 ),
	m_nBytesRecv( 0 )
{}

SocketList::~SocketList()
{
	if ( m_pReadFD != NULL )
		delete m_pReadFD;
	if ( m_pWriteFD != NULL )
		delete m_pWriteFD;
	if ( m_pErrorFD != NULL )
		delete m_pErrorFD;
}

bool SocketList::add( Socket * a_pSocket )
{
	if ( a_pSocket == NULL )
		return false;

	if ( std::find( m_Sockets.begin(), m_Sockets.end(), a_pSocket ) == m_Sockets.end() )
	{
		if ( m_Sockets.size() >= FD_SETSIZE )
			return false;		// at the maximum number of sockets already

		m_Sockets.push_back( a_pSocket );
		m_bDirty = true;
	}

	return true;
}

void SocketList::remove( Socket * a_pSocket )
{
	SockList::iterator iRemove = std::find( m_Sockets.begin(), m_Sockets.end(), a_pSocket );
	if ( iRemove != m_Sockets.end() )
	{
		// add in the total bytes recv/sent on this socket before removing, so our bytesReceived() & bytesSent() 
		// functions will return the total number of bytes sent by all sockets, even those removed.
		m_nBytesSent += a_pSocket->bytesSent();
		m_nBytesRecv += a_pSocket->bytesReceived();

		m_Sockets.remove( a_pSocket );
		m_bDirty = true;
	}
}

void SocketList::release()
{
	m_Sockets.clear();

	m_Accepted.clear();
	m_Ready.clear();
	m_Closed.clear();
	m_bDirty = true;
}

dword SocketList::update()
{
#if ENABLE_SELECT
	updateSelectData();

	timeval tv = { 0, 0 };
	if ( ::select( m_nNFDS, (fd_set *)m_pReadFD, (fd_set *)m_pWriteFD, (fd_set *)m_pErrorFD, &tv ) < 0 )
		LOG_ERROR( "SocketList", "select() return error. (0x%x)", ERRNO );
#endif

	dword nFlags = 0;
	for( SockList::iterator iSocket = m_Sockets.begin();
		iSocket != m_Sockets.end(); ++iSocket )
	{
		Socket * pSocket = *iSocket;

		if ( pSocket->m_Socket == INVALID_SOCKET )
		{
			m_Closed.push_back( pSocket );
			nFlags |= EVENT_CLOSED;
			continue;
		}

#if ENABLE_SELECT
		bool bError = FD_ISSET( pSocket->m_Socket, (fd_set *)m_pErrorFD ) != 0;
		bool bRecv = bError || FD_ISSET( pSocket->m_Socket, (fd_set *)m_pReadFD ) != 0;
		bool bSend = FD_ISSET( pSocket->m_Socket, (fd_set *)m_pWriteFD ) != 0;
#else
		bool bSend = true;
		bool bRecv = true;
#endif

		Socket::State eState = pSocket->update( bSend, bRecv );
		switch( eState )
		{
		case Socket::SS_LISTENING:
			while( pSocket->accepted() > 0 )
			{
				m_Accepted.push_back( pSocket->accept() );
				nFlags |= EVENT_ACCEPTED;
			}
			break;
		case Socket::SS_DISCONNECTED:
			{
				m_Closed.push_back( pSocket );
				nFlags |= EVENT_CLOSED;
			}
			break;
		case Socket::SS_CONNECTED:
			if ( pSocket->receivedBytes() > 0 )
			{
				m_Ready.push_back( pSocket );
				nFlags |= EVENT_READY;
			}
			break;
		case Socket::SS_CONNECTING:
			break;
		default:
			LOG_ERROR( "SocketList", "Unknown socket state %d", eState );
			break;
		}
	}

	return nFlags;
}

Socket::Ref SocketList::accepted()
{
	if (  m_Accepted.size() > 0 )
	{
		Socket::Ref pAccepted = m_Accepted.front();
		m_Accepted.pop_front();

		return pAccepted;
	}

	return NULL;
}

Socket::Ref SocketList::dataReady()
{
	if (  m_Ready.size() > 0 )
	{
		Socket::Ref pReady = m_Ready.front();
		m_Ready.pop_front();

		return pReady;
	}

	return NULL;
}

Socket::Ref SocketList::closed()
{
	if (  m_Closed.size() > 0 )
	{
		Socket::Ref pClosed = m_Closed.front();
		m_Closed.pop_front();

		return pClosed;
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------------

void SocketList::updateSelectData()
{
	if ( m_pReadFD == NULL )
		m_pReadFD = new fd_set;
	if ( m_pWriteFD == NULL )
		m_pWriteFD = new fd_set;
	if ( m_pErrorFD == NULL )
		m_pErrorFD = new fd_set;

	FD_ZERO( (fd_set *)m_pReadFD );
	FD_ZERO( (fd_set *)m_pWriteFD );
	FD_ZERO( (fd_set *)m_pErrorFD );

	m_nNFDS = 0;

	int i = 0;
	for( SockList::iterator iSocket = m_Sockets.begin();
		iSocket != m_Sockets.end(); ++iSocket )
	{
		Socket::Ref pSocket = *iSocket;
		if ( pSocket->m_Socket == INVALID_SOCKET )
			continue;

		// map the socket descriptor to the Socket object..
		FD_SET( pSocket->m_Socket, (fd_set *)m_pReadFD );
		FD_SET( pSocket->m_Socket, (fd_set *)m_pWriteFD );
		FD_SET( pSocket->m_Socket, (fd_set *)m_pErrorFD );
		m_nNFDS = Max( m_nNFDS, pSocket->m_Socket + 1 );
	}
}


//---------------------------------------------------------------------------------------------------
//EOF
