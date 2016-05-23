/*
	AudioBufferDS.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define DIRECTSOUND_VERSION		0x0700

#include "Debug/Assert.h"
#include "AudioDS/AudioBufferDS.h"

#include <math.h>

IMPLEMENT_FACTORY( AudioBufferDS, AudioBuffer );
REGISTER_FACTORY_KEY(  AudioBufferDS, 4783391674897030488 );

//-------------------------------------------------------------------------------

#define RELEASEQI( qinterface )	if (qinterface != NULL) { qinterface->Release(); qinterface = NULL; }

List< AudioBufferDS * >		AudioBufferDS::s_BufferList;
CriticalSection				AudioBufferDS::s_BufferListLock;

//----------------------------------------------------------------------------

AudioBufferDS::AudioBufferDS()
{
	m_pDevice = NULL;
	m_CRC = 0;
	m_Size = m_Channels = m_Bits = m_Rate = 0;
	m_Volume = 0.5f;
	m_Pan = 0.0f;

	m_Playing = false;
	m_Looping = false;
	m_Locked = false;
	m_pLockedWave = NULL;
	m_LockedSize = 0;
	m_Voice = -1;
	m_bStopOnNextEvent = false;

	m_DSBuffer = NULL;
	m_DSNotify = NULL;

	s_BufferListLock.lock();
	s_BufferList.insert( this );
	s_BufferListLock.unlock();
}

AudioBufferDS::AudioBufferDS( AudioDeviceDS * pDevice ) 
{
	m_pDevice = pDevice;
	m_CRC = 0;
	m_Size = m_Channels = m_Bits = m_Rate = 0;
	m_Volume = 0.5f;
	m_Pan = 0.0f;

	m_Playing = false;
	m_Looping = false;
	m_Locked = false;
	m_Voice = -1;
	m_bStopOnNextEvent = false;
	
	m_DSBuffer = NULL;
	m_DSNotify = NULL;

	s_BufferListLock.lock();
	s_BufferList.insert( this );
	s_BufferListLock.unlock();
}

AudioBufferDS::AudioBufferDS( const AudioBufferDS * pDuplicate )
{
	m_CRC = pDuplicate->m_CRC;
	m_pDevice = pDuplicate->m_pDevice;
	m_Size = pDuplicate->m_Size;
	m_Channels = pDuplicate->m_Channels;
	m_Bits = pDuplicate->m_Bits;
	m_Rate = pDuplicate->m_Rate;
	m_Volume = pDuplicate->m_Volume;
	m_Pan = pDuplicate->m_Pan;

	m_Playing = false;
	m_Looping = false;
	m_Locked = false;
	m_Voice = -1;
	m_bStopOnNextEvent = false;
	
	m_DSBuffer = NULL;
	m_DSNotify = NULL;

	// duplicate the DirectSound objects
	//
	if ( m_pDevice->m_DS->DuplicateSoundBuffer( pDuplicate->m_DSBuffer, &m_DSBuffer ) != DS_OK )
		throw AudioDevice::DeviceFailure();
	if ( m_DSBuffer->QueryInterface( IID_IDirectSoundNotify, (LPVOID *)&m_DSNotify ) != DS_OK )
		throw AudioDevice::DeviceFailure();

	s_BufferListLock.lock();
	s_BufferList.insert( this );
	s_BufferListLock.unlock();

	// A bug in the DX sound buffer duplication caused the volume to not get set correctly
	// on duplicated sound buffers, so set our volume to 0 now, so when the actual volume is
	// set DX recognizes the change.
	setVolume( 0.0f );
}

AudioBufferDS::~AudioBufferDS()
{
	s_BufferListLock.lock();
	s_BufferList.remove( this );
	s_BufferListLock.unlock();

	release();
}

//----------------------------------------------------------------------------

dword AudioBufferDS::crc() const
{
	return m_CRC;
}

int AudioBufferDS::size() const
{
	return( m_Size );
}

int AudioBufferDS::channels() const
{
	return( m_Channels );
}

int AudioBufferDS::bits() const
{
	return( m_Bits );
}

int AudioBufferDS::rate() const
{
	return( m_Rate );
}

float AudioBufferDS::volume() const
{
	return( m_Volume );
}

float AudioBufferDS::pan() const
{
	return( m_Pan );
}

bool AudioBufferDS::playing() const
{
	return( m_Playing );
}

int AudioBufferDS::position() const
{
	dword playPosition;
	if ( m_DSBuffer->GetCurrentPosition( &playPosition, NULL ) != DS_OK )
		return -1;

	return (int)playPosition;
}

bool AudioBufferDS::looping() const
{
	return( m_Looping );
}

//----------------------------------------------------------------------------

bool AudioBufferDS::initializeBuffer( int size, int channels, int bits, int rate )
{
	// store buffer information
	//
	m_CRC = 0;
	m_Size = size;
	m_Channels = channels;
	m_Bits = bits;
	m_Rate = rate;

	// create the DirectSound buffer
	//
	WAVEFORMATEX wfx;
	memset(&wfx,0,sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = m_Channels;
	wfx.nSamplesPerSec = m_Rate;
	wfx.wBitsPerSample = m_Bits;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	DSBUFFERDESC dsbdesc;
	memset(&dsbdesc,0,sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY; 
	dsbdesc.lpwfxFormat = &wfx;
	dsbdesc.dwBufferBytes = m_Size;

	if ( m_pDevice->m_DS->CreateSoundBuffer( &dsbdesc, &m_DSBuffer,NULL) != DS_OK )
		return false;
	if ( m_DSBuffer->QueryInterface( IID_IDirectSoundNotify, (LPVOID *)&m_DSNotify ) != DS_OK )
		return false;

	return true;
}

bool AudioBufferDS::release()
{
	stop();

	RELEASEQI( m_DSNotify );
	RELEASEQI( m_DSBuffer );

	return true;
}

bool AudioBufferDS::setRate( int rate )
{
	ASSERT( m_DSBuffer );

	m_Rate = rate;
	if ( m_DSBuffer->SetFrequency( m_Rate ) != DS_OK )
		return false;

	return true;
}

bool AudioBufferDS::setVolume( float volume )
{
	ASSERT( m_DSBuffer );
	ASSERT( volume >= 0 && volume <= 1 );

	m_Volume = volume;
	if ( m_DSBuffer->SetVolume( AudioDeviceDS::linearToDb( m_Volume * m_pDevice->volume() ) ) != DS_OK )
		return false;

	return true;
}

bool AudioBufferDS::setPan( float pan )
{
	ASSERT( m_DSBuffer );
	ASSERT( pan >= -1 && pan <= 1 );

	m_Pan = pan;

	int db = AudioDeviceDS::linearToDb( (float)fabs(m_Pan) );
	if ( m_Pan < 0 )
		db = -(db + 10000);
	else
		db = db + 10000;

	if ( m_DSBuffer->SetPan( db ) != DS_OK )
		return false;

	return true;
}

bool AudioBufferDS::play( bool loop /*= false*/ )
{
	ASSERT( m_DSBuffer );
	ASSERT( m_DSNotify );

	m_Looping = loop;
	if ( m_pDevice->allocateVoice( this ) < 0 )
		return false;

	m_Playing = true;
	m_pStream = NULL;

	// setup notify event so we know when the sound ends and we can free the voice
	DSBPOSITIONNOTIFY dspn;
	dspn.dwOffset = DSBPN_OFFSETSTOP;
	dspn.hEventNotify = m_pDevice->voiceEvent( m_Voice );
	
	if ( m_DSNotify->SetNotificationPositions(1, &dspn) == DS_OK &&
		m_DSBuffer->Play( 0, 0, m_Looping ? DSBPLAY_LOOPING : 0 ) == DS_OK )
	{
		// success
		return true;
	}

	// failed to play buffer
	m_pDevice->freeVoice( m_Voice );
	m_Voice = -1;
	m_Playing = false;

	return false;
}

bool AudioBufferDS::play( Stream * pStream ) 
{
	if (! m_DSBuffer ) 
		return false;
	if (! m_DSNotify )
		return false;
	if (! m_pStream )
		return false;

	m_Looping = true;
	if ( m_pDevice->allocateVoice( this ) < 0 )
		return false;

	m_Playing = true;
	m_pStream = pStream;
	m_pStream->m_pAttached = this;

	// fill the buffer before beginning play
	//
	byte *	receive = (byte *)lockBuffer();
	int		received = 0;

	while( received < m_Size )
	{
		int bytes = m_pStream->request( receive, m_Size - received );
		ASSERT( bytes > 0 );	// the client shouldn't make this buffer larger than he can provide

		received += bytes;
		receive += bytes;
	}

	unlockBuffer();

	// setup notify events
	//
	HANDLE voiceEvent = m_pDevice->voiceEvent( m_Voice );

	DSBPOSITIONNOTIFY dspn[ 3 ];
	dspn[0].dwOffset = m_Size / 2;
	dspn[0].hEventNotify = voiceEvent;
	dspn[1].dwOffset = m_Size - 1;
	dspn[1].hEventNotify = voiceEvent;
	dspn[2].dwOffset = DSBPN_OFFSETSTOP;
	dspn[2].hEventNotify = voiceEvent;
	
	if ( m_DSNotify->SetNotificationPositions(3, dspn) == DS_OK &&
		m_DSBuffer->Play( 0, 0, DSBPLAY_LOOPING ) == DS_OK)
	{
		// success
		return true;
	}

	// failure
	m_pDevice->freeVoice( m_Voice );
	m_Voice = -1;
	m_Playing = false;

	m_pStream->m_pAttached = NULL;
	m_pStream = NULL;
	
	return false;
}

bool AudioBufferDS::stop()
{
	// let the thread free the voice channel
	if ( m_DSBuffer != NULL )
		m_DSBuffer->Stop();

	return true;
}

void * AudioBufferDS::lockBuffer()
{
	ASSERT( !m_Locked );

	if ( m_DSBuffer->Lock( 0, 0, &m_pLockedWave, &m_LockedSize, 0, 0, DSBLOCK_ENTIREBUFFER ) != DS_OK )
		return NULL;

	m_Locked = true;

	ASSERT( m_Size == (int)m_LockedSize );
	return( m_pLockedWave );
}

void AudioBufferDS::unlockBuffer()
{
	ASSERT( m_Locked );

	// calculate the CRC of the waveform data before unlocking..
	m_CRC = 0;
	for(int i=0;i<(m_Size / 4);++i)
		m_CRC += ((dword *)m_pLockedWave)[i];

	if ( m_DSBuffer->Unlock( m_pLockedWave, m_LockedSize, NULL, NULL ) != DS_OK )
		throw AudioDevice::DeviceFailure();
	m_Locked = false;
}

//-------------------------------------------------------------------------------

void AudioBufferDS::voiceEvent()
{
	if ( m_Playing && m_Voice >= 0 )
	{
		dword bufferStatus;
		m_DSBuffer->GetStatus( &bufferStatus );

		// the audio device thread has called this function because our voice event has been triggered
		if ( m_pStream != NULL && (bufferStatus & DSBSTATUS_PLAYING) != 0 )
		{
			dword	bufferMid = m_Size / 2;

			void *	buffer = NULL;
			dword	bufferSize = bufferMid;

			// we are streaming, determine if we need to fill the first half or the last half of the buffer
			//
			dword	bufferStart = 0;		// assume we're filling the first half
			if ( position() < (int)bufferMid )
				bufferStart = bufferMid;	// nope, we're filling the last half

			if ( m_DSBuffer->Lock( bufferStart, bufferSize, &buffer, &bufferSize, 0, 0, 0 ) != DS_OK )
				throw AudioDevice::DeviceFailure();
			ASSERT( bufferSize == bufferMid );

			byte *	receive = (byte *)buffer;
			dword	received = 0;

			while( received < bufferSize )
			{
				dword bytes = m_pStream->request( receive, bufferSize - received );
				if ( bytes == 0 )
				{
					// stream has ended, stop this buffer from playing

					// fill the remainder with silence, so we don't get a click
					memset( receive, 0, bufferSize - received );
					// release the stream pointer
					m_pStream = NULL;
					// stop this sound on the next event
					m_bStopOnNextEvent = true;
					break;
				}

				received += bytes;
				receive += bytes;
			}

			if ( m_DSBuffer->Unlock( buffer, bufferSize, NULL, 0) != DS_OK )
				throw AudioDevice::DeviceFailure();
		}
		else 
		{
			if ( m_bStopOnNextEvent )
			{
				if ( (bufferStatus & DSBSTATUS_PLAYING) != 0 )
				{
					m_DSBuffer->Stop();						// streams stop themselves this way
					bufferStatus &= ~DSBSTATUS_PLAYING;		// clear the flag
				}
			}

			// free the voice only if the sound has stopped playing..
			if ( (bufferStatus & DSBSTATUS_PLAYING) == 0 )
			{
				m_Playing = false;
				m_pDevice->freeVoice( m_Voice );
				m_Voice = -1;
			}
		}
	}
}

//----------------------------------------------------------------------------
// EOF
