/*
	AudioCodec.h

	This class is used to compress/decompress audio data
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef AUDIOCODEC_H
#define AUDIOCODEC_H

#include "Standard/Reference.h"
#include "Standard/Buffer.h"
#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

const ClassKey	CLASS_KEY_AUDIO_CODEC( LL(4179117189762823764) );
const ClassKey	CLASS_KEY_AUDIO_CODEC_SPEECH( LL(4179117239384440478) );		// Speech compression
const ClassKey	CLASS_KEY_AUDIO_CODEC_MP3( LL(4179117239384440478) );			// MP3 compression

//----------------------------------------------------------------------------

class DLL AudioCodec : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Exceptions
	class			Failure {};

	// Types
	struct PCM
	{
		int			rate;
		int			bits;
		int			channels;
	};
	typedef Reference< AudioCodec >		Ref;

	// Construction
	AudioCodec();

	// Mutators
	virtual bool			initialize( PCM & format ) = 0;					// initialize this codec to take this type of PCM data
	virtual bool			release() = 0;

	virtual bool 			encode( const Buffer & input, 
								Buffer & output ) = 0;						// encode from PCM data
	virtual bool			decode( const Buffer & input,
								Buffer & output ) = 0;						// decode to PCM data

	// Static
	static AudioCodec *		create( const ClassKey & key );
};

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
