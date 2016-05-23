/*
	PrimitiveWindow.h

	This display primitive renders a quad using the current material & surface
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_WINDOW_H
#define PRIMITIVE_WINDOW_H

#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveWindow : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveWindow>	Ref;

	// Mutators
	virtual bool				initialize( const RectInt & window, const RectFloat & uv, Color diffuse ) = 0;
	// Helpers
	static Ref					create( DisplayDevice * pDevice, const RectInt & window, const RectFloat & uv, Color diffuse )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( window, uv, diffuse );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, const RectInt & window, const RectFloat & uv, Color diffuse )
	{
		pDevice->push( create( pDevice, window, uv, diffuse ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
