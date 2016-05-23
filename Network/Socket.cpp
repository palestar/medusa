/*
	Socket.cpp

	Concrete implementation of the Socket interface that uses TCP stream connections..
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/SafeThread.h"
#include "Debug/Log.h"
#include "Standard/Time.h"
#include "Standard/ZLIB.h"

#include "Socket.h"

//! define to non-zero to enable TCP keep alive option
#define ENABLE_KEEP_ALIVE				0
//! Define to non-zero to support reading from legacy sockets which don't send a 0 sized block to end a message
#define ENABLE_LEGACY_SOCKETS			0
//! Define to non-zero to block socket on  close
#define ENABLE_BLOCK_ON_CLOSE			0
//! Define to non-zero to enable linger on shutdown
#define ENABLE_LINGER_ON_CLOSE			0

#if defined(WIN32)
#include <winsock2.h>

typedef int				socklen_t;

//! Prevent windows.h pain..
#undef RGB

#elif defined(_XBOX)
#include <xtl.h>
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

#endif

//----------------------------------------------------------------------------

Socket::Socket() : 
	m_Socket( INVALID_SOCKET), 
	m_eState( SS_DISCONNECTED ),
	m_pCodec( new ZLIB() ),
	m_Port( 0 ), 
	m_PeerPort( 0 ), 
	m_BytesSent( 0 ), 
	m_BytesReceived( 0 ), 
	m_ConnectTime( 0 )
{}

Socket::Socket( const char * a_pCodecName ) :
	m_Socket( INVALID_SOCKET), 
	m_eState( SS_DISCONNECTED ),
	m_pCodec( Codec::createNamed( a_pCodecName ) ),
	m_Port( 0 ), 
	m_PeerPort( 0 ), 
	m_BytesSent( 0 ), 
	m_BytesReceived( 0 ), 
	m_ConnectTime( 0 )
{}

Socket::Socket( Codec * a_pCodec ) :
	m_Socket( INVALID_SOCKET), 
	m_eState( SS_DISCONNECTED ),
	m_pCodec( a_pCodec ),
	m_Port( 0 ), 
	m_PeerPort( 0 ), 
	m_BytesSent( 0 ), 
	m_BytesReceived( 0 ), 
	m_ConnectTime( 0 )
{}

Socket::Socket( int a_nRawSocket, Codec * a_pCodec ) : 
	m_Socket( a_nRawSocket ), 
	m_eState( SS_CONNECTED ),
	m_pCodec( a_pCodec ),
	m_Port( 0 ), 
	m_PeerPort( 0 ), 
	m_BytesSent( 0 ), 
	m_BytesReceived( 0 ), 
	m_ConnectTime( Time::seconds() )
{
	if (! readySocket() )
		TRACE( "Socket::connect() - Failed to ready socket!" );
}

Socket::~Socket()
{
	// close this socket
	close();

	// free our codec
	if ( m_pCodec != NULL )
	{
		delete m_pCodec;
		m_pCodec = NULL;
	}
}

//----------------------------------------------------------------------------

bool Socket::connect( const char * pAddress, int nPort )
{
	AutoLock lock( &m_Lock );
	if ( m_eState == SS_CONNECTING 
		&& m_PeerAddress == pAddress 
		&& m_PeerPort == nPort )
	{
		LOG_DEBUG_LOW( "Socket", "connect() called on socket that is already trying to connect." );
		return true;
	}
	else if ( m_eState != SS_DISCONNECTED )
	{
		LOG_DEBUG_LOW( "Socket", "connect() called on open socket, closing current connection connection." );
		close();
	}

	if (! createSocket() )
	{
		LOG_DEBUG_LOW( "Socket", "createSocket() returned false." );
		return false;
	}

	m_PeerAddress = pAddress;
	m_PeerPort = nPort;
	m_BytesSent = m_BytesReceived = 0;
	m_ConnectTime = 0;

	// check the address for a port number, if found override the provided port number
	int coln = m_PeerAddress.reverseFind( ':' );
	if ( coln > 0 )
	{
		// get the port number
		m_PeerPort = strtol( m_PeerAddress.buffer() + coln + 1, NULL, 10 );
		// remove the port number from the end
		m_PeerAddress.left( coln );
	}

    sockaddr_in connectAddress;
    memset(&connectAddress,0,sizeof(connectAddress));   

    connectAddress.sin_family = AF_INET;
    connectAddress.sin_port = htons( m_PeerPort );
	connectAddress.sin_addr.s_addr = getHostAddress( m_PeerAddress );

	if (! readySocket() )
	{
		LOG_ERROR( "Socket", "Socket::connect() - Failed to ready socket!" );
		return false;
	}

	// make the connection, this socket should bind to an available port
	if ( ::connect( m_Socket, (sockaddr *)&connectAddress, sizeof(connectAddress)) == SOCKET_ERROR )
	{
		if (! isBlocked() )
		{
			LOG_ERROR( "Socket", "Socket::connect() - Failed to connect" );
			return false;
		}
	}

	m_eState = SS_CONNECTING;

	// made the connection
	return true;
}

bool Socket::listen( int port )
{
	AutoLock lock( &m_Lock );
	if (! createSocket() )
		return false;
	if ( m_eState == SS_LISTENING && m_Port == port )
		return true;

	// bind this socket to the port
    sockaddr_in thisAddress;
    memset(&thisAddress,0,sizeof(thisAddress));   

    thisAddress.sin_family = AF_INET;
    thisAddress.sin_addr.s_addr = INADDR_ANY;
    thisAddress.sin_port = htons( port );

	// allow the port to bind more than once
	dword reuse = 1;
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(dword) );

	// bind the port, then listen for a connection
	if ( ::bind( m_Socket, (sockaddr *)&thisAddress, sizeof(thisAddress)) != 0 )
		return false;

	if( ::listen( m_Socket, SOMAXCONN) < 0 )
		return false;
	if (! readySocket() )
		return false;
	
	m_Port = port;
	m_eState = SS_LISTENING;

	return true;
}

Socket::Ref Socket::accept()
{
	if ( m_Accepted.size() <= 0 )
		return NULL;

	Socket::Ref pSocket = m_Accepted.front();
	m_Accepted.pop_front();

	return pSocket;
}

void Socket::close()
{
	AutoLock lock( &m_Lock );

	if ( m_Socket != INVALID_SOCKET )
	{
#if ENABLE_BLOCK_ON_CLOSE
		// put socket into blocking_mode mode before calling shutdown, otherwise the socket lingers
		setBlockingMode( true );
#endif

#if defined(_WIN32)
		if ( ::shutdown( m_Socket, SD_BOTH ) < 0 )
			LOG_DEBUG_LOW( "Socket", "shutdown returned error." );
		if ( ::closesocket( m_Socket ) < 0 )
			LOG_DEBUG_LOW( "Socket", "closesocket returned error." );
#else
		if ( ::shutdown( m_Socket, SHUT_RDWR ) < 0 )
			LOG_DEBUG_LOW( "Socket", "shutdown returned error (%u).", errno );
		if ( ::close( m_Socket ) < 0 )
			LOG_DEBUG_LOW( "Socket", "close returned error (%u).", errno );
#endif
		// close may be called multiple times, if this isn't set to NULL, then a handle number 
		// already in use by another socket may get closed
		m_Socket = INVALID_SOCKET;
	}

	m_EncodeBuffer.release();
	m_DecodeBuffer.release();
	m_ReceiveBuffer.release();
	m_SendBuffer.release();
	m_eState = SS_DISCONNECTED;
	
	if ( m_pCodec != NULL )
		m_pCodec->reset();
}

int Socket::send( const void * pBuffer, int nBytes )
{
	AutoLock lock( &m_Lock );
	if ( m_Socket == INVALID_SOCKET )
		return -1;
	if ( nBytes < 1 )
		return 0;

	// push bytes into the encode buffer, flush will push bytes into the send buffer and update() will take care of sending those bytes
	// over the actual socket.
	m_EncodeBuffer.push( pBuffer, nBytes );
	m_BytesSent += nBytes;

	return nBytes;
}

void Socket::flush()
{
	AutoLock lock( &m_Lock );
	if ( m_EncodeBuffer.size() > 0 )
	{
		while( m_EncodeBuffer.size() > 0 )
		{
			dword nInBytes = m_EncodeBuffer.contiguousSize();
			if ( nInBytes > (CODEC_BUFFER_SIZE / 2) )
				nInBytes = (CODEC_BUFFER_SIZE / 2);

			// compress the data
			char sEncoded[ CODEC_BUFFER_SIZE ];		// allocate on the stack for speed..
			int nOutBytes = m_pCodec->encode( m_EncodeBuffer.contiguousBytes(), nInBytes, 
				sEncoded + sizeof(word), sizeof(sEncoded) - sizeof(word), Codec::CL_FAST );

			if ( nOutBytes <= 0 )
			{
				// compression error occur ed
				LOG_ERROR( "Socket", "Codec error has occured, failed to compress socket data." );
				close();
				return;
			}
			m_EncodeBuffer.pop( nInBytes );
			
			// store the length into the buffer
			((word *)sEncoded)[0] = (word)nOutBytes;
			// push into the send buffer for sending now..
			m_SendBuffer.push( sEncoded, nOutBytes + sizeof(word) );
		}

		// push a zero length to mark the end of the message.
		word END_MSG = 0;
		m_SendBuffer.push( &END_MSG, sizeof( END_MSG ) );
	}
}

int Socket::peek( void * pBuffer, int nBytes )
{
	AutoLock lock( &m_Lock );
	if ( m_Socket == INVALID_SOCKET )
		return -1;
	if ( nBytes < 1 )
		return 0;

	return m_ReceiveBuffer.peek( pBuffer, nBytes );
}

int Socket::pop( int nBytes )
{
	AutoLock lock( &m_Lock );
	if ( m_Socket == INVALID_SOCKET )
		return -1;
	if ( nBytes < 1 )
		return 0;

	int nBytesRemoved = m_ReceiveBuffer.pop( nBytes );
	m_BytesReceived += nBytesRemoved;

	return nBytesRemoved;
}

int Socket::receive( void * pBuffer, int nBytes )
{
	AutoLock lock( &m_Lock );
	if ( m_Socket == INVALID_SOCKET )
		return -1;
	if ( nBytes < 1 )
		return 0;

	int nBytesRecv = m_ReceiveBuffer.pop( pBuffer, nBytes );
	m_BytesReceived += nBytesRecv;

	return nBytesRecv;
}

Socket::State Socket::update(bool a_bSendData /*= true*/, bool a_bReceiveData /*= true*/)
{
	AutoLock lock( &m_Lock );
	if ( m_Socket == INVALID_SOCKET )
		return SS_DISCONNECTED;

	switch( m_eState )
	{
	case SS_CONNECTED:
		{
			if ( (a_bSendData && !sendData()) 
				|| (a_bReceiveData && !receiveData()) )
			{
				// close our socket..
				close();
			}
		}
		break;
	case SS_CONNECTING:
		{
			// socket will be connected once it becomes writable, use select() to test..
			fd_set w, f;
			FD_ZERO( &w );	
			FD_ZERO( &f );
			FD_SET( m_Socket, &w );
			FD_SET( m_Socket, &f );

			timeval tv = { 0, 0 };
			int nReady = ::select( m_Socket + 1, NULL, &w, &f, &tv );
			if ( nReady > 0 )
			{
				if ( FD_ISSET( m_Socket, &w ) )
				{
					m_eState = SS_CONNECTED;
					LOG_DEBUG_MED( "Socket", "Socket %u is connected.", m_Socket );
				}
				else if ( FD_ISSET( m_Socket, &f ) )
				{
					m_eState = SS_DISCONNECTED;
					LOG_DEBUG_MED( "Socket", "Socket %u failed to connect.", m_Socket );
				}
			}
			else if ( nReady < 0 && !isBlocked() )
			{
				LOG_DEBUG_MED( "Socket", "select returned error for socket %u", m_Socket );
				close();
			}
		}
		break;
	case SS_LISTENING:
		{
			SOCKET nNewSocket = ::accept( m_Socket, 0, 0 );
			while( nNewSocket != INVALID_SOCKET )
			{
				LOG_DEBUG_MED( "Socket", "Accepted new socket %u.", nNewSocket );
				m_Accepted.push_back( new Socket( nNewSocket, m_pCodec->clone() ) );

				nNewSocket = ::accept( m_Socket, 0, 0 );
			}
		}
		break;
	}

	return m_eState;
}

void Socket::lock()
{
	m_Lock.lock();
}

void Socket::unlock()
{
	m_Lock.unlock();
}

void Socket::setCodec( Codec * a_pCodec )
{
	if ( a_pCodec != NULL )
	{
		if ( m_pCodec != NULL && m_pCodec != a_pCodec )
			delete m_pCodec;
		m_pCodec = a_pCodec;
	}
}

//---------------------------------------------------------------------------------------------------

dword Socket::getHostAddress( const char * a_pAddress )
{
	dword nAddr = inet_addr(a_pAddress);
	if ( nAddr == INADDR_NONE )
	{
		typedef std::map< std::string, u_long > AddrCache;
		static AddrCache			addrCache;
		static CriticalSection		addrCacheLock;

		AutoLock lock( &addrCacheLock );
		AddrCache::iterator iAddr = addrCache.find( a_pAddress );
		if ( iAddr == addrCache.end() )
		{
			// the address may be a dotted address (128.38.39.1)
			hostent * pH = gethostbyname( a_pAddress );
			if ( pH == NULL )
			{
				LOG_ERROR( "Socket", "Failed to lookup host address. (%s).", a_pAddress );
				return INADDR_NONE;
			}

			addrCache[ a_pAddress ] = nAddr = ((in_addr *)(pH->h_addr))->s_addr;
		}
		else
			nAddr = iAddr->second;
	}

	return nAddr;
}

//----------------------------------------------------------------------------

bool Socket::createSocket()
{
	// create the sockets only if they have no been already
	if ( m_Socket == INVALID_SOCKET )
	{
		m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( m_Socket == INVALID_SOCKET  )
			return false;	// failed to create socket
	}

	return true;
}

bool Socket::readySocket()
{
	if ( m_Socket == INVALID_SOCKET )
	{
		TRACE( "Socket::readySocket() - No valid socket to get ready!" );
		return false;
	}

	int	addressSize = sizeof(sockaddr_in);

	// get the socket information
	//
	sockaddr_in thisAddress;
	memset(&thisAddress,0,addressSize);

	if ( getsockname( m_Socket, (sockaddr *)&thisAddress,(socklen_t *)&addressSize) == 0)
	{
		ASSERT( thisAddress.sin_family == AF_INET );

		m_Port = ntohs( thisAddress.sin_port );
		m_Address = inet_ntoa( thisAddress.sin_addr );
	}

	// get the peer information
	//
	addressSize = sizeof(sockaddr_in);

	sockaddr_in peerAddress;
	memset(&thisAddress,0,addressSize);

	if ( getpeername( m_Socket, (sockaddr *)&peerAddress,(socklen_t *)&addressSize) == 0)
	{
		ASSERT( peerAddress.sin_family == AF_INET );

		m_PeerPort = ntohs( peerAddress.sin_port );
		m_PeerAddress = inet_ntoa( peerAddress.sin_addr );
	}

	if (! setBlockingMode( false ) )
	{
		LOG_ERROR( "Socket", "Failed to put socket into non-blocking_mode mode!" );
		return false;
	}

#if ENABLE_KEEP_ALIVE
	// enable TCP keep alive so we will detect dropped connections..
	int keepalive = 1;
	if ( ::setsockopt( m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive, sizeof(keepalive) ) < 0 )
		LOG_WARNING( "Socket", "Failed to turn on SO_KEEPALIVE." );
#endif

#if ENABLE_LINGER_ON_CLOSE
	linger s_l;
	s_l.l_onoff = 1;
	s_l.l_linger = 30;
	if ( ::setsockopt( m_Socket, SOL_SOCKET, SO_LINGER, (const char *)&s_l, sizeof(s_l)) < 0 )
		LOG_ERROR( "Socket", "Failed to set SO_LINGER." );
#endif

#if !defined(_WIN32)
	// turn off SIGPIPE signal, otherwise your program just exits when a connection is closed.
	if ( ::signal(SIGPIPE, SIG_IGN) == SIG_ERR )
		LOG_WARNING( "Socket", "Failed to ignore SIGPIPE signal!" );

#if ENABLE_KEEP_ALIVE
	// after 60 seconds of idle, send up to 5 keep alive packets with 5 second intervals between before
	// declaring the socket ded.
	keepalive = 60;
	if ( ::setsockopt( m_Socket, SOL_TCP, TCP_KEEPIDLE, (char *)&keepalive, sizeof(keepalive) ) < 0 )
		LOG_WARNING( "Socket", "Failed to set TCP_KEEPIDLE." );
	keepalive = 5;
	if ( ::setsockopt( m_Socket, SOL_TCP, TCP_KEEPCNT, (char *)&keepalive, sizeof(keepalive) ) < 0 )
		LOG_WARNING( "Socket", "Failed to set TCP_KEEPCNT." );
	keepalive = 5;
	if ( ::setsockopt( m_Socket, SOL_TCP, TCP_KEEPINTVL, (char *)&keepalive, sizeof(keepalive) ) < 0 )
		LOG_WARNING( "Socket", "Failed to set TCP_KEEPINTVL." );
#endif

#endif

	return true;
}

bool Socket::setBlockingMode( bool a_bBlocking )
{
	// put socket into non-blocking_mode mode..
	dword blocking_mode = a_bBlocking ? 0 : 1;
#if defined(_WIN32)
	return ::ioctlsocket( m_Socket, FIONBIO, &blocking_mode ) == 0;
#else
	return ::ioctl( m_Socket, FIONBIO, &blocking_mode ) == 0;
#endif
}

bool Socket::isBlocked() const
{
#if !defined(_WIN32)
	return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS;
#else
	return WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAENOBUFS;
#endif
}

bool Socket::sendData()
{
	// try to send any bytes in our send buffer..
	if ( m_SendBuffer.size() <= 0 )
		return true;

	bool bError = false;
	int nSendBytes = m_SendBuffer.size();
	int nBytesSent = 0;

	while( nBytesSent < nSendBytes )
	{
		// we are in our own thread for sending.. let it block if needed, I don't care!
		int nSent = ::send( m_Socket, (const char *)m_SendBuffer.contiguousBytes(), m_SendBuffer.contiguousSize(), 0);
		if ( nSent < 0 )
		{
			if (! isBlocked() )
			{
				LOG_ERROR( "Socket", "Error on sending data, closing socket." );
				bError = true;
			}

			break;
		}
		else if ( nSent > 0 )
		{
			nBytesSent += nSent;
			m_SendBuffer.pop( nSent );
		}
	}

	return !bError;
}

bool Socket::receiveData()
{
	bool bClose = false;

	// track the number of bytes received, we cap this amount so we don't spend too much of our time in this update
	// for this single socket.
	int nBytesRecevied = 0;
	while( nBytesRecevied < MAX_RECV_UPDATE )
	{
		char sRecvBuffer[ RECV_CHUNK_SIZE ];
		int nReceived = ::recv( m_Socket, sRecvBuffer, sizeof(sRecvBuffer), 0 );
		if ( nReceived <= 0 )
		{
			if ( nReceived < 0 && !isBlocked() )
			{
				LOG_ERROR( "Socket", "Error on receiving data, closing socket." );
				bClose = true;
			}
			else if ( nReceived == 0 )
			{
				LOG_DEBUG_LOW( "Socket", "Socket closed gracefully." );
				bClose = true;
			}
			break;
		}
		else if ( nReceived > 0 )
		{
			m_DecodeBuffer.push( sRecvBuffer, nReceived );
			nBytesRecevied += nBytesRecevied;
		}
	}

	while( m_DecodeBuffer.size() > 0 )
	{
		bool bMessageReady = false;

		int nMessageSize = 0;
		while( !bMessageReady )
		{
			word nBlockSize = 0;
			if ( m_DecodeBuffer.peek( &nBlockSize, sizeof(word), nMessageSize ) != sizeof(word) )
				break;

			// we are looking for a zero length to mark the end of the message..
			if ( nBlockSize == 0 )
				bMessageReady = true;

			nBlockSize += sizeof(word);
			nMessageSize += nBlockSize;

			if ( m_DecodeBuffer.size() < nMessageSize )
				break;		// not enough data received to decode message yet..

#if ENABLE_LEGACY_SOCKETS
			if ( m_DecodeBuffer.size() >= nMessageSize )
				bMessageReady = true;
#endif
		}

		if (! bMessageReady )
			break;

		while( nMessageSize > 0 )
		{
			word nBlockSize = 0;
			if ( m_DecodeBuffer.pop( &nBlockSize, sizeof(word) ) != sizeof(word) )
				break;

			if ( nBlockSize > 0 )
			{
				char sDecode[ CODEC_BUFFER_SIZE ];
				m_DecodeBuffer.pop( sDecode, nBlockSize );

				char sDecoded[ CODEC_BUFFER_SIZE / 2 ];
				int nBytesDecoded = m_pCodec->decode( sDecode, nBlockSize, 
					sDecoded, sizeof(sDecoded) );

				if ( nBytesDecoded < 0 )
				{
					LOG_ERROR( "Socket", "Error on socket %p decoding received bytes!", this );
					bClose = true;
					break;
				}

				m_ReceiveBuffer.push( sDecoded, nBytesDecoded );
			}

			nMessageSize -= (nBlockSize + sizeof(word));
		}
	}

	return !bClose;
}

//----------------------------------------------------------------------------
// EOF

