/*
	MovieDS.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define MOVIEDS_DLL
#include "MovieDS/MovieDS.h"
#include "Debug/Assert.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( MovieDS, Movie );

MovieDS::MovieDS()
{}

//-------------------------------------------------------------------------------

bool MovieDS::playing() const
{
	return false;
}

bool MovieDS::paused() const
{
	return false;
}

int MovieDS::volume() const
{
	return 0;
}

dword MovieDS::length() const
{
	return 0;
}

dword MovieDS::position() const
{
	return 0;
}

bool MovieDS::repeat() const
{
	return false;
}

//----------------------------------------------------------------------------

bool MovieDS::initialize( const wchar * pFile, int volume /*= 100*/ )
{
	return false;
}

bool MovieDS::initialize( DisplayDevice * pDevice, const wchar * pFile, int volume /*= 100*/ )
{
	// not implemented
	return false;
}

bool MovieDS::initialize( PrimitiveSurface * pSurface, const wchar * pFile, int volume /*= 100*/ )
{
	// not implemented
	return false;
}

bool MovieDS::play()
{
	return false;
}

bool MovieDS::stop()
{
	return false;
}

bool MovieDS::pause()
{
	return false;
}

bool MovieDS::setPosition( dword ms )
{
	return false;
}

bool MovieDS::setRepeat( bool repeat )
{
	return false;
}

bool MovieDS::setVolume( int volume )
{
	return false;
}

void MovieDS::release()
{
}

//----------------------------------------------------------------------------
//EOF
