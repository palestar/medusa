/*
	Movie.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef MOVIE_H
#define MOVIE_H

#include "Display/PrimitiveSurface.h"
#include "Standard/Types.h"
#include "Standard/Point.h"
#include "Standard/String.h"
#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

// concrete movie objects
const ClassKey		CLASS_KEY_MOVIE(3977131662882069166LL);
const ClassKey		CLASS_KEY_MOVIE_DS(3977131717630319510LL);

//-------------------------------------------------------------------------------

class DLL Movie : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	Movie();

	// Accessors
	virtual bool		playing() const = 0;	// is the movie currently playing
	virtual bool		paused() const = 0;		// is the movie currently paused
	virtual int			volume() const = 0;		// get the current volume 
	virtual dword		length() const = 0;		// get the length of the movie in milliseconds
	virtual dword		position() const = 0;	// get the current position in the movie in ms
	virtual bool		repeat() const = 0;		// is the repeat option on or off

	// Mutators
	virtual bool		initialize( const wchar * pFile, 
							int volume = 100 ) = 0;				// play movie fullscreen
	virtual bool		initialize( DisplayDevice * pDevice, 
							const wchar * pFile, 
							int volume = 100 ) = 0;				// play movie on the provided display device
	virtual bool		initialize( PrimitiveSurface * pSurface, 
							const wchar * pFile, 
							int volume = 100 ) = 0;				// play movie into the specified surface

	virtual bool		play() = 0;				// play the movie
	virtual bool		stop() = 0;				// stop the movie
	virtual bool		pause() = 0;			// pause/unpause this movie
	virtual bool		setPosition( dword ms ) = 0;	// set the movie current position
	virtual bool		setRepeat( bool repeat ) = 0;	// toggle repeat option on or off
	virtual bool		setVolume( int volume ) = 0;	// set the movie playback volume

	virtual void		release() = 0;			// release the movie

	// Static
	static Movie *		create();				// create the default movie player for this platform
	static Movie *		create( const ClassKey & key );
};

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF


