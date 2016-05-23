/*
	AudioCodec.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define AUDIO_DLL
#include "Audio/AudioCodec.h"
#include "Factory/FactoryTypes.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( AudioCodec, Widget );

AudioCodec::AudioCodec()
{}

//----------------------------------------------------------------------------

AudioCodec * AudioCodec::create( const ClassKey & key )
{
	return WidgetCast<AudioCodec>(Factory::createWidget( key ));
}

//----------------------------------------------------------------------------
//EOF
