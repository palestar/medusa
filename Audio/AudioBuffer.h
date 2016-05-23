/*
	AudioBuffer.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL AudioBuffer : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< AudioBuffer >	Ref;

	class DLL Stream 
	{				
	public:
		virtual ~Stream() 
		{
			if ( m_pAttached != NULL && m_pAttached->m_pStream == this )
				m_pAttached->m_pStream = NULL;
		}

		// this class is used by the client to provide audio data to a audio buffer on demand
		// inherit from this class, and store your audio data in the pointers when request() is called.
		virtual int		request( void * pDst, int dstSize ) = 0;

		// Return the amount of bytes actually stored.

		//! Buffer this Stream is attached onto, it will clear itself from that buffer on destruction.
		AudioBuffer *	m_pAttached;
	};

	//! Construction
	AudioBuffer() : m_pStream( NULL )
	{}

	// Accessors
	virtual dword		crc() const = 0;		// CRC of the audio data
	virtual int			size() const = 0;		// size of buffer in bytes
	virtual int			channels() const = 0;	// number of channels
	virtual int			bits() const = 0;		// number of bits per sample
	virtual int			rate() const = 0;		// current sample rate
	virtual float		volume() const = 0;		// current volume, 0.0 - 1.0
	virtual float		pan() const = 0;		// current pan, -1 (left ) - 1 (right)

	virtual bool		playing() const = 0;	// is buffer playing
	virtual int			position() const = 0;	// current sample position
	virtual bool		looping() const = 0;	// is buffer looping

	// Mutators
	virtual bool		initializeBuffer( int size, int channels, int bits, int rate ) = 0;
	virtual bool		release() = 0;

	virtual bool		setRate( int rate ) = 0;
	virtual bool		setVolume( float volume ) = 0;
	virtual bool		setPan( float pan ) = 0;

	virtual bool		play( bool loop = false ) = 0;	// returns false if sound cannot play, true on success
	virtual bool		play( Stream * pStream ) = 0;
	virtual bool		stop() = 0;

	virtual void *		lockBuffer() = 0;
	virtual void		unlockBuffer() = 0;

	// Helpers
	int					bytes() const;			// number of bytes per sample
	int					samples() const;		// sample length of this buffer
	int					time() const;			// time in ms of this buffer at the current rate

protected:
	Stream *			m_pStream;
};

//----------------------------------------------------------------------------

inline int AudioBuffer::bytes() const
{
	return( (bits() + 7) / 8 );
}

inline int AudioBuffer::samples() const
{
	return( size() / (bytes() * channels()) );
}

inline int AudioBuffer::time() const
{
	return( (samples() * 1000U) / rate() );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
