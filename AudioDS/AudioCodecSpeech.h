/*
	AudioCodecSpeech.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef AUDIOCODECSPEECH_H
#define AUDIOCODECSPEECH_H

#include "Audio/AudioCodec.h"

// Fix for VS2005
#ifndef POINTER_64
#define POINTER_64		__ptr64
#endif

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>  // Multimedia registration
#include <msacm.h>	// Audio Compression Manager

//----------------------------------------------------------------------------

class AudioCodecSpeech : public AudioCodec
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
					AudioCodecSpeech();
	virtual			~AudioCodecSpeech();

	// AudioCodec interface
	bool			initialize( PCM & format );	// initialize this codec to take this type of PCM data
	bool			release();

	bool			encode( const Buffer & input, Buffer & output );		
	bool			decode( const Buffer & input, Buffer & output );

private:
	// Data
	HACMDRIVER		m_hDriver;					// ACM driver handle

	HACMSTREAM		m_PreEncodeStream;
	HACMSTREAM		m_EncodeStream;

	HACMSTREAM		m_DecodeStream;
	HACMSTREAM		m_FinalDecodeStream;

	WAVEFORMATEX	m_DecodedWF;				// decoded
	
	WAVEFORMATEX *	m_PreEncodeWF;				// pre-encode format
	WAVEFORMATEX *	m_EncodedWF;				// encoded format

	dword			calculateBufferSize( dword srcSize, 
						WAVEFORMATEX * pSrcWF, WAVEFORMATEX * pDstWF ) const;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF


