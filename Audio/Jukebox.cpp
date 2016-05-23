/*
	Jukebox.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define AUDIO_DLL
#include "Audio/Jukebox.h"
#include "Standard/Library.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( Jukebox, Widget );

//----------------------------------------------------------------------------

static void LoadAudioLibs()
{
// Load all available audio devices
#ifdef _DEBUG
	static Library LIB_AUDIODS( ("AudioDSD.dll") );			// DirectSound
	static Library LIB_AUDIOPSX( ("AudioPSXD.dll") );		// PlayStation 2
	static Library LIB_AUDIOXB( ("AudioXBD.dll") );			// XBOX
#else
	static Library LIB_AUDIODS( ("AudioDS.dll") );			// DirectSound
	static Library LIB_AUDIOPSX( ("AudioPSX.dll") );			// PlayStation 2
	static Library LIB_AUDIOXB( ("AudioXB.dll") );			// XBOX
#endif
}

Jukebox * Jukebox::create()
{
	LoadAudioLibs();
	return WidgetCast<Jukebox>( Factory::createNamedWidget( "Jukebox" ) );
}

//----------------------------------------------------------------------------
//EOF
