/**
	@file UDPServer.h
	@brief UDP Server Message Handler
	
	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/8/2006 11:28:57 PM
*/

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "Debug/SafeThread.h"
#include "Standard/Types.h"
#include "Standard/Mutex.h"
#include "File/FileFIFO.h"
#include "File/Stream.h"
#include "NetworkDLL.h"

//---------------------------------------------------------------------------------------------------

class DLL UDPServer
{
public:
	// Types
	enum {
		MAX_MTU		= 512,		// largest number of bytes that can be transmitted per UDP
		MAX_MRU		= 512,		// largest number of bytes that can be received per UDP
	};

	// File object used to contain the data for a UDP packet
	class UDP : public FileFIFO
	{
	public:
		// Types
		typedef Reference< UDP >		Ref;

		// Construction
		UDP( UDPServer * pServer, void * pSocket, u32 nAddress, u16 nPort, u8 nMessage, bool bSent = false );
		UDP( UDPServer * pServer, void * pSocket, u32 nAddress, u16 nPort, u8 nMessage, const byte * pData, int nDataSize, bool bSent = true );
		~UDP();

		// File interface
		virtual void	flush();

		// Accessors
		bool			sent() const;
		void *			socket() const;
		u32				address() const;
		u16				port() const;
		u8				message() const;

		// Mutators
		void			setSent( bool bSent );
		void			setDestination( u32 nAddress, u16 nPort );		// change the destination if this packet, set the "sent" flag to false as well
		void			setMessage( u8 nMessage );

		// sends this UDP packet to it's destination, will fail if this packet is already marked as
		// sent, so call setDestination() if you want to resend a packet to another location
		bool			send();			

	private:
		// Data
		UDPServer *		m_pServer;
		bool			m_bSent;		// if false, then a packet will be sent before this object is destroyed
		void *			m_pSocket;
		u32				m_nAddress;		// destination for this UDP packet
		u16				m_nPort;
		u8				m_nMessage;		// message ID for this UDP packet
	};

	// Construction
	UDPServer();
	~UDPServer();

	// Accessors
	bool				running() const;			// returns true if started
	unsigned int		port() const;				// get our incoming port

	dword				packetsSent() const;
	dword				bytesSentUDP() const;
	dword				packetsReceived() const;
	dword				bytesReceivedUDP() const;

	//! Start handling incoming packets on the provided port, returns false on failure
	bool				start( u16 nPort );
	//! shutdown and stop receiving packets
	bool				stop();			
	//! Increment sent stats
	void				addSent( dword nPackets, dword nBytes );
	//! Associate a client ID with an address and port
	bool				registerClient( u32 nClient, u32 nAddress, u16 nPort );
	//! Returns true if the given client ID is already registered or false if not..
	bool				isClientRegistered( u32 nClient );
	//! Remove an association for the specified client
	bool				removeClient( u32 nClient );
	//! Send a UDP packet to the specified client
	OutStream			sendUDP( u32 nClient, u8 nMessage );
	OutStream			sendUDP( u32 nAddress, u16 nPort, u8 nMessage );

	// NOTE: The virtuals below may be called from another thread, make sure you lock any needed resources

	//! Called when a packet is received from a unregistered client
	virtual void		onUDP( u32 nAddress, u16 nPort, u8 nMessage, UDP * pUDP );
	//! Called when a packet is received from a register client
	virtual void		onClientUDP( u32 nClient, u8 nMessage, UDP * pUDP );
	//! Resolve a name to a 32-bit address
	static u32			getAddress( const char * pHostName );

protected:
	class UDPThread : public SafeThread	
	{
	public:
		// Construction
		UDPThread( UDPServer * pServer );
		// Thread interface
		int			run();
	private:
		UDPServer *		m_pServer;
	};
	friend class UDPThread;

	int					receiveThread();

	// Types
	typedef Hash< u64, u32 >	AddressClientHash;
	typedef Hash< u32, u64 >	ClientAddressHash;

	// Data
	bool				m_bStarted;				// true if start() has been called
	u16					m_nPort;				// port we are listening for incoming packets on..
	UDPThread *			m_pThread;				// our incoming packet thread, blocks while waiting for incoming UDP packets
	void *				m_pSocket;				// our socket object

	Mutex				m_HashLock;				
	AddressClientHash	m_AddressClient;		// Address -> Client
	ClientAddressHash	m_ClientAddress;		// Client -> Address

	dword				m_nPacketsSent;
	dword				m_nBytesSent;
	dword				m_nPacketsReceived;
	dword				m_nBytesReceived;
};

//---------------------------------------------------------------------------------------------------

inline bool UDPServer::running() const
{
	return m_bStarted;
}

inline unsigned int UDPServer::port() const
{
	return m_nPort;
} 

inline dword UDPServer::packetsSent() const
{
	return m_nPacketsSent;
}

inline dword UDPServer::bytesSentUDP() const
{
	return m_nBytesSent;
}

inline dword UDPServer::packetsReceived() const
{
	return m_nPacketsReceived;
}

inline dword UDPServer::bytesReceivedUDP() const
{
	return m_nBytesReceived;
}

//---------------------------------------------------------------------------------------------------

inline bool UDPServer::isClientRegistered( u32 nClient )
{
	AutoLock LOCK( &m_HashLock );
	return m_ClientAddress.find( nClient ).valid();
}

//---------------------------------------------------------------------------------------------------

inline UDPServer::UDP::UDP( UDPServer * pServer, void * pSocket, u32 nAddress, u16 nPort, u8 nMessage, bool bSent /*= false*/ ) 
	: m_pServer( pServer ), m_pSocket( pSocket ), m_nAddress( nAddress ), m_nPort( nPort ), m_nMessage( nMessage ), m_bSent( bSent )
{}

inline UDPServer::UDP::UDP( UDPServer * pServer, void * pSocket, u32 nAddress, u16 nPort, u8 nMessage, const byte * pData, int nDataSize, bool bSent /*= true*/ ) 
	: m_pServer( pServer ), m_pSocket( pSocket ), m_nAddress( nAddress ), m_nPort( nPort ), m_nMessage( nMessage ), m_bSent( bSent )
{
	m_Buffer.push( pData, nDataSize );
}

inline UDPServer::UDP::~UDP()
{
	flush();
}

inline void UDPServer::UDP::flush()
{
	if ( !m_bSent )
		send();
}

inline bool UDPServer::UDP::sent() const
{
	return m_bSent;
}

inline void * UDPServer::UDP::socket() const
{
	return m_pSocket;
}

inline u32 UDPServer::UDP::address() const
{
	return m_nAddress;
}

inline u16 UDPServer::UDP::port() const
{
	return m_nPort;
}

inline u8 UDPServer::UDP::message() const
{
	return m_nMessage;
}

inline void UDPServer::UDP::setSent( bool bSent )
{
	m_bSent = bSent;
}

inline void UDPServer::UDP::setDestination( u32 nAddress, u16 nPort )
{
	m_nAddress = nAddress;
	m_nPort = nPort;
	m_bSent = false;
}

inline void UDPServer::UDP::setMessage( u8 nMessage )
{
	m_nMessage = nMessage;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
