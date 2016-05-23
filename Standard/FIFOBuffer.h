/*
	FIFOBuffer.h

	This object provides a FIFO for raw data. You can push() data into this buffer, then pop
	the data from the buffer in the same order it was pushed in. It's designed to handle large amounts
	of data with the minimim amount of overhead.

	This class is used by the network code which has large buffers for incoming and outgoing data 
	in compressed and uncompressed states. Data can be removed from the head without a memcpy() which
	is necessary when using the previous FileMemory::remove() function.

	Data is stored in allocated blocks (currently 2048 bytes). So it can grow in size fairly quickly,
	and those blocks are deleted from the head as data is popped.
	
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef FIFOBUFFER_H
#define FIFOBUFFER_H

#include "Standard/Types.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------

class DLL FIFOBuffer
{
public:
 	// Types
	enum {
		BLOCK_SIZE = 1024 * 64,			// memory allocation block size in bytes
	};

	// Construction
	FIFOBuffer();
	FIFOBuffer( const void *pBytes, int nBytes );
	FIFOBuffer( const FIFOBuffer & copy, int nBytes );
	FIFOBuffer( const FIFOBuffer & copy );
	virtual	~FIFOBuffer();
    
	// Accessors
    int					size() const;										// returns the number of bytes in the buffer
	int					peek( void * a_pBuffer, int a_nBytes, int a_nOffset = 0 ) const;			
																			// get data from the buffer into a user provided buffer

	int					contiguousSize() const;								// size of the amount of contigious bytes available
	void *				contiguousBytes() const;							// pointer to a block of contiguious data

	// Mutators
	int					pop( int nBytes );									// remove n bytes from the FIFO
	int					pop( void * pBuffer, int nBytes );
    int					push( const void * pBuffer, int nBytes );			// push n bytes into the FIFO
	int					push( const FIFOBuffer & buffer, int nBytes );		// push b bytes into the FIFO from another buffer
	int					pipe( FIFOBuffer & buffer, int nBytes );			// push n bytes into this FIFO from another FIFO, remove bytes from source FIFO
    int					flush();											// flush all data from the FIFO, returns the number of bytes flushed
    
    void				initialize();
    void				initialize( const void * pBytes, int nBytes );
	void				initialize( const FIFOBuffer & copy, int nBytes );
	void				initialize( const FIFOBuffer & copy );
    void				release();

	FIFOBuffer &		operator=( const FIFOBuffer & copy );

private:
    // Data
    int					m_nBottom;			// bottom byte offset
    int					m_nTop;				// top byte offset
	int					m_nBlocks;			// number of allocation blocks
    byte **				m_pBlocks;			// file data

	// Mutators
	void				grow( int nBytes );								// make sure
}; 

//----------------------------------------------------------------------------

#endif

//--------------------------------------------------------------------------
// EOF
