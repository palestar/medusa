/*
	Jukebox.h

	This class is an abstract interface for an object that plays a collection of audio files in order
	from a directory on the disk.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef JUKEBOX_H
#define JUKEBOX_H

#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Jukebox : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	struct PlayList
	{
		CharString			name;			// name of this playlist
		Array< CharString >	files;			// full filenames of all tracks in this playlist

		PlayList & operator=( const PlayList & copy );
	};

	// Construction
	Jukebox()
	{}

	// Accessors
	virtual int				volume() const = 0;						// current volume

	virtual int				playListCount() const = 0;				// number of play lists 
	virtual const PlayList &	
							playList( int n ) const = 0;			// get the name of play list n
	virtual int				activePlayList() const = 0;				// get the current play list name

	virtual int				trackCount() const = 0;					// how many tracks in the current playlist
	virtual const char *	track(int n ) const = 0;				// get track n
	virtual int				activeTrack() const = 0;				// currently playing track in the active play list

	virtual int				findPlayList( 
								const char * pName ) const = 0;		// find the specified playlist by name, returns -1 if not found
	
	// Mutators
	virtual bool			initialize( const char * pFilePath, int volume ) = 0;	
	virtual bool			play() = 0;								// unpause playback
	virtual bool			stop() = 0;								// pause playing
	virtual void			release() = 0;

	virtual bool			setVolume( int volume ) = 0;			// set the current volume, 0 <= volume <= 100
	virtual bool			setPlayList( int n, 
								bool bSwitchNow = false ) = 0;		// set the current playlist
	virtual bool			setPlayList( const char * pName,
								bool bSwitchNow = false ) = 0;		// set the current playlist by name

	virtual bool			setTrack( int n ) = 0;					// change the current playing track
	virtual bool			nextTrack() = 0;						// move to the next track
	virtual bool			previousTrack() = 0;					// move to the previous track


	virtual int				addPlayList( const char * pName ) = 0;	// add a new play list, returns the index of the new play list
	virtual bool			deletePlayList( int n ) = 0;			// delete the specified play list, returns true on success
	virtual int				addTrack( int playList, 
								const char * pFile ) = 0;			// add a track to the specified play list
	virtual bool			deleteTrack( int playlist, 
								int track ) = 0;					// remove a track from a play list

	// Static
	static Jukebox *		create();
};

//----------------------------------------------------------------------------

inline Jukebox::PlayList & Jukebox::PlayList::operator=( const PlayList & copy )
{
	name = copy.name;
	files = copy.files;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
