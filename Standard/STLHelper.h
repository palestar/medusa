/**
	@file STLHelper.h
	@brief STL helper functions

	(c)2012 Palestar Inc
	@author Richard Lyle @date 9/3/2012 12:47:29 AM
*/

#ifndef STLHELPER_H
#define STLHELPER_H

#include <vector>

//---------------------------------------------------------------------------------------------------

template<typename T>
inline bool removeSwap( std::vector<T> & a_Vector, T a_Remove )
{
	for( size_t i=0;i<a_Vector.size();++i)
	{
		if ( a_Vector[i] == a_Remove )
		{
			if ( i < (a_Vector.size() - 1) )
				a_Vector[ i ] = a_Vector[ a_Vector.size() - 1 ];
			a_Vector.pop_back();

			return true;
		}
	}
	return false;
}

template<typename T>
inline void removeSwapIndex( std::vector<T> & a_Vector, size_t i )
{
	if ( i < (a_Vector.size() - 1) )
		a_Vector[ i ] = a_Vector[ a_Vector.size() - 1 ];
	a_Vector.pop_back();
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
