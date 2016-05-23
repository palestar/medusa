/*
	Buffer.h

	Generic Buffer class which can be used to hold a block of data.
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef BUFFER_H
#define BUFFER_H

#include "Standard/Types.h"
#include "MedusaDll.h"

#include <string.h>		// memcpy

//----------------------------------------------------------------------------

class DLL Buffer
{
public:
	Buffer();
	Buffer( dword nBytes );
	Buffer( const Buffer & copy );
	Buffer( void * pBuffer, dword nBytes, bool bCopy = true );
	~Buffer();

	// Accessors
	void *		buffer() const;					
	dword		bufferSize() const;

	// Mutators
	void		set( void * pBuffer, dword nBytes );		// Set the internal buffers
	void *		detach();									// detaches the internal buffer, returns the buffer pointer
	bool		allocate( dword nBytes );					// allocates a new internal buffer, any previous buffer is freed
	void		free();										// frees the internal buffer

	Buffer &	operator=( const Buffer & copy );

private:
	void *		m_pBuffer;			// pointer to the buffer
	dword		m_nBytes;			// size of the buffer in bytes
};

//----------------------------------------------------------------------------

inline void * Buffer::buffer() const
{
	return m_pBuffer;
}

inline dword Buffer::bufferSize() const
{
	return m_nBytes;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
