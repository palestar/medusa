/*
	Movie.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

//----------------------------------------------------------------------------

#define MOVIE_DLL
#include "Movie/Movie.h"
#include "Standard/Library.h"

//-------------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( Movie, Widget );

Movie::Movie()
{}

//-------------------------------------------------------------------------------

void LoadMovieLibs()
{
#ifdef _DEBUG
	static Library LIB_MOVIEDS( "MovieDSD.dll" );
#else
	static Library LIB_MOVIEDS( "MovieDS.dll" );
#endif
}

//----------------------------------------------------------------------------


Movie * Movie::create()
{
	LoadMovieLibs();
	// if found, create the first available class type derived from this object
	if ( Factory::typeCount( CLASS_KEY_MOVIE ) > 0 )
		return create( Factory::type( CLASS_KEY_MOVIE, 0 ) );

	return NULL;
}

Movie * Movie::create( const ClassKey & key )
{
	LoadMovieLibs();
	return( WidgetCast<Movie>(Factory::createWidget( key )) );
}

//-------------------------------------------------------------------------------
// EOF

