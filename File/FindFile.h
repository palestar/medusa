/*
	FindFile.h

	This object is used to find files on the disk using a mask
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FIND_FILE_H
#define FIND_FILE_H

#include "Standard/String.h"
#include "Standard/Array.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning( disable: 4251 ) // d:\projects\hydra\file\findfile.h(38) : warning C4251: 'm_Files' : class 'Hydra::Array<class Hydra::String,8>' needs to have dll-interface to be used by clients of class 'Hydra::FindFile'

class DLL FindFile
{
public:
	// Construction
	FindFile();
	FindFile( const char * fileMask, 
		bool bFindHidden = false, bool bFullPath = false );
	virtual	~FindFile();

	// Accessors
	int				fileCount() const;
	const char *	file( int n ) const;
	int				directoryCount() const;
	const char *	directory( int n ) const;

	// Mutators
	void			findFiles( const char * fileMask, 
						bool bFindHidden = false, 
						bool bFullPath = false );
	void			sortFiles();
	void			sortDirectories();
	void			release();

private:
	// Data
	Array< CharString >	
					m_Files;
	Array< CharString >	
					m_Directories;

	// Functions
	static void *	findOpen( const char * pMask );
	static void		findName( void * pFind, 
						char * pName, int nMaxName );		// returns the file name
	static dword	findTime( void * pFind );				// returns the time in seconds when the file was last modified
	static bool		findIsLocked( void * pFind );			// is the current file read only
	static bool		findIsHidden( void * pFind );			// is the current file hidden
	static bool		findIsDirectory( void * pFind );		// is the current file a directory
	static void *	findNext( void * pFind );				// returns the new find handle or NULL if no more files, no need to call closeFindFile() afterwards
	static void		findClose( void * pFind );
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
