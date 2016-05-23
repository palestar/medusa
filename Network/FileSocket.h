/*
	FileSocket.h

	File interface for a socket object, this allows the use of the Stream object on a socket
	connection. 

	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_SOCKET_H
#define FILE_SOCKET_H

#include "File/File.h"
#include "Socket.h"
#include "NetworkDLL.h"

//-------------------------------------------------------------------------------

class DLL FileSocket : public File
{
public:
	// Types
	typedef Reference< FileSocket >			Ref;

    // Construction
	FileSocket();
	FileSocket( Socket * pSocket );
	FileSocket( const FileSocket & copy );
	virtual	~FileSocket();

	// File interface
    File *				clone() const; 

    bool				isReadable() const;
    bool				isWritable() const;

    Position			position() const;						// always returns 0
    Size				size() const;							// returns the size of the receive buffer

    Size				read( void * pBuffer, Size nBytes );
    Size				write( const void * pBuffer, Size nBytes );
    void				flush();
    
    Position			setPosition( Position position );
    Size				setSize( Size newSize);			
    Size				setEOF();						

	void				lock();
	void				unlock();

	// Accessors
	Socket *			socket() const;
	// Mutators
	void				setSocket( Socket * a_pSocket );

protected:
    // Data
	Socket::Ref			m_pSocket;
};

//---------------------------------------------------------------------------------------------------

inline Socket * FileSocket::socket() const
{
	return m_pSocket;
}

#endif

//-------------------------------------------------------------------------------
// EOF
