/*
	Chopper.h

	This class is used to reduce the triangle count of a mesh, used by NodeComplexMesh to generate detail levels
	(c)2005 Palestar, Richard Lyle
*/

#ifndef CHOPPER_H
#define CHOPPER_H

#include "Math/Vector3.h"
#include "Standard/Array.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL Chopper 
{
public:
	// Types
	struct Triangle
	{
		int			v[ 3 ];		// vertex indicies
	};

	static void			chop( Array< Vector3 > & vert, Array< Triangle > &tri, 
							Array< int > &map, Array< int > &permutation );
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
