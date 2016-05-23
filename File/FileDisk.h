/*
	FileDisk.h

	Concrete File implementation for a standard file-system file located on a hard-drive
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_DISK_H
#define FILE_DISK_H

#include "Standard/String.h"
#include "Standard/Array.h"

#include "File.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL FileDisk : public File
{
public:
    // Types
	typedef Reference< FileDisk >		Ref;

    enum AccessType	{
        READ,
		WRITE,
        READ_WRITE,
    };

    // Construction
	FileDisk();
	FileDisk(const FileDisk & copy);
	FileDisk( const char * pFileName, AccessType access = READ );
	virtual	~FileDisk();
    
    // File Interface
    File *				clone() const;

    bool				isReadable() const;
    bool				isWritable() const;
    Position			position() const;   
    Size				size() const;

    Size				read( void * p, Size nBytes ); 
    Size				write( const void * p, Size nBytes );
    void				flush();
    Position			setPosition( Position );
    Size				setSize( Size );
    Size				setEOF();
    
  	void				lock();
	void				unlock();

	// Accessors
    void *				handle() const;
    AccessType			access() const;
    const char *		fileName() const;
	
	// Mutators
	bool				open( const char * pFileName, AccessType access = READ, bool searchPath = true);
    void				close(); 

    // Static
	static void			normalizePath( char * pPath );		// normalize all the slashes in the given path the system standard
	static CharString	home();								// get the home directory (same directory as the EXE)
	static bool			seekHome();							// make the current directory the home directory
	static CharString	currentDirectory();					// get the current directory

    static void			addSearchPath(const char * pPath );
    static void			resetSearchPaths();

	static bool			fileExists( const char * file );
	static bool			fileLocked( const char * pFile );		// returns true if the file is read-only
	static bool			fileHidden( const char * pFile );		// returns true if file is hidden
	static dword		fileDate( const char * file );
	static dword		fileCreationDate( const char * file );
	static dword		fileSize( const char * file );
	static bool			deleteFile( const char * file );
	static bool			renameFile( const char * oldName, const char * newName );
	static bool			copyFile( const char * pFrom, const char * pTo );
	static bool			hideFile( const char * pFile, bool bHide );
	static bool			lockFile( const char * pFile, bool bReadOnly );
	static bool			touchFile( const char * file, dword date = 0 );

	static bool			isDirectory( const char * pFile );
	static bool			createDirectory( const char * pDir );
	static bool			removeDirectory( const char * pDir, bool bForce = false );

	static void *		loadFile( const char * file );		// allocate memory and load entire file
	static bool			saveFile( const char * file, const void * data, dword bytes );

	static char *		loadTextFile( const char * file );	// same as loadFile, except it's null terminated
	static bool			saveTextFile( const char * file, const char * pText );
	static wchar *		loadUnicodeFile( const char * pFile );
	static bool			saveUnicodeFile( const char * pFile, const wchar * pText );
	static void			unloadTextFile( char * pText );
	static void			unloadUnicodeFile( wchar * pText );

private:
    // Data
    void *              m_pHandle;
    AccessType			m_Access;
	CharString			m_sFileName;

	// Statis
	static Array< CharString >
						s_SearchPaths;
}; 

//----------------------------------------------------------------------------



#endif

//--------------------------------------------------------------------------
// EOF
