/*
	Library.h

	Dynamic Library Loader Object, unloads the library on destruction
	(c)2005 Palestar, Richard Lyle
*/

#ifndef LIBRARY_H
#define LIBRARY_H

#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning(disable:4251)	// d:\projects\hydra\standard\library.h(29) : warning C4251: 'm_Handles' : class 'Hydra::Array<void *,8>' needs to have dll-interface to be used by clients of class 'Hydra::Library'

class DLL Library
{
public:
	// Construction
	Library();	
	Library( const char * pLib, bool a_bExitOnFailure = false );	
	virtual	~Library();

	// Mutators
	bool			load( const char * pLib );
	void			free();

private:
	void *			m_pLibrary;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
