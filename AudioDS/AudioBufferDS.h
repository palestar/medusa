/*
	AudioBufferDS.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef AUDIO_BUFFER_DS_H
#define AUDIO_BUFFER_DS_H

#include "Audio/AudioBuffer.h"
#include "AudioDS/AudioDeviceDS.h"

//----------------------------------------------------------------------------

class AudioBufferDS : public AudioBuffer
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< AudioBufferDS >	Ref;

	// Construction
	AudioBufferDS();
	AudioBufferDS( AudioDeviceDS * pDevice );
	AudioBufferDS( const AudioBufferDS * pDuplicate );
	virtual			~AudioBufferDS();

	// Accessors
	dword			crc() const;				// crc of the audio data
	int				size() const;				// size of buffer in bytes
	int				channels() const;			// number of channels
	int				bits() const;				// number of bits per sample
	int				rate() const;				// current sample rate
	float			volume() const;				// current volume, 0.0 - 1.0
	float			pan() const;				// current pan, 0.0 (left ) - 1.0 (right)

	bool			playing() const;			// is buffer playing
	int				position() const;			// current sample position
	bool			looping() const;			// is buffer looping

	// Mutators
	bool			initializeBuffer( int size, int channels, int bits, int rate );
	bool			release();

	bool			setRate( int rate );
	bool			setVolume( float volume );
	bool			setPan( float pan );

	bool			play( bool loop = false );	// returns false if sound cannot play, true on success
	bool			play( Stream * pStream );
	bool			stop();

	void *			lockBuffer();
	void			unlockBuffer();

	// Internal
	void			voiceEvent();

	static List< AudioBufferDS * >
					s_BufferList;
	static CriticalSection
					s_BufferListLock;

private:
	// Data
	AudioDeviceDS * 
					m_pDevice;					// our device
	LPDIRECTSOUNDBUFFER
					m_DSBuffer;					// DirectSound buffer object
	LPDIRECTSOUNDNOTIFY
					m_DSNotify;					// DirectSound notify object

	dword			m_CRC;						// CRC of the audio data
	int				m_Size;						// size of this buffer in bytes
	int				m_Bits;						// bits per sample
	int				m_Channels;					// number of channels
	int				m_Rate;						// current rate

	float			m_Volume;					// current volume
	float			m_Pan;						// current pan

	volatile bool	m_Playing;					// is buffer playing
	bool			m_Looping;					// is buffer looping
	
	bool			m_Locked;					// is buffer locked
	void *			m_pLockedWave;
	dword			m_LockedSize;

	volatile int	m_Voice;					// which voice did we get, if any
	volatile bool	m_bStopOnNextEvent;			// set to true to stop a sound on the next event received

	friend class AudioDeviceDS;
};


#endif

//----------------------------------------------------------------------------
// EOF
