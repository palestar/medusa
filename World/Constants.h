/**
	@file Constants.h
	@brief TODO
	
	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/15/2006 9:26:25 PM
*/

#ifndef WORLD_CONSTANTS_H
#define WORLD_CONSTANTS_H

#include "Standard/Constant.h"
#include "Math/Constants.h"
#include "Math/Vector3.h"
#include "File/Stream.h"
#include "Standard/Color.h"

#include "WorldDll.h"

//----------------------------------------------------------------------------

const int		TICKS_PER_SECOND		= 20;									// time steps per second
const int		TICKS_PER_MINUTE		= TICKS_PER_SECOND * 60;
const int		TICKS_PER_HOUR			= TICKS_PER_MINUTE * 60;
const int		TICK_DURATION_MS		= 1000 / TICKS_PER_SECOND;				// = 50
const float		TICK_DURATION_S			= float(TICK_DURATION_MS) / 1000.0f;	// = 0.05f

const float		UNIVERSE_SECTOR = 5000;
const float		GALACTIC_UNIT_SIZE = 500000.0f;

//----------------------------------------------------------------------------

inline float tickTime( dword ticks )
{
	return( ticks * TICK_DURATION_S );
}

inline dword timeTick( float time )
{
	return( dword(time * TICKS_PER_SECOND) );
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
