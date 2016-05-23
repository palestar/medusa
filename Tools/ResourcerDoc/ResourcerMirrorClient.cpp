/*
	ResourcerMirrorClient.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "stdafx.h"
#include "ResourcerMirrorClient.h"
#include "ResourcerDoc.h"

//----------------------------------------------------------------------------

void ResourcerMirrorClient::onStatus( const char * pStatus )
{
	message( pStatus );
}

void ResourcerMirrorClient::onError( const char * pError )
{
	message( pError );
}

void ResourcerMirrorClient::onFileProgress( const char * pFile, dword nTotalBytes, dword nBytes )
{
	if ( CResourcerDoc::document() != NULL )
	{
		CharString sStatus;
		sStatus.format( "Downloading %s ... %u k of %u k", pFile, nBytes / 1000, nTotalBytes / 1000 );

		CResourcerDoc::document()->setStatus( sStatus );
	}
}

int ResourcerMirrorClient::onFileConflict( const char * pFile )
{
	int nMB = MessageBox( NULL, CharString().format( "File '%s' is in conflict, do you wish to over-write the local changes?", pFile ), 
		"Conflict", MB_YESNOCANCEL );

	if ( nMB == IDYES )
		return MirrorClient::USE_REMOTE;
	else if ( nMB == IDNO )
		return MirrorClient::USE_LOCAL;

	return MirrorClient::ABORT;
}

bool ResourcerMirrorClient::onFileDownload( const char * pFile )
{
	message( CharString().format("Downloading '%s'...", pFile ) );
	return true;
}

bool ResourcerMirrorClient::onFileUpload( const char * pFile )
{
	message( CharString().format("Uploading '%s'...", pFile ) );
	return true;
}

bool ResourcerMirrorClient::onFileDelete( const char * pFile )
{
	message( CharString().format("Deleting '%s'...", pFile ) );
	return true;
}

bool ResourcerMirrorClient::onFileDeleteRemote( const char * pFile )
{
	message( CharString().format("Deleting '%s'...", pFile ) );
	return true;
}

//----------------------------------------------------------------------------

void ResourcerMirrorClient::message( const char * pMessage )
{
	if ( CResourcerDoc::document() != NULL )
		CResourcerDoc::document()->pushMessage( pMessage );
}

//----------------------------------------------------------------------------
//EOF
