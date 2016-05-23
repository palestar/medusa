/*
	FileMemoryBlocked.h

	Based on FileMemory 
	- Faster write() because the memory is allocated in blocks
	- No contigous memory accessor

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef FILEMEMORYBLOCKED_H
#define FILEMEMORYBLOCKED_H

#include "File/File.h"
#include "File/Stream.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------

class DLL FileMemoryBlocked : public File
{
public:
 	// Types
	typedef Reference< FileMemoryBlocked >		Ref;

	enum {
		BLOCK_SIZE = 1024 * 16,			// memory allocation block size in bytes
	};

   // Construction
	FileMemoryBlocked();
	FileMemoryBlocked(const void *pBytes, Size nBytes );
	FileMemoryBlocked(const FileMemoryBlocked & file );
	virtual	~FileMemoryBlocked();
    
	// File interface
    File *				clone() const; 

    bool				isReadable() const;
    bool				isWritable() const;
    Position			position() const;
    Size				size() const;

    Size				read( void * pBuffer, Size nBytes );
    Size				write( const void * pBuffer, Size nBytes );
    void				flush();
    
    Position			setPosition( Position nPos );
    Size				setSize( Size nSize ); 
    Size				setEOF();

  	void				lock();
	void				unlock();

	// Mutators
    void				open();
    void				open( const void * pBytes, Size nBytes );
	void				open( const FileMemoryBlocked & copy );
    void				close();

	void				pipe( FileMemoryBlocked & in, Size nBytes );		// copy over n bytes from the provided object at the current positions
																			// useful to get data from one FileMemoryBlocked to another without having
																			// to use another allocated buffer, and avoids the extra memcpy() (s)

	FileMemoryBlocked &	operator=( const FileMemoryBlocked & copy );

private:
    // Data
    Position			m_Pos;          // current position
    Size                m_Size;         // current size
	int					m_nBlocks;		// number of allocation blocks
    byte **             m_pBlocks;		// file data
}; 

//----------------------------------------------------------------------------



#endif

//--------------------------------------------------------------------------
// EOF
