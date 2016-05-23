/*
	Trace.h

	Tracing macros
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TRACE_H
#define TRACE_H

#include "Log.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#undef TRACE
#undef CTRACE

#if defined(_DEBUG) || defined(MEDUSA_TRACE_ON)
	#define TRACE(...)				Logging::report( __FILE__, __LINE__, LL_DEBUG_HIGH, "TRACE", __VA_ARGS__ );
	#define CTRACE(condition,...)	if (condition) { Logging::report( __FILE__, __LINE__, LL_DEBUG_HIGH, "TRACE", __VA_ARGS__ ); }
#else
	#define TRACE(...)				static_cast<void>(0)
	#define CTRACE(...)				static_cast<void>(0)
#endif

#endif

//----------------------------------------------------------------------------
// EOF
