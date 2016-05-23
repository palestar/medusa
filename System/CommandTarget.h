/*
	CommandTarget.h

	This pure base class is used to define a class as a receipient of the message heirarchy,
	which includes messages received from windows and internal messages passed by game objects

	(c)2005 PaleStar Development, Richard Lyle
*/


#ifndef COMMANDTARGET_H
#define COMMANDTARGET_H

#include "Standard/Array.h"
#include "Standard/Types.h"
#include "Standard/Point.h"
#include "System/Messages.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning( disable: 4251 )

class DLL CommandTarget
{
public:
	// Types
	struct Message
	{
		// Data
		dword		message;
		dword		wparam;
		dword		lparam;
		dword		origin;
	};

	// Construction
							CommandTarget();
	virtual					~CommandTarget();

	virtual bool			onMessage( const Message & msg ) = 0;

	// Static
	static int				targetCount();
	static CommandTarget *	target( int n );

	static bool				postWindowMessage( void * hWnd, dword message, 
								dword wparam, dword lparam );
	static bool				postMessage( const Message & msg );

private:
	// Static Data
	static Array< CommandTarget * >
							s_Targets;			// array of all existing command targets
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
