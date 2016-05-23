/*
	RandomNumber.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "RandomNumber.h"
#include <assert.h>

// Constants
const unsigned int		MULT1			= 3123123123U;
const unsigned int		MULT2			= 534234U;
const unsigned int		BASEMULTIPLIER	= 31415821U;

// Static 
unsigned int RandomNumber::s_Seed		= 5602034U;

//-------------------------------------------------------------------------------

unsigned int RandomNumber::generate( unsigned int lowerBound, unsigned int upperBound, unsigned int seed)
{
	assert( lowerBound < upperBound );
	unsigned int range = upperBound - lowerBound;

	// assign the seed if not 0
	if (seed != 0)
		s_Seed = seed;

	// update the seed
	unsigned int op11 = seed / MULT2;
	unsigned int op12 = seed % MULT2;
	unsigned int op21 = BASEMULTIPLIER / MULT2;
	unsigned int op22 = BASEMULTIPLIER % MULT2;

	seed = (((((op12 * op21) + (op11 * op22)) % MULT2) * MULT2) + (op12 * op22)) % MULT1;

	return( lowerBound + (s_Seed % range) );

}

//-------------------------------------------------------------------------------
// EOF
