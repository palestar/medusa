/*
	Assert.h

	Programmer error checking routines
	(c)2005 Palestar, Richard Lyle
*/

#ifndef ASSERT_H
#define ASSERT_H

#include "Error.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#undef ASSERT
#if defined(_DEBUG) || defined(MEDUSA_ASSERT_ON)
#define ASSERT(EXP)				if ( !(EXP) ) ProcessError( #EXP, __FILE__, __LINE__ )
#define ASSERT_ERR(EXP,ERR)		if ( !(EXP) ) ProcessError( ERR, __FILE__, __LINE__ )
#else
#define ASSERT(EXP)		static_cast<void>(0)
#endif

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
