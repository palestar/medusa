/*
	Socket.h
	This class wraps a TCP stream connection.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <list>

#include "Standard/Mutex.h"
#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "Standard/FIFOBuffer.h"
#include "Standard/Reference.h"
#include "Standard/Time.h"

#include "NetworkDLL.h"

//----------------------------------------------------------------------------

class Codec;
class SocketList;

class DLL Socket : public Referenced
{
public:
	// Types
	typedef Reference<Socket>		Ref;

	enum State {
		SS_DISCONNECTED = 0,
		SS_CONNECTING,
		SS_CONNECTED,
		SS_LISTENING
	};

	// Construction
	Socket();
	Socket( const char * a_pCodecName );
	Socket( Codec * a_pCodec );
	Socket( int a_nRawSocket, Codec * a_pCodec );
	virtual	~Socket();

	//! Accessors
	State			state() const;									// get the current state of this socket
	int				accepted() const;								// number of accepted connections, called accept() to pop the new Socket object
	Codec *			codec() const;									// get the codec being used
	const char *	address() const;								// this address
	int				port() const;									// this port
	const char *	peerAddress() const;							// address of the peer
	int				peerPort() const;								// port of the peer

	dword			bytesSent() const;								// number of bytes sent
	dword			bytesReceived() const;							// number of bytes received 
	dword			connectedTime() const;							// number of seconds connected
	dword			receivedBytes() const;							// how many bytes are in the receive buffer
	dword			sendingBytes() const;							// how many bytes are in the send buffer
	dword			instanceID() const;

	//! Mutators
	bool			connect( const char * address, int port );		// initiate connection to address:port, this will block until a connection is established
	bool			listen( int port );								// start listening for incoming connections on the given port
	Socket::Ref		accept();										// grab a new accepted connection
	void			close();										// close this device

	int				send( const void * pBuffer, int nBytes );		// write data into send buffer, flush() must be called to actually send..
	void			flush();										// flush data pushed into the buffers with send(), all data will arrive atomically on the other side.

	int				peek( void * pBuffer, int nBytes );				// read data, but don't remove from the buffers
	int				pop( int nBytes );								// attempt to remove bytes from incoming buffer, returns the number of bytes actually removed
	int				receive( void * pBuffer, int nBytes );			// read data, will not block and returns the number of bytes actually read

	State			update( bool a_bSendData = true, 
						bool a_bReceiveData = true );				// update this socket, this should be called often to send & receive any data
	void			lock();
	void			unlock();

	void			setCodec( Codec * a_pCodec );					// set the compression to use, Socket will take ownership of the codec and delete it on destruction.

	//! Helper function to lookup a address by it's DNS name
	static dword	getHostAddress( const char * a_pAddress );

private:
	// Mutators
	bool			createSocket();									// creates the actual socket handles if needed
	bool			readySocket();									// ready a connected socket for communications
	bool			setBlockingMode( bool a_bBlocking );			// set the socket blocking mode
	bool			isBlocked() const;								// helper function, returns true if the last send/recv would have isBlocked	
	
	bool			sendData();
	bool			receiveData();

	enum {
		RECV_CHUNK_SIZE				= 8 * 1024,			// buffer size per recv call
		MAX_RECV_UPDATE				= 128 * 1024,		// maximum number of bytes to receive per call to update()
		CODEC_BUFFER_SIZE			= 64 * 1024,
	};

	// Data
	int				m_Socket;
	State			m_eState;
	std::list< Socket::Ref >
					m_Accepted;
	Codec *			m_pCodec;

	CharString		m_Address;
	int				m_Port;
	CharString		m_PeerAddress;
	int				m_PeerPort;


	dword			m_BytesSent;
	dword			m_BytesReceived;
	dword			m_ConnectTime;

	Mutex			m_Lock;

	FIFOBuffer		m_DecodeBuffer;			// bytes that need to be decoded
	FIFOBuffer		m_ReceiveBuffer;
	FIFOBuffer		m_EncodeBuffer;			// bytes to be encoded before sending...
	FIFOBuffer		m_SendBuffer;

	friend class SocketList;
};

//---------------------------------------------------------------------------------------------------

inline Socket::State Socket::state() const
{
	return m_eState;
}

inline int Socket::accepted() const
{
	return (int)m_Accepted.size();
}

inline Codec * Socket::codec() const
{
	return m_pCodec;
}

inline const char * Socket::address() const
{
	return( m_Address );
}

inline int Socket::port() const
{
	return( m_Port );
}

inline const char * Socket::peerAddress() const
{
	return( m_PeerAddress );
}

inline int Socket::peerPort() const
{
	return( m_PeerPort );
}

inline dword Socket::bytesSent() const
{
	return( m_BytesSent );
}

inline dword Socket::bytesReceived() const
{
	return( m_BytesReceived );
}

inline dword Socket::connectedTime() const
{
	return( Time::seconds() - m_ConnectTime );
}

inline dword Socket::receivedBytes() const
{
	return m_ReceiveBuffer.size();
}

inline dword Socket::sendingBytes() const
{
	return m_SendBuffer.size();
}

inline dword Socket::instanceID() const
{
	return reinterpret_cast<dword>( this );
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
