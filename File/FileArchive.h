/*
	FileArchive.h

	This file object is used to write / read data from a Archive object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_ARCHIVE_H
#define FILE_ARCHIVE_H

#include "File/Archive.h"
#include "File/File.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL FileArchive : public File
{
public:
	// Types
	typedef Reference< FileArchive >		Ref;

    // Construction
	FileArchive();
	FileArchive( const FileArchive & );
	FileArchive( Archive * pArchive, const WidgetKey & file );
	virtual	~FileArchive();
    
	// File interface
    File *				clone() const; 
    bool				isReadable() const;
    bool				isWritable() const;
    Position			position() const;
    Size				size() const;

    Size				read( void * pBuffer, Size bytes );
    Size				write( const void * pBuffer, Size bytes );
    void				flush();
   
    Position			setPosition( Position nPosition );
    Size				setSize( Size nSize); 
    Size				setEOF();

	void				lock();
	void				unlock();

    // Mutators
    bool				open( Archive * pArchive, const WidgetKey & file );
    void				close(); 

	void				setVersion( dword version );
	void				setFlags( dword flags );


private:
    // Data
	Archive *			m_pArchive;
	Archive::Handle	*	m_pHandle;
}; 

//-------------------------------------------------------------------------------



#endif

//--------------------------------------------------------------------------
// EOF
