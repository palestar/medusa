/*
	JukeboxDS.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "File/Path.h"
#include "Standard/Settings.h"
#include "File/FindFile.h"
#include "File/FileDisk.h"
#include "JukeboxDS.h"

// Fix for VS2005
#ifndef POINTER_64
#define POINTER_64		__ptr64
#endif

#include <dshow.h>		// DirectShow

//----------------------------------------------------------------------------

#define RELEASEQI( qinterface )	if (qinterface != NULL) { qinterface->Release(); qinterface = NULL; }

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( JukeboxDS, Jukebox );

JukeboxDS::JukeboxDS() : m_Active( false ), m_Volume( 100 ), m_pPlayerThread( NULL ), m_ActiveList( -1 ), m_CurrentTrack( -1 )
{}

JukeboxDS::~JukeboxDS()
{
	release();
}

//----------------------------------------------------------------------------

int JukeboxDS::volume() const
{
	return m_Volume;
}

int JukeboxDS::playListCount() const
{
	return m_PlayLists.size();
}

const Jukebox::PlayList & JukeboxDS::playList( int n ) const
{
	return m_PlayLists[ n ];
}
	
int JukeboxDS::activePlayList() const
{
	return m_ActiveList;
}

int JukeboxDS::activeTrack() const
{
	return m_CurrentTrack;
}

int JukeboxDS::trackCount() const
{
	if ( m_PlayLists.isValid( m_ActiveList ) )
		return m_PlayLists[ m_ActiveList ].files.size();

	return 0;
}

const char * JukeboxDS::track(int n ) const
{
	if ( m_PlayLists.isValid( m_ActiveList ) )
		if ( m_PlayLists[ m_ActiveList ].files.isValid( n ) )
			return m_PlayLists[ m_ActiveList ].files[ n ];

	return NULL;
}

int	JukeboxDS::findPlayList( const char * pName ) const
{
	for(int i=0;i<m_PlayLists.size();i++)
		if ( m_PlayLists[i].name.compareNoCase( pName ) == 0 )
			return i;

	// not found!
	return -1;
}

//----------------------------------------------------------------------------

bool JukeboxDS::initialize( const char * pFilePath, int volume )
{
	release();

	// build the file list
	m_FilePath = Path( pFilePath ).directory();
	if (! m_FilePath.endsWith(("\\")) )
		m_FilePath += ("\\");
	m_JukeboxConfig = m_FilePath + ("config.ini");

	// load the jukebox configuration if the file exists
	if ( FileDisk::fileExists( m_JukeboxConfig ) )
	{
		Settings config( ("Jukebox"), m_JukeboxConfig );

		int listCount = config.get( ("listCount"), (dword)0 );
		for(int i=0;i<listCount;i++)
		{
			int newList = addPlayList( config.get( CharString().format( ("List%dName"), i ), ("") ) );
			int fileCount = config.get( CharString().format(("List%dFileCount"), i ), (dword)0 );

			for(int j=0;j<fileCount;j++)
				addTrack( newList, config.get( CharString().format(("List%dFile%d"), i, j), ("")) );
		}
	}
	else
	{
		// create a default playlist with all mp3 files in the directory
		addTrack( addPlayList( ("Default") ), m_FilePath + ("*.mp3") );
		// save configuration
		//saveConfig();
	}

	if ( !m_PlayLists.isValid( 0 ) )
		return false;
	if ( !m_PlayLists[ 0 ].files.isValid( 0 ) )
		return false;

	m_ActiveList = 0;
	m_CurrentTrack = 0;

	m_Volume = Clamp( volume, 0, 100 );

	// start the thread
	play();

	return true;
}

bool JukeboxDS::play()
{
	if ( m_pPlayerThread == NULL && !m_Active )
	{
		m_Active = true;

		m_pPlayerThread = new PlayerThread( this );
		m_pPlayerThread->resume();
		m_TrackEvent.signal();

		return true;
	}

	return false;	// not implemented yet
}

bool JukeboxDS::stop()
{
	if ( m_pPlayerThread != NULL && m_Active )
	{
		m_Active = false;

		// stop the player thread
		delete m_pPlayerThread;
		m_pPlayerThread = NULL;

		return true;
	}

	return false;	// player not playing
}


void JukeboxDS::release()
{
	// stop any playing
	stop();

	m_FilePath = "";
	m_JukeboxConfig = "";
	m_PlayLists.release();
	m_ActiveList = m_CurrentTrack = -1;
}

bool JukeboxDS::setVolume( int volume )
{
	if ( volume >= 0 && volume <= 100 )
	{
		m_Volume = volume;
		m_UpdateVolume.signal();
		return true;
	}

	// invalid volume number
	return false;
}

bool JukeboxDS::setPlayList( int n, bool bSwitchNow /*= false*/ )
{
	bool done = false;

	m_Lock.lock();
	if ( n >= 0 && n < m_PlayLists.size() && n != m_ActiveList )
	{
		m_ActiveList = n;
		m_CurrentTrack = 0;
		if ( bSwitchNow )
			m_TrackEvent.signal();

		done = true;
	}
	m_Lock.unlock();

	return done;
}

bool JukeboxDS::setPlayList( const char * pName, bool bSwitchNow /*= false*/ )
{
	int n = findPlayList( pName );
	if ( n >= 0 )
		return setPlayList( n, bSwitchNow );

	return false;
}

bool JukeboxDS::setTrack( int n )
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( m_ActiveList ) )
	{
		const PlayList & list = m_PlayLists[ m_ActiveList ];
		if ( n >= 0 && n < list.files.size() )
		{
			m_CurrentTrack = n;
			m_TrackEvent.signal();
			done = true;
		}
	}
	m_Lock.unlock();

	return done;
}

bool JukeboxDS::nextTrack()
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( m_ActiveList ) )
	{
		const PlayList & list = m_PlayLists[ m_ActiveList ];
		if ( list.files.size() > 0 )
		{
			m_CurrentTrack++;
			if ( m_CurrentTrack >= list.files.size() )
				m_CurrentTrack = 0;
			m_TrackEvent.signal();

			done = true;
		}
	}
	m_Lock.unlock();

	return done;
}

bool JukeboxDS::previousTrack()
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( m_ActiveList ) )
	{
		const PlayList & list = m_PlayLists[ m_ActiveList ];
		if ( list.files.size() > 0 )
		{
			m_CurrentTrack--;
			if ( m_CurrentTrack < 0 )
				m_CurrentTrack = list.files.size() - 1;
			m_TrackEvent.signal();

			done = true;
		}
	}
	m_Lock.unlock();

	return done;
}

int JukeboxDS::addPlayList( const char * pName )
{
	int index = -1;

	m_Lock.lock();
	if ( pName != NULL )
	{
		PlayList newList;
		newList.name = pName;

		m_PlayLists.push( newList );
		index = m_PlayLists.size() - 1;
	}
	m_Lock.unlock();

	return index;
}

bool JukeboxDS::deletePlayList( int n )
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( n ) )
	{
		m_PlayLists.remove( n );

		if ( m_ActiveList == n )
		{
			m_ActiveList = m_CurrentTrack = -1;
			m_TrackEvent.signal();
		}
		done = true;
	}
	m_Lock.unlock();

	return done;
}

int JukeboxDS::addTrack( int playList, const char * pFile )
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( playList ) )
	{
		PlayList & list = m_PlayLists[ playList ];

		CharString directory( Path( pFile ).directory() );

		// build the file list
		FindFile ff( pFile );
		for(int i=0;i<ff.fileCount();i++)
			list.files.push( directory + ff.file( i ) );	
		done = true;
	}
	m_Lock.unlock();

	return done;
}

bool JukeboxDS::deleteTrack( int playList, int track )
{
	bool done = false;

	m_Lock.lock();
	if ( m_PlayLists.isValid( playList ) )
	{
		PlayList & list = m_PlayLists[ playList ];
		if ( list.files.isValid( track ) )
		{
			list.files.remove( track );

			if ( m_ActiveList == playList && m_CurrentTrack == track )
			{
				m_CurrentTrack = 0;
				m_TrackEvent.signal();
			}

			done = true;
		}
	}
	m_Lock.unlock();

	return done;
}

//----------------------------------------------------------------------------

void JukeboxDS::saveConfig()
{
	Settings config( ("Jukebox"), m_JukeboxConfig );

	config.put( ("listCount"), m_PlayLists.size() );
	for(int i=0;i<m_PlayLists.size();i++)
	{
		config.put( CharString().format(("List%dName"), i), m_PlayLists[i].name );
		config.put( CharString().format(("List%dFileCount"), i ), m_PlayLists[i].files.size() );

		for(int j=0;j<m_PlayLists[i].files.size();j++)
			config.put( CharString().format(("List%dFile%d"), i, j), m_PlayLists[i].files[j] );
	}
}

//----------------------------------------------------------------------------

int JukeboxDS::player()
{
    if( FAILED(CoInitialize(NULL)) )
		return -1;

	IGraphBuilder *	pGB = NULL;
	IMediaControl *	pMC = NULL;
	IBasicAudio *	pBA = NULL;
	IMediaEvent *	pME = NULL;

	while( m_Active )
	{
		// check the state of the music, if stopped move onto the next file
		if ( pME != NULL )
		{
			long eventCode;
			if ( pME->WaitForCompletion( 0, &eventCode ) == S_OK )
				nextTrack();		// current song has ended, next track
		}

		// check for a volume change
		if ( m_UpdateVolume.signaled() )
		{
			m_UpdateVolume.clear();
			if ( pBA != NULL )
			{
				// set the volume
				long pv = (100 - m_Volume) * -100;
				pBA->put_Volume( pv );
			}
		}

		if (! m_TrackEvent.wait( 250 ) )
		{
			AutoLock lock( &m_Lock );

			if ( m_PlayLists.isValid( m_ActiveList ) && m_PlayLists[ m_ActiveList ].files.isValid( m_CurrentTrack ) )
			{
				// get the filename of the current track
				CharString file = m_PlayLists[ m_ActiveList ].files[ m_CurrentTrack ];

				// release the previous interfaces
				RELEASEQI( pGB );
				RELEASEQI( pMC );
				RELEASEQI( pBA );
				RELEASEQI( pME );

				// Create DirectShow Graph
				if ( FAILED( CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pGB) ) )
					return -1;
				try {
					// load the first file
					if ( FAILED( pGB->RenderFile( String( file ), NULL ) ) )
						return -1;
				}
				catch( ... )
				{
					// driver failure, don't crash the game due to a bad codec..
					return -1;
				}
				// Get the IMediaControl Interface
				if ( FAILED( pGB->QueryInterface(IID_IMediaControl, (void **)&pMC ) ) )
					return -1;
				// Get the IBasicAudio interface
				if ( FAILED( pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA) ) )
					return -1;
				// Get the IBasicAudio interface
				if ( FAILED( pGB->QueryInterface(IID_IMediaEvent, (void **)&pME) ) )
					return -1;
				// set the volume
				long pv = (100 - m_Volume) * -100;
				pBA->put_Volume( pv );
				// play the music
				pMC->Run();
			}
		}
	}

	RELEASEQI( pGB );
	RELEASEQI( pMC );
	RELEASEQI( pBA );
	RELEASEQI( pME );

	CoUninitialize();

	return 0;
}

//----------------------------------------------------------------------------
//EOF
