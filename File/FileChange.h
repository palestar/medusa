/*
	FileChange.h

	This class is used to wait for a directory/file change, written for the MirrorServer
	who uses this to wait for a change to the directory structure.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_CHANGE_H
#define FILE_CHANGE_H

#include "Standard/Types.h"
#include "Standard/Exception.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL FileChange
{
public:
	// Exceptions
	DECLARE_EXCEPTION( FileChangeFailure );

	// Construction
	FileChange();
	FileChange( const char * a_pDirectory );

	virtual	~FileChange();

	// Mutators
	bool				open( const char * a_pDirectory );
	void				close();
	void				wait();
	bool				signaled();

private:
	void *				m_pHandle;

	// File Change
	static void *		fileChangeOpen( const char * a_pDirectory );	// opens a file change handle, to watch for files that change
	static bool			fileChangeWait( void * pFile );					// waits for files to change
	static bool			fileChangePoll( void * pFile );					// checks if any files have changed, returns true if they have
	static void			fileChangeClose( void * pFile );	
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
