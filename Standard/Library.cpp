/*
	Library.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Library.h"
#include "Debug/Log.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#endif

//----------------------------------------------------------------------------

Library::Library() : m_pLibrary( NULL )
{}

Library::Library( const char * pLib, bool a_bExitOnFailure /*= false*/ ) : m_pLibrary( NULL )
{
	if (! load( pLib ) && a_bExitOnFailure )
	{
		LOG_ERROR( "Library", "Failed to load library (%s), exiting with error.", pLib );
		exit(1);
	}
}	

Library::~Library()
{
	free();
}

//----------------------------------------------------------------------------

bool Library::load( const char * pLib )
{
#if defined(_WIN32)
	m_pLibrary = LoadLibrary( pLib );
	//if ( m_pLibrary == NULL)
	//	m_pLibrary = LoadLibraryEx( pLib, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
	return m_pLibrary != NULL;
#else
	m_pLibrary = dlopen( pLib, RTLD_LAZY );
	if (! m_pLibrary )
		LOG_ERROR( "Library", "Failed to load %s, errno = %u\n", pLib, errno );
	return m_pLibrary != NULL;
#endif
}

void Library::free()
{
#if defined(_WIN32)
	if ( m_pLibrary != NULL )
	{
		FreeLibrary( (HINSTANCE)m_pLibrary );
		m_pLibrary = NULL;
	}
#else
	if ( m_pLibrary != NULL )
	{
		dlclose( m_pLibrary );
		m_pLibrary = NULL;
	}
#endif
}

//----------------------------------------------------------------------------
// EOF
