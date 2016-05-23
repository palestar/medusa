/*
	AudioDeviceDS.h

	Abtract Interface to a audio device
	(c)2005 Palestar, Richard Lyle
*/

#ifndef AUDIO_DEVICE_DS_H
#define AUDIO_DEVICE_DS_H

//----------------------------------------------------------------------------

#include "Debug/SafeThread.h"
#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "Audio/AudioDevice.h"

// Fix for VS2005
#ifndef POINTER_64
#define POINTER_64		__ptr64
#endif

// DirectX 6.0 >
#include < DSound.h >

//----------------------------------------------------------------------------

class AudioBufferDS;	// forward declare
class AudioDeviceDS : public AudioDevice
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< AudioDeviceDS >		Ref;

	// Construction
						AudioDeviceDS();
	virtual				~AudioDeviceDS();

	// Thread interface
	int					run();
	// Accessors
	float				volume() const;			// master volume level, 0 (silence) - 1 (full)
	// Mutators
	bool				initialize( void * hWnd, 
							int voices = 32,
							int primaryRate = 44100, 
							int primaryBits = 16, 
							int primaryChannels = 2);
	void				release();

	AudioBuffer *		createBuffer();
	AudioBuffer *		duplicateBuffer( AudioBuffer * copy );

	void				setVolume( float volume );	

	bool				beginRecord( AudioListener * pListener );
	void				stopRecord();

	// When allocating voices, we use these variable to determine when not to play a sound
	// that is the same as one already playing. This is done to prevent sounds from stacking
	// causing an echo chamber effect.
	static float		sm_VoiceDupPriorityPercent;
	static float		sm_VoiceDupPositionPercent;
private:

	//-------------------------------------------------------------------------------

	class AudioThread : public SafeThread
	{
	public:
		// Construction
		AudioThread( AudioDeviceDS * pDevice ) : m_pDevice( pDevice )
		{}
		// Thread Interface
		int run() 
		{
			return( m_pDevice->voiceManager() );
		}
	private:
		AudioDeviceDS *	m_pDevice;
	};
	class CaptureThread : public SafeThread
	{
	public:
		// Construction
		CaptureThread( AudioDeviceDS * pDevice ) : m_pDevice( pDevice )
		{}
		// Thread Interface
		int run() 
		{
			return( m_pDevice->captureAudio() );
		}
	private:
		AudioDeviceDS *	m_pDevice;
	};

	//-------------------------------------------------------------------------------
	
	// Types
	typedef Hash< dword, Reference< AudioBufferDS > >	VoiceHash;

	// Mutators
	int					allocateVoice( AudioBufferDS * pBuffer );
	HANDLE				voiceEvent( int voice );
	void				freeVoice( int voice );

	int					voiceManager();
	int					captureAudio();

	// Static
	static int			linearToDb( float linear );		// linear >= 0 && linear <= 1

	// Data
	bool				m_DeviceActive;			// is device active

	LPDIRECTSOUND		m_DS;					// DirectSound interface
	LPDIRECTSOUNDBUFFER	m_DSPrimary;			// DirectSound Primary buffer
	
	float				m_Volume;				// current master volume

	CriticalSection		m_Lock;					// device lock
	AudioThread *		m_pThread;				// audio thread

	HANDLE				m_WakeEvent;			// wake the thread event
	Array< int >		m_FreeVoices;			// list of unallocated voices
	VoiceHash			m_AllocatedVoices;		// hash of allocated voices
	Array< HANDLE >		m_VoiceEvents;			// voice events

	// audio capture
	LPDIRECTSOUNDCAPTURE        
						m_DSC;					// DirectSound Capture device
	LPDIRECTSOUNDCAPTUREBUFFER
						m_DSCB;					// capture buffer
	LPDIRECTSOUNDNOTIFY
						m_DSSN;					// capture notify object

	Event	            m_CaptureEvents[ 3 ];
	int					m_CaptureRate;
	int					m_CaptureBits;
	AudioListener *		m_pAudioListener;		// capture listener
	CaptureThread *		m_pCaptureThread;

	// Friends
	friend class AudioThread;
	friend class CaptureThread;
	friend class AudioBufferDS;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

