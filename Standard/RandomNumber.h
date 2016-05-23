/*
	RandomNumber.h

	Creates a random number
	(c)2005 Palestar, Richard Lyle
*/

#ifndef RANDOM_NUMBER_H
#define RANDOM_NUMBER_H

#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL RandomNumber
{
public:
	// Construction
	RandomNumber(unsigned int lowerBound = 0,unsigned int upperBound = 65536, unsigned int seed = 0) 
		: m_Number( generate( lowerBound, upperBound, seed ) ) {}
	// Accessors
	operator				unsigned int() const		{ return( m_Number); }

private:
	// Data
	unsigned int			m_Number;
	static unsigned int		s_Seed;

	// Helpers
	static unsigned int		generate( unsigned int lowerBound, unsigned int upperBound, unsigned int seed);
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
