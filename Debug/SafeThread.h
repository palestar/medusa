/*
	SafeThread.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef SAFETHREAD_H
#define SAFETHREAD_H

#include "Standard/Thread.h"
#include "Debug/ExceptionHandler.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL SafeThread : public Thread
{
public:
	// Construction
	SafeThread( Priority p = STANDARD, int stack = DEFAULT_STACK_SIZE );
protected:
	// Thread interface
	virtual int	runProxy();
};

#endif

//----------------------------------------------------------------------------
//EOF
