/**
	@file UDPServer.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/10/2006 8:50:17 PM
*/

#include "UDPServer.h"
#include "Socket.h"
#include "Debug/Trace.h"

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

typedef int				SOCKET;

#define INVALID_SOCKET			(-1)
#define SOCKET_ERROR            (-1)

#endif

//---------------------------------------------------------------------------------------------------

// File interface
bool UDPServer::UDP::send()
{
	if ( m_bSent )
		return false;		// packet has already been sent
	if ( !address() || !port() )
		return false;		// invalid address or port
	if (! m_pSocket )
		return false;		// no valid socket to send

	int nDataSize = m_Buffer.size();
	if ( nDataSize > (MAX_MTU - 1) )
		return false;		// data is too large...

	// get data from FIFO for sending over socket, we're using peek so the
	// user can send the same UDP packet again to another destination..
	byte Data[ MAX_MTU ];
	Data[ 0 ] = m_nMessage;
	if ( nDataSize > 0 )
		m_Buffer.peek( Data + 1, nDataSize );

    sockaddr_in Address;
    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = htonl( address() );
    Address.sin_port = htons( port() );

	int nSent = ::sendto( (SOCKET)m_pSocket, (const char *)Data, nDataSize + 1, 0, (sockaddr *)&Address, sizeof(Address) );
	if ( nSent < 0 )
		return false;		// some type of error has occurred while trying to send..
	if ( nSent != (nDataSize + 1) )
		return false;		// failed to send all the bytes!
	if ( m_pServer )
		m_pServer->addSent( 1, nSent );

	// mark this packet as sent now..
	m_bSent = true;
	return true;
}

//---------------------------------------------------------------------------------------------------

UDPServer::UDPServer() : m_bStarted( false ), 
	m_nPort( 0 ), 
	m_pThread( NULL ), 
	m_pSocket( NULL ),
	m_nPacketsSent( 0 ),
	m_nBytesSent( 0 ),
	m_nPacketsReceived( 0 ),
	m_nBytesReceived( 0 )
{}

UDPServer::~UDPServer()
{
	stop();
}

bool UDPServer::start( u16 nPort )
{
	if ( m_bStarted )
		return false;

	m_bStarted = true;
	m_nPort = nPort;

	SOCKET nSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( nSocket == INVALID_SOCKET  )
		return false;	// failed to create socket
	m_pSocket = (void *)nSocket;

	// bind this socket to the port
    sockaddr_in thisAddress;
    memset(&thisAddress,0,sizeof(thisAddress));   

    thisAddress.sin_family = AF_INET;
    thisAddress.sin_addr.s_addr = INADDR_ANY;
    thisAddress.sin_port = htons( m_nPort );

	// bind the port, then listen for a connection
	if ( ::bind( (SOCKET)m_pSocket, (sockaddr *)&thisAddress, sizeof(thisAddress)) != 0 )
		return false;		// failed to bind!

	// start the thread that will actually block to receive ALL incoming packets for this server...
	m_pThread = new UDPThread( this );
	if (! m_pThread )
		return false;		// not really going to happen, because our memory allocate with throw an exception if a memory allocation fails..
	m_pThread->resume();
	
	return true;
}

bool UDPServer::stop()
{
	if (! m_bStarted )
		return false;

	m_nPort = 0;
	m_bStarted = false;

	if ( m_pSocket )
	{
		// NOTE: Closing the socket needs to be done first, this causes the thread if blocking to unblock so it
		// can exit cleanly...
#if defined(_WIN32)
		closesocket( reinterpret_cast<SOCKET>( m_pSocket ) );
#else
		::close( reinterpret_cast<SOCKET>( m_pSocket ) );
#endif
		m_pSocket = NULL;
	}

	if ( m_pThread )
	{
		delete m_pThread;
		m_pThread = NULL;
	}


	m_AddressClient.release();
	m_ClientAddress.release();

	return true;
}

void UDPServer::addSent( dword nPackets, dword nBytes )
{
	m_nPacketsSent += nPackets;
	m_nBytesSent += nBytes;
}

//! Associate a client ID with an address and port
bool UDPServer::registerClient( u32 nClient, u32 nAddress, u16 nPort )
{
	AutoLock LOCK( &m_HashLock );
	if ( m_ClientAddress.find( nClient ).valid() )
		removeClient( nClient );
	u64 nHash = (((u64)nPort) << 32) | nAddress;
	if( m_AddressClient.find( nHash ).valid() )
		return false;		// address already registered to another client!

	m_ClientAddress[ nClient ] = nHash;
	m_AddressClient[ nHash ] = nClient;
	return true;
}

//! Remove an association for the specified client
bool UDPServer::removeClient( u32 nClient )
{
	AutoLock LOCK( &m_HashLock );
	ClientAddressHash::Iterator iAddress = m_ClientAddress.find( nClient );
	if (! iAddress.valid() )
		return false;

	m_AddressClient.remove( *iAddress );
	m_ClientAddress.remove( nClient );
	return true;
}

//! Send a UDP packet to the specified client
OutStream UDPServer::sendUDP( u32 nClient, u8 nMessage )
{
	u64 nHash = 0;

	m_HashLock.lock();
	ClientAddressHash::Iterator iAddress = m_ClientAddress.find( nClient );
	if ( iAddress.valid() )
		nHash = *iAddress;
	m_HashLock.unlock();

	u16 nPort = (u16)((nHash >> 32) & 0xffff);
	u32 nAddress = (u32)(nHash & 0xffffffff);

	return sendUDP( nAddress, nPort, nMessage );
}

OutStream UDPServer::sendUDP( u32 nAddress, u16 nPort, u8 nMessage )
{
	return OutStream( new UDP( this, m_pSocket, nAddress, nPort, nMessage ) );
}

//! Called when a packet is received from a unregistered client
void UDPServer::onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP )
{}

//! Called when a packet is received from a register client
void UDPServer::onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP )
{}

//---------------------------------------------------------------------------------------------------

u32 UDPServer::getAddress( const char * pHostName )
{
    sockaddr_in Address;
    memset(&Address,0,sizeof(Address));   

    Address.sin_family = AF_INET;
    Address.sin_port = 0;
	Address.sin_addr.s_addr = Socket::getHostAddress( pHostName );

	return ntohl( Address.sin_addr.s_addr );
}

//---------------------------------------------------------------------------------------------------

UDPServer::UDPThread::UDPThread( UDPServer * pServer ) : SafeThread( HIGH, DEFAULT_STACK_SIZE ), m_pServer( pServer )
{}

int UDPServer::UDPThread::run()
{
	return m_pServer->receiveThread();
}

int UDPServer::receiveThread()
{
	LOG_STATUS( "UDPServer", "UDP Receive thread is starting..." );
	while( m_bStarted )
	{
		byte Data[ MAX_MRU ];

		sockaddr_in Address;
		int nAddressSize = sizeof(Address);

		int nReceived = ::recvfrom( (SOCKET)m_pSocket, (char *)Data, sizeof(Data), 0, (sockaddr *)&Address, (socklen_t *)&nAddressSize );
		if ( nReceived < 1 )
			continue;		// error while receiving packet..

		m_nPacketsReceived += 1;
		m_nBytesReceived += nReceived;

		u32 nAddress = ntohl( Address.sin_addr.s_addr );
		u16 nPort = ntohs( Address.sin_port );
		u64 nHash = (((u64)nPort) << 32) | nAddress;

		// The first byte of each received packet is the message ID
		u8 nMessage = Data[ 0 ];
		// build a UDP object for the remaining data in the buffer, may be 0 bytes...
		UDP::Ref pUDP = new UDP( this, m_pSocket, nAddress, nPort, nMessage, Data + 1, nReceived - 1 );

		// look up which client sent this UDP packet in our Hash... 
		u32 nClient = 0;

		m_HashLock.lock();
		AddressClientHash::Iterator iClient = m_AddressClient.find( nHash );
		if ( iClient.valid() )
			nClient = *iClient;
		m_HashLock.unlock();

		if ( nClient != 0 )
			onClientUDP( nClient, nMessage, pUDP );
		else
			onUDP( nAddress, nPort, nMessage, pUDP );
	}
	LOG_STATUS( "UDPServer", "UDP Receive thread is stopping..." );

	return 0;
}


//---------------------------------------------------------------------------------------------------
//EOF
