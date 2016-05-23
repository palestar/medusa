/*
	FileMemory.h

	File in memory
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_MEMORY_H
#define FILE_MEMORY_H

#include "File/File.h"
#include "File/Stream.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------

class DLL FileMemory : public File
{
public:
	// Types
	typedef Reference< FileMemory >		Ref;

    // Construction
	FileMemory();
	FileMemory(const void *pBytes, Size bytes, bool copy = true );
	FileMemory(const FileMemory & file );
	FileMemory(const FileMemory & file, bool copy );
	virtual	~FileMemory();
    
	// File interface
    File *				clone() const; 

    bool				isReadable() const;
    bool				isWritable() const;
    Position			position() const;
    Size				size() const;

    Size				read( void* pDest, Size nBytes );
    Size				write( const void* pSrc, Size nBytes );
    void				flush();
    
    Position			setPosition( Position nPos );
    Size				setSize( Size nSize ); 
    Size				setEOF();

  	void				lock();
	void				unlock();

	void				remove( Size nBytes );				// remove n bytes from current position

	// Accessors
    void *				bytes() const;
	// Mutators
    void				open();
    void				open( const void *pBytes, Size bytes, bool copy = true );
	void				open( const FileMemory & copy );
    void				close();

	FileMemory &		operator=( const FileMemory & copy );

private:
    // Data
    Position			m_Pos;          // current position
    Size                m_Size;         // current size
    byte *              m_pBytes;       // file data

	bool				m_Allocated;	// did this object allocate the memory
	bool				m_Blocked;		// is the memory allocated in blocks
}; 

//----------------------------------------------------------------------------

DLL const InStream & operator>>( const InStream & input, FileMemory & user );
DLL const OutStream & operator<<( const OutStream & output, const FileMemory & user );

//----------------------------------------------------------------------------



#endif

//--------------------------------------------------------------------------
// EOF
