/*
	MovieDS.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef MOVIEDS_H
#define MOVIEDS_H

#include "Movie/Movie.h"
#include "MovieDS/MovieDSDLL.h"

//#include <dshow.h>		// DirectShow 



//----------------------------------------------------------------------------

class DLL MovieDS : public Movie
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	MovieDS();

	// Movie interface
	bool			playing() const;					// is the movie currently playing
	bool			paused() const;						// is the movie currently paused
	int				volume() const;						// get the current volume 
	dword			length() const;						// get the length of the movie in milliseconds
	dword			position() const;					// get the current position in the movie in ms
	bool			repeat() const;						// is the repeat option on or off

	bool			initialize( const wchar * pFile, 
						int volume = 100 );				// play movie fullscreen
	bool			initialize( DisplayDevice * pDevice, 
						const wchar * pFile, 
						int volume = 100 );				// play movie on the provided display device
	bool			initialize( PrimitiveSurface * pSurface, 
						const wchar * pFile, 
						int volume = 100 );				// play movie into the specified surface

	bool			play();								// play the movie
	bool			stop();								// stop the movie
	bool			pause();							// pause/unpause this movie
	bool			setPosition( dword ms );			// set the movie current position
	bool			setRepeat( bool repeat );			// toggle repeat option on or off
	bool			setVolume( int volume );			// set the movie playback volume

	void			release();							// release the movie

private:
	// Data
    //IGraphBuilder *	m_pGraph;
    //IMediaControl *	m_pControl;
    //IMediaEvent   *	m_pEvent;
};

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

