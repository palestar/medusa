/*
	File.h

	File Abstract class
	Concrete versions: FileDisk and FileMemory

	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_H
#define FILE_H

#include "Standard/Types.h"
#include "Standard/Exception.h"
#include "Standard/Reference.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL File : public Referenced
{
public:
	// Exceptions
	DECLARE_EXCEPTION( FileError );

	// Types
	typedef Reference< File >		Ref;
    typedef dword					Position;
    typedef Position				Size;

    // Construction
	File();
    virtual	~File();

    // Accessors
    virtual File *		clone() const = 0;								// clone this file, will never return NULL, will return a this pointer if file isn't cloneable

    virtual bool		isReadable() const = 0;							// can bytes be read from this file
    virtual bool		isWritable() const = 0;							// is this file writable, false if not..

    virtual Position	position() const = 0;							// current file read/write position
    virtual Size		size() const = 0;								// size of the file in bytes

    // Mutators
    virtual Size		read( void * pBuffer, Size nBytes ) = 0;
    virtual Size		write( const void * pBuffer, Size nBytes ) = 0;
    virtual void		flush() = 0;

    virtual void		insert( const void * pBuffer, Size nBytes ); 
    virtual void		append( const void * pBuffer, Size nBytes );
    virtual void		remove( Size nBytes ); 
    
    virtual Position	setPosition( Position nPos ) = 0;
    virtual Size		setSize( Size nSize ) = 0; 
    virtual Size		setEOF() = 0;

	virtual void		lock() = 0;										// write lock interface
	virtual void		unlock() = 0;

	// Helpers
	Position			rewind( int nBytes = -1 );
	Position			skip( int nBytes = 1 );
	void				end();
}; 

//----------------------------------------------------------------------------

inline File::Position File::rewind( int nBytes /*= -1*/ )
{
	if ( nBytes < 0 )
		return setPosition(0);
	return setPosition( position() - nBytes );
}

inline File::Position File::skip( int nBytes /*= 1*/ )
{
	return setPosition( position() + nBytes );
}

inline void File::end()
{
	setPosition( size() );
}

//----------------------------------------------------------------------------




#endif

//--------------------------------------------------------------------------
// EOF
