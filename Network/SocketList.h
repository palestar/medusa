/**
	@file SocketList.h
	@brief This class maintains a list of sockets and can return sockets for various states.

	(c)2012 Palestar Inc
	@author Richard Lyle @date 8/16/2012 10:12:57 PM
*/

#ifndef SOCKETLIST_H
#define SOCKETLIST_H

#include <set>
#include <list>

#include "Socket.h"
#include "NetworkDLL.h"

//---------------------------------------------------------------------------------------------------

class DLL SocketList : public Referenced
{
public:
	//! Types
	typedef Reference< SocketList >		Ref;
	
	enum UpdateFlags {
		EVENT_ACCEPTED = 0x1,
		EVENT_READY = 0x2,
		EVENT_CLOSED = 0x4,
	};

	//! Construction
	SocketList();
	virtual ~SocketList();

	//! Accessors
	dword			bytesReceived() const;
	dword			bytesSent() const;

	//! Mutators
	bool			add( Socket * a_pSocket );
	void			remove( Socket * a_pSocket );	
	void			release();

	dword			update();			// This should be called as often as possible to update socket states, returns one or more UpdateFlags
	Socket::Ref		accepted();			// returns a newly connected socket, returns NULL if no new sockets are connected.
	Socket::Ref		dataReady();		// returns a socket that has data ready to be read, returns NULL if no socket has data
	Socket::Ref		closed();			// returns any socket that may have been closed, return NULL if no sockets have been closed

private:
	//! Types
	typedef std::map< int, Socket::Ref >		SockMap;
	typedef std::list< Socket::Ref >			SockList;

	//! Data
	SockList		m_Sockets;			// linked list of all sockets
	SockList		m_Accepted;			// list of new connections
	SockList		m_Ready;			// list of connections with data ready to be read
	SockList		m_Closed;			// list of closed connections

	dword			m_nBytesSent;
	dword			m_nBytesRecv;

	// select data, m_bDirty is set when we add/remove sockets
	bool			m_bDirty;
	int				m_nNFDS;
	void *			m_pReadFD;
	void *			m_pWriteFD;
	void *			m_pErrorFD;

	void			updateSelectData();
};

//---------------------------------------------------------------------------------------------------

inline dword SocketList::bytesReceived() const
{
	dword nRecv = m_nBytesRecv;
	for( SockList::const_iterator iSocket = m_Sockets.begin();
		iSocket != m_Sockets.end(); ++iSocket )
	{
		nRecv += (*iSocket)->bytesReceived();
	}

	return nRecv;
}

inline dword SocketList::bytesSent() const
{
	dword nSent = m_nBytesSent;
	for( SockList::const_iterator iSocket = m_Sockets.begin();
		iSocket != m_Sockets.end(); ++iSocket )
	{
		nSent += (*iSocket)->bytesSent();
	}

	return nSent;
}


//---------------------------------------------------------------------------------------------------


#endif

//---------------------------------------------------------------------------------------------------
//EOF
