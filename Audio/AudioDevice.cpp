/*
	AudioDevice.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define AUDIO_DLL
#include "Standard/Library.h"
#include "Audio/Sound.h"
#include "Audio/AudioDevice.h"
#include "Factory/FactoryTypes.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( AudioDevice, Widget );
IMPLEMENT_ABSTRACT_FACTORY( AudioBuffer, Widget );

AudioDevice::AudioDevice() 
{}

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

//----------------------------------------------------------------------------

AudioDevice * AudioDevice::create()
{
	LoadAudioLibs();
	if ( Factory::typeCount( classKey() ) > 0 )
		return WidgetCast<AudioDevice>(Factory::createWidget( Factory::type( classKey(), 0 ) ));

	return NULL;
}

AudioDevice * AudioDevice::create( const char * pClass )
{
	LoadAudioLibs();
	return WidgetCast<AudioDevice>(Factory::createNamedWidget( pClass ));
}

//----------------------------------------------------------------------------
// EOF

