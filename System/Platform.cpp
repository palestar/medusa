/*
	Platform.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define SYSTEM_DLL
#include "Debug/Assert.h"
#include "Standard/Library.h"
#include "System/Platform.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( Platform, Widget );

// our singleton platform object
Platform *		Platform::sm_pPlatform = NULL;

//----------------------------------------------------------------------------

Platform::Platform()
{
	ASSERT( sm_pPlatform == NULL );
	sm_pPlatform = this;
}

Platform::~Platform()
{
	ASSERT( sm_pPlatform != NULL );
	sm_pPlatform = NULL;
}

//----------------------------------------------------------------------------

const char * Platform::buttonText( Buttons a_eButton )
{
	if ( sm_pPlatform != NULL && sm_pPlatform->buttonsInverted() )
	{
		if ( a_eButton == LEFT )
			a_eButton = RIGHT;
		else if ( a_eButton == RIGHT )
			a_eButton = LEFT;
	}

	static const char * BUTTON_NAMES[] = 
	{
		"Left Button",
		"Right Button",
		"Middle Button"
	};

	return BUTTON_NAMES[ a_eButton ];
}

Platform * Platform::create()
{
#if defined(_WIN32)
#ifdef _DEBUG
	// dynamically load the correct DLL's
	static Library LIB_SYSTEMWIN( "SystemWinD.dll" );
#else
	static Library LIB_SYSTEMWIN( "SystemWin.dll" );
#endif
#endif

	return WidgetCast<Platform>( Factory::createNamedWidget( "Platform" ) );
}

//----------------------------------------------------------------------------
// EOF
