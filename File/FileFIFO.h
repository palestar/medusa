/*
	FileFIFO.h

	File interface for a FIFOBuffer object.
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILEFIFO_H
#define FILEFIFO_H

#include "Standard/FIFOBuffer.h"
#include "File/File.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL FileFIFO : public File
{
public:
	// Types
	typedef Reference< FileFIFO >		Ref;

    // Construction
	FileFIFO();
	FileFIFO( const FileFIFO & copy );				// clones
	FileFIFO( const FIFOBuffer & fifo );			
    virtual	~FileFIFO();

    // File interface
    File *				clone() const; 

    bool				isReadable() const;
    bool				isWritable() const;

    Position			position() const;
    Size				size() const;

    Size				read( void * pBuffer, Size nBytes );
    Size				write( const void * pBuffer, Size nBytes );
    void				flush();
	void				lock();
	void				unlock();

    Position			setPosition( Position nPos );			
    Size				setSize( Size nSize ); 
    Size				setEOF();

	FIFOBuffer &		getBuffer();

protected:
	// Data
	FIFOBuffer			m_Buffer;
}; 

//---------------------------------------------------------------------------------------------------

inline FIFOBuffer & FileFIFO::getBuffer()
{
	return m_Buffer;
}

//---------------------------------------------------------------------------------------------------

#endif

//--------------------------------------------------------------------------
// EOF
