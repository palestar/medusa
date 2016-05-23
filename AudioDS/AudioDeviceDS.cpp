/*
	AudioDeviceDS.cpp

	Abtract Interface to a audio device
	(c)2005 Palestar, Richard Lyle
*/

#define DIRECTSOUND_VERSION		0x0700

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "AudioDS/AudioDeviceDS.h"
#include "AudioDS/AudioBufferDS.h"

//----------------------------------------------------------------------------

#ifdef HYDRA_STATIC
// allow programs to include this class by add /INCLUDE:AUDIO_DS to their linker options
extern "C" int AUDIO_DS = 1;
#endif

//----------------------------------------------------------------------------

#define RELEASEQI( qinterface )	if (qinterface != NULL) { qinterface->Release(); qinterface = NULL; }

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( AudioDeviceDS, AudioDevice );

AudioDeviceDS::AudioDeviceDS() : m_DS( NULL ), m_DSPrimary( NULL ), m_Volume( 1.0f ), m_DeviceActive( false ), m_pThread( NULL ), 
		m_DSC( NULL ), m_DSCB( NULL ), m_DSSN( NULL ), m_pCaptureThread( NULL ), m_WakeEvent( NULL )
{
	TRACE("AudioDeviceDS created!");
}

AudioDeviceDS::~AudioDeviceDS()
{
	release();

	TRACE("AudioDeviceDS destroyed!");
}

//----------------------------------------------------------------------------

float AudioDeviceDS::volume() const
{
	return( m_Volume );
}

//----------------------------------------------------------------------------

bool AudioDeviceDS::initialize( void * hWnd, 
									int voices /*= 32*/,
									int primaryRate /*= 44100 */, 
									int primaryBits /*= 16*/, 
									int primaryChannels /*= 2*/ )
{
	release();

	TRACE( "AudioDeviceDS::initializeDevice" );
	
	// create the DirectSound device
	//
	if ( DirectSoundCreate(NULL, &m_DS, NULL ) != DS_OK )
		return false;
	//if ( m_DS->SetCooperativeLevel( (HWND)hWnd, DSSCL_PRIORITY ) != DS_OK )
	//	throw DeviceFailure();
	if ( m_DS->SetCooperativeLevel( (HWND)hWnd, DSSCL_EXCLUSIVE ) != DS_OK )
		return false;

	// Get the primary sound buffer
	//
	DSBUFFERDESC dsbdesc;
	memset(&dsbdesc,0,sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME; 

	if ( m_DS->CreateSoundBuffer( &dsbdesc, &m_DSPrimary, NULL) != DS_OK )
		return false;
	
	// set the primary buffer format
	//
	PCMWAVEFORMAT pcmwf;
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = primaryChannels;
	pcmwf.wf.nSamplesPerSec = primaryRate;
	pcmwf.wf.nBlockAlign = (primaryBits >> 3) * primaryChannels;
	pcmwf.wf.nAvgBytesPerSec = primaryRate * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample = primaryBits;

	if ( m_DSPrimary->SetFormat( (LPWAVEFORMATEX)&pcmwf) != DS_OK )
		return false;

	// setup the voices
	//
	m_WakeEvent = CreateEvent(NULL,false,false,NULL);

	m_VoiceEvents.allocate( voices );
	m_FreeVoices.allocate( voices );
	for(int i=0;i<m_VoiceEvents.size();i++)
	{
		m_VoiceEvents[i] = CreateEvent(NULL,false,false,NULL);
		m_FreeVoices[i] = i;
	}

	// Start the audio thread
	//
	m_DeviceActive = true;
	m_pThread = new AudioThread( this );
	m_pThread->resume();

	return true;
}

void AudioDeviceDS::release()
{
	TRACE( "AudioDeviceDS::release" );
	if ( m_pThread != NULL )
	{
		// stop the audio thread
		m_DeviceActive = false;
		SetEvent( m_WakeEvent );

		// delete the audio thread
		delete m_pThread;
		m_pThread = NULL;
	}

	m_VoiceEvents.release();
	m_AllocatedVoices.release();
	m_FreeVoices.release();

	// release all audio buffers created by this device
	List< AudioBufferDS * >::Iterator buffers = AudioBufferDS::s_BufferList.head();
	while( buffers.valid() )
	{
		AudioBufferDS * pBuffer = *buffers;
		buffers++;

		if ( pBuffer->m_pDevice == this )
		{
			TRACE( CharString().format("AudioDeviceDS::release, deleting lost AudioBuffer %p", pBuffer) );
			delete pBuffer;
		}
	}

	// release DirectSound
	RELEASEQI( m_DSPrimary );
	RELEASEQI( m_DS );

	// release device data
	if ( m_WakeEvent != NULL )
	{
		CloseHandle( m_WakeEvent );
		m_WakeEvent = NULL;
	}

	for(int i=0;i<m_VoiceEvents.size();i++)
		CloseHandle( m_VoiceEvents[i] );
}

AudioBuffer * AudioDeviceDS::createBuffer()
{
	try {
		return( new AudioBufferDS( this ) );
	}
	catch( DeviceFailure )
	{}

	return NULL;
}

AudioBuffer * AudioDeviceDS::duplicateBuffer( AudioBuffer * copy )
{
	try {
		return( new AudioBufferDS( WidgetCast<AudioBufferDS>( copy ) ) );
	}
	catch( DeviceFailure )
	{}

	return NULL;
}

void AudioDeviceDS::setVolume( float volume )
{
	ASSERT( volume >= 0 && volume <= 1 );
	m_Volume = volume;
}

bool AudioDeviceDS::beginRecord( AudioListener * pListener )
{
	m_pAudioListener = pListener;

	// create the DS capture device, use the default capture device
    if (DirectSoundCaptureCreate(NULL, &m_DSC, NULL) != DS_OK )
        return false;

	// determine the best capture format
    DSCCAPS dsccaps;
    dsccaps.dwSize = sizeof(DSCCAPS);

	if ( m_DSC->GetCaps( &dsccaps ) != DS_OK )
		return false;

	if ( dsccaps.dwFormats & WAVE_FORMAT_1M16 ) 
	{
		m_CaptureBits = 16;
		m_CaptureRate = 11025;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_1M08 )
	{
		m_CaptureBits = 8;
		m_CaptureRate = 11025;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_2M08 )
	{
		m_CaptureBits = 8;
		m_CaptureRate = 22050;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_2M16 )
	{
		m_CaptureBits = 16;
		m_CaptureRate = 22050;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_4M08 )
	{
		m_CaptureBits = 8;
		m_CaptureRate = 44100;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_4M16 )
	{
		m_CaptureBits = 16;
		m_CaptureRate = 44100;
	}
	else if ( dsccaps.dwFormats & WAVE_FORMAT_48M16 )
	{
		m_CaptureBits = 16;
		m_CaptureRate = 48000;
	}
	else
	{
		TRACE( "AudioDeviceDS::beginRecord, no supported format!" );
		// no supported format
		stopRecord();
		return false;
	}

	WAVEFORMATEX wfx;
	memset(&wfx,0,sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = m_CaptureRate;
	wfx.wBitsPerSample = m_CaptureBits;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	DSCBUFFERDESC dscbDesc;
    dscbDesc.dwSize = sizeof(DSCBUFFERDESC);
    dscbDesc.dwFlags = 0;
    dscbDesc.dwBufferBytes = wfx.nAvgBytesPerSec * 2;
    dscbDesc.dwReserved = 0;
    dscbDesc.lpwfxFormat = &wfx;
 
	if ( m_DSC->CreateCaptureBuffer( &dscbDesc, &m_DSCB, NULL ) != DS_OK )
	{
		TRACE( "AudioDeviceDS::beginRecord, failed to create capture buffer!" );

		stopRecord();
		return false;
	}

	// lock the buffer and clear the contents
	void * pLock = NULL;
	dword lockSize = 0;
	if ( m_DSCB->Lock( 0, dscbDesc.dwBufferBytes, &pLock, &lockSize, NULL, NULL, 0 ) != DS_OK )
		throw DeviceFailure();

	memset( pLock, 0, lockSize );
	m_DSCB->Unlock( pLock, lockSize, 0, 0 );

	DSBPOSITIONNOTIFY dscbpn[ 2 ];
	dscbpn[ 0 ].dwOffset = 0;
	dscbpn[ 0 ].hEventNotify = m_CaptureEvents[ 0 ].event();
	dscbpn[ 1 ].dwOffset = dscbDesc.dwBufferBytes / 2;
	dscbpn[ 1 ].hEventNotify = m_CaptureEvents[ 1 ].event();
	
	m_DSCB->QueryInterface( IID_IDirectSoundNotify, (void **)&m_DSSN );
	if ( m_DSSN->SetNotificationPositions( 2, dscbpn ) != DS_OK )
	{
		TRACE( "AudioDeviceDS::beginRecord, failed to set notify position!" );
		stopRecord();
		return false;
	}

	// start the capture
	if ( m_DSCB->Start( DSCBSTART_LOOPING ) != DS_OK )
	{
		TRACE( "AudioDeviceDS::beginRecord, failed to begin recording!" );
		stopRecord();
		return false;
	}

	// create the capture thread
	m_CaptureEvents[ 2 ].clear();
	m_pCaptureThread = new CaptureThread( this );
	m_pCaptureThread->resume();

	return true;
}

void AudioDeviceDS::stopRecord()
{
	if ( m_DSCB != NULL )
		m_DSCB->Stop();

	if ( m_pCaptureThread != NULL )
	{
		// send the stop event
		m_CaptureEvents[ 2 ].signal();
		// delete the thread
		delete m_pCaptureThread;
		m_pCaptureThread = NULL;
	}

	RELEASEQI( m_DSSN );
	RELEASEQI( m_DSCB );
	RELEASEQI( m_DSC );
}

//-------------------------------------------------------------------------------

float AudioDeviceDS::sm_VoiceDupPriorityPercent = 0.8f;
float AudioDeviceDS::sm_VoiceDupPositionPercent = 0.1f;

int AudioDeviceDS::allocateVoice( AudioBufferDS * pBuffer )
{
	int allocatedVoice = -1;

	AutoLock lock( &m_Lock );

	if ( m_FreeVoices.size() > 0 )
	{
		allocatedVoice = m_FreeVoices.last();
		m_FreeVoices.pop();

		m_AllocatedVoices[ (dword)voiceEvent( allocatedVoice ) ] = pBuffer;
	}
	else 
	{
		// check playing sounds, steal a voice from the lowest priority sound provided
		// it's lower than this new sound..
		int nPriority = (int)(pBuffer->volume() * 100);
		int nLowestPriority = nPriority;
		int nLowestVoice = -1;

		for(int i=0;i<m_VoiceEvents.size();++i)
		{
			AudioBufferDS::Ref pPlaying = m_AllocatedVoices[ (dword)m_VoiceEvents[i] ];
			if ( pPlaying.valid() )
			{
				int nPlayingPriority = (int)(pPlaying->volume() * 100);
				if ( nPlayingPriority < nLowestPriority )
				{
					nLowestVoice = i;
					nLowestPriority = nPlayingPriority;
				}
			}
		}

		if ( nLowestVoice >= 0 )
		{
			HANDLE nVoiceEvent = voiceEvent( nLowestVoice );

			AudioBufferDS::Ref pStopBuffer = m_AllocatedVoices[ (dword)nVoiceEvent ];
			if ( pStopBuffer.valid() )
			{
				// stop the buffer directly, do not release the voice in the normal fashion..
				pStopBuffer->m_Playing = false;
				pStopBuffer->m_Voice = -1;
				pStopBuffer->m_DSBuffer->Stop();
			}

			m_AllocatedVoices[ (dword)nVoiceEvent ] = pBuffer;
			allocatedVoice = nLowestVoice;
		}
	}

	pBuffer->m_Voice = allocatedVoice;
	return( allocatedVoice );
}

HANDLE AudioDeviceDS::voiceEvent( int voice )
{
	return( m_VoiceEvents[ voice ] );
}

void AudioDeviceDS::freeVoice( int voice )
{
	m_Lock.lock();

	m_FreeVoices.push( voice );
	m_AllocatedVoices[ (dword)voiceEvent( voice ) ] = NULL;
	
	m_Lock.unlock();
}

//----------------------------------------------------------------------------

// table for converting from a linear volume, to decibels
//

int AudioDeviceDS::linearToDb( float linear )
{
	ASSERT( linear >= 0 && linear <= 1 );

	static int DbTable[101] = 
	{
		(-10000),		(-10000),		(-10000),		(-10000),		(-9287),		
		(-8643),		(-8117),		(-7673),		(-7287),		(-6947),		
		(-6643),		(-6368),		(-6117),		(-5886),		(-5673),		
		(-5473),		(-5287),		(-5112),		(-4947),		(-4791),		
		(-4643),		(-4503),		(-4368),		(-4240),		(-4117),		
		(-4000),		(-3886),		(-3777),		(-3673),		(-3571),		
		(-3473),		(-3379),		(-3287),		(-3198),		(-3112),		
		(-3029),		(-2947),		(-2868),		(-2791),		(-2716),		
		(-2643),		(-2572),		(-2503),		(-2435),		(-2368),		
		(-2304),		(-2240),		(-2178),		(-2117),		(-2058),		
		(-2000),		(-1942),		(-1886),		(-1831),		(-1777),		
		(-1724),		(-1673),		(-1621),		(-1571),		(-1522),		
		(-1473),		(-1426),		(-1379),		(-1333),		(-1287),		
		(-1242),		(-1198),		(-1155),		(-1112),		(-1070),		
		(-1029),		(-988),			(-947),			(-908),			(-868),		
		(-830),			(-791),			(-754),			(-716),			(-680),		
		(-643),			(-608),			(-572),			(-537),			(-503),		
		(-468),			(-435),			(-401),			(-368),			(-336),		
		(-304),			(-272),			(-240),			(-209),			(-178),		
		(-148),			(-117),			(-87),			(-58),			(-28),		
		(0)
	};

	return( DbTable[ (int)( linear * 100 ) ] );
}

//-------------------------------------------------------------------------------

int AudioDeviceDS::voiceManager()
{
	// build a list of events to wait for
	//
	int			eventCount = m_VoiceEvents.size() + 1;
	HANDLE *	pEvents = new HANDLE[ eventCount ];
	pEvents[ 0 ] = m_WakeEvent;

	for(int i=0;i<m_VoiceEvents.size();i++)
		pEvents[ i + 1 ] = m_VoiceEvents[i];

	// enter thread loop
	//

	while( m_DeviceActive )
	{
		int eventIndex = WaitForMultipleObjects( eventCount, pEvents, false, INFINITE ) - WAIT_OBJECT_0;
		if ( eventIndex >= 0 )
		{
			HANDLE received = pEvents[ eventIndex ];
			if ( received != m_WakeEvent )
			{
				// lock the audio system
				m_Lock.lock();

				// voice event, find the audio buffer who has that voice
				//
				AudioBufferDS::Ref buffer = m_AllocatedVoices[ (dword)received ];
				if ( buffer.valid() )
					buffer->voiceEvent();

				m_Lock.unlock();
			}
		}
	}

	delete pEvents;

	return 0;
}

int AudioDeviceDS::captureAudio()
{
	m_pAudioListener->onBeginCapture( m_CaptureRate, m_CaptureBits, 1 );

	HANDLE events[ 3 ];
	events[ 0 ] = (HANDLE)m_CaptureEvents[ 0 ].event();
	events[ 1 ] = (HANDLE)m_CaptureEvents[ 1 ].event();
	events[ 2 ] = (HANDLE)m_CaptureEvents[ 2 ].event();

	dword blockSize = ((m_CaptureBits >> 3) * m_CaptureRate);

	while( true )
	{
		int eventIndex = WaitForMultipleObjects( 3, events, false, INFINITE ) - WAIT_OBJECT_0;
		if ( eventIndex == 2 )
		{
			// send any additional captured audio to the client
			dword capture, read;
			if ( m_DSCB->GetCurrentPosition( &capture, &read ) != DS_OK )
				return -1;

			void *	locked = NULL;
			dword	lockedSize = 0;

			if ( read <= blockSize )
			{
				if ( m_DSCB->Lock( 0, read, &locked, &lockedSize, NULL, NULL, 0 ) != DS_OK )
					return -1;
			}
			else
			{
				if ( m_DSCB->Lock( blockSize, read - blockSize, &locked, &lockedSize, NULL, NULL, 0 ) != DS_OK )
					return -1;
			}

			// send the capture buffer to the AudioListener
			m_pAudioListener->onCapture( locked, lockedSize );
			m_DSCB->Unlock( locked, lockedSize, NULL, 0 );

			break;		// stop event
		}

		void *	locked = NULL;
		dword	lockedSize = 0;

		if ( eventIndex == 0 )
		{
			// lock the last half of the capture buffer
			if ( m_DSCB->Lock( blockSize, blockSize, &locked, &lockedSize, NULL, NULL, 0 ) != DS_OK )
				return -1;
		}
		else
		{
			// lock the first half of the capture buffer
			if ( m_DSCB->Lock( 0, blockSize, &locked, &lockedSize, NULL, NULL, 0 ) != DS_OK )
				return -1;
		}

		// send the capture buffer to the AudioListener
		m_pAudioListener->onCapture( locked, lockedSize );
		// unlock the capture buffer
		m_DSCB->Unlock( locked, lockedSize, NULL, 0 );
	}

	m_pAudioListener->onEndCapture();

	return 0;
}

//----------------------------------------------------------------------------
// EOF

