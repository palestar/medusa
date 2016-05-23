/*
	FileMap.h

	Memory-Mapped file object, this type of file is always write protected!
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_MAP_H
#define FILE_MAP_H

#include "Standard/String.h"
#include "File/File.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL FileMap : public File
{
public:
	// Types
	typedef Reference< FileMap >		Ref;

    // Construction
	FileMap();
	FileMap( const FileMap & );
	FileMap( const char * pFileName );
	virtual	~FileMap();
    
    // File Interface
    File *				clone() const; 

    bool				isReadable() const;
    bool				isWritable() const;
    Position			position() const;
    Size				size() const;

    Size				read( void * pBuffer, Size nBytes );
    Size				write( const void * pBuffer, Size nBytes );
    void				flush();
    
    Position			setPosition( Position );
    Size				setSize( Size newSize); 
    Size				setEOF();

  	void				lock();
	void				unlock();

	// Accessors
	void *				view() const;
	
	// Mutators
    bool				open( const char * pFilename );
    void				close(); 


private:
    // Data
	CharString			m_sFileName;
	void *				m_hMapping;		// handle to the mapping object
	void *				m_pView;		// memory-mapped view of the file
	dword				m_ViewSize;		// number of bytes mapped
	dword				m_ViewPos;		// current position

	// Static
	static void *		fileMapOpen( const char * pFile );
	static dword		fileMapSize( void * pMapping );
	static void *		fileMapPointer( void * pMapping );		// returns a pointer to the memory mapped
	static void			fileMapClose( void * pMapping );
};

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
