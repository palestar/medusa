/*
	CommandTarget.cpp

	This pure base class is used to define a class as a receipient of the message heirarchy,
	which includes messages received from windows and internal messages passed by game objects

	(c)2005 PaleStar Development, Richard Lyle
*/

#define SYSTEM_DLL

#include "Debug/Trace.h"
#include "System/CommandTarget.h"
#include "Standard/StringHash.h"
#include "Standard/Limits.h"
#include "Debug/Assert.h"



//-------------------------------------------------------------------------------

Array< CommandTarget * >	CommandTarget::s_Targets;			// array of all existing command targets

//-------------------------------------------------------------------------------

CommandTarget::CommandTarget()
{
	// add the targets array
	s_Targets.push( this );
}

CommandTarget::~CommandTarget()
{
	// remove from target array
	s_Targets.removeSearch( this );
}

//-------------------------------------------------------------------------------

int CommandTarget::targetCount()
{
	return( s_Targets.size() );
}

CommandTarget *	CommandTarget::target( int n )
{
	return( s_Targets[ n ] );
}

//----------------------------------------------------------------------------

bool CommandTarget::postWindowMessage( void * hWnd, dword message, dword wparam, dword lparam )
{
	// send other messages directly
	Message msg;
	msg.message = message;
	msg.wparam = wparam;
	msg.lparam = lparam;
	msg.origin = 0;
	
	return( postMessage( msg ) );
}

bool CommandTarget::postMessage( const Message & msg )
{
	// pass message from newest to oldest command targets
	for(int i=CommandTarget::targetCount()-1;i>=0;i--)
		if ( CommandTarget::target( i )->onMessage( msg ) )
			return true;

	return false;
}

//-------------------------------------------------------------------------------
// EOF
