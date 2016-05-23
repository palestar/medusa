/*
	Sound.h
	Sound class, used to contain a waveform
	(c)2006 Palestar, Richard Lyle
*/

#ifndef SOUND_H
#define SOUND_H

#include "Resource/Resource.h"
#include "Standard/CriticalSection.h"
#include "Standard/Exception.h"
#include "Audio/AudioBuffer.h"
#include "Audio/AudioDevice.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Sound : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Exceptions
	DECLARE_EXCEPTION( FailedToLoadWave );

	// Types
	typedef ResourceLink< Sound >	Link;
	typedef Reference< Sound >	Ref;

	struct Region
	{
		String		name;		// region name
		dword		start;		// starting sample 
		dword		end;		// ending sample
	};

	// Construction
	Sound();
	Sound( const char * waveFile );
	virtual	~Sound();

	// Widget interface
	bool				read( const InStream & );
	bool				write( const OutStream & );

	// Accessors
	int					rate() const;
	int					channels() const;
	int					bits() const;	
	
	int					waveSize() const;
	void *				wave() const;

	float				time( int rate = -1 ) const;

	// Mutators
	void				initializeSound( int rate, int channels, int bits, 
							int waveSize, const void * pWave );
	void				release();

	AudioBuffer *		play( AudioDevice * pDevice, bool looped = false );
	AudioBuffer *		play( AudioDevice * pDevice, float volume, float pan, 
							bool looped = false );
	bool				exportWave( const char * pFileName );

	// Static
	static bool			loadWave( const char * file, int * pRate, int * pChannels, int *pBits, 
							int * pWaveSize, void ** pWave );
	static File *		mapWave( const char * file, int * pRate, int * pChannels, int *pBits, 
							int * pWaveSize, void ** pWave );
	static bool			saveWave( const char * pFileName, int nRate, int nChannels, int nBits,
							int nWaveSize, void * pWave );

	static bool			loadRegions( const char * file, Array< Region > & regions );

	static void			releaseAudioBuffers();

	static List< Sound * >
						s_SoundList;
	static CriticalSection
						s_SoundListLock;

private:
	// Mutators
	void				initializeAudioBuffer( AudioDevice * pDevice );

	// Data
	int					m_Rate;				// sample rate
	int					m_Channels;			// number of channels
	int					m_Bits;				// bits per sample

	int					m_WaveSize;			// size of waveform in bytes
	void *				m_pWave;			// the waveform

	// Audio Data
	AudioBuffer::Ref	m_Buffer;			// created audio buffer
};

//----------------------------------------------------------------------------

inline int Sound::rate() const
{
	return( m_Rate );
}

inline int Sound::channels() const
{
	return( m_Channels );
}

inline int Sound::bits() const
{
	return( m_Bits );
}

inline int Sound::waveSize() const
{
	return( m_WaveSize );
}

inline void * Sound::wave() const
{
	return( m_pWave );
}

inline float Sound::time( int rate /*= -1*/ ) const
{
	if ( rate < 0 )
		rate = m_Rate;

	int bytesPerSecond = rate * ((m_Bits >> 3) * m_Channels);
	return( float( m_WaveSize ) / bytesPerSecond );
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
