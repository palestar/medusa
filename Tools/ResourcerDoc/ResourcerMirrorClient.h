/*
	ResourcerMirrorClient.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef RESOURCERMIRRORCLIENT_H
#define RESOURCERMIRRORCLIENT_H

#include "Network/MirrorClient.h"



//----------------------------------------------------------------------------

class ResourcerMirrorClient : public MirrorClient
{
public:
	// MirrorClient interface
	virtual void	onStatus( const char * pStatus );
	virtual void	onError( const char * pError );
	virtual void	onFileProgress( const char * pFile, 
						dword nTotalBytes, dword nBytes );

	virtual int		onFileConflict( const char * pFile );						// see MirrorClient::ConflictAction
	virtual bool	onFileDownload( const char * pFile );						// false to skip download
	virtual bool	onFileUpload( const char * pFile );							// false to skip upload
	virtual bool	onFileDelete( const char * pFile );							// false to abort local delete
	virtual bool	onFileDeleteRemote( const char * pFile );					// false to abort remote delte

	// Mutators
	void			message( const char * pMessage );
};

#endif

//----------------------------------------------------------------------------
//EOF
