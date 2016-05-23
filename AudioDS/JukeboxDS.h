/*
	JukeboxDS.h

	Concrete implementation of Jukebox
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef JUKEBOXDS_H
#define JUKEBOXDS_H

#include "Debug/SafeThread.h"
#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "Audio/Jukebox.h"

//----------------------------------------------------------------------------

class JukeboxDS : public Jukebox
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	JukeboxDS();
	virtual ~JukeboxDS();

	// Jukebox interface
	int					volume() const;						// current volume

	int					playListCount() const;				// get the number of playlists
	const PlayList &	playList( int n ) const;			// get playlist n
	
	int					activePlayList() const;				// get the current play list name

	int					trackCount() const;					// how many tracks in the current playlist
	const char *		track(int n ) const;				// get track n
	int					activeTrack() const;				// currently playing track in the active play list

	int					findPlayList( 
							const char * pName ) const;		// find the specified playlist by name, returns -1 if not found

	// Mutators
	bool				initialize( const char * pFilePath, int volume );	
	bool				play();			
	bool				stop();				
	void				release();

	bool				setVolume( int volume );			// set the current volume, 0 <= volume <= 100
	bool				setPlayList( int n,
							bool bSwitchNow = false );		// set the current playlist
	bool				setPlayList( const char * pName,
							bool bSwitchNow = false );		// set the current playlist by name

	bool				setTrack( int n );					// change the current playing track
	bool				nextTrack();						// move to the next track
	bool				previousTrack();					// move to the previous track


	int					addPlayList( const char * pName );	// add a new play list, returns the index of the new play list
	bool				deletePlayList( int n );			// delete the specified play list, returns true on success
	int					addTrack( int playList, 
								const char * pFile );		// add a track to the specified play list
	bool				deleteTrack( int playlist, 
								int track );				// remove a track from a play list

private:
	// Data
	bool				m_Active;							// are we active
	int					m_Volume;							// current volume
	Event				m_UpdateVolume;						// signaled when the volume is changed
	CharString			m_FilePath;							// path to the music directory
	CharString			m_JukeboxConfig;					// jukebox configuration file

	CriticalSection		m_Lock;								// thread lock
	Array< PlayList >	m_PlayLists;						// play lists found
	int					m_ActiveList;						// currently active playlist
	int					m_CurrentTrack;						// current track
	Event				m_TrackEvent;						// signaled when the track is changed

	// Mutators
	void				saveConfig();						// save configuration

	int					player();

	class PlayerThread : public SafeThread
	{
	public:
		// Construction
		PlayerThread( JukeboxDS * pJukeBox ) : m_pJukeBox( pJukeBox ) 
		{}
		// Thread interface
		int				run()
		{
			return m_pJukeBox->player();
		}
	private:
		// Data
		JukeboxDS *		m_pJukeBox;
	};
	
	PlayerThread *		m_pPlayerThread;

	// Friend
	friend class PlayerThread;
};


//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
