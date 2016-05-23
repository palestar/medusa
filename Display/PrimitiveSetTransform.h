/*
	PrimitiveSetTransform.h

	This primitive is pushed into the display device to transform the rendered geometry from object-space into view-space.
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_SET_TRANSFORM_H
#define PRIMITIVE_SET_TRANSFORM_H

#include "Math/Vector3.h"
#include "Math/Matrix33.h"
#include "Display/DisplayDevice.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveSetTransform : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveSetTransform>	Ref;

	// Accessors
	virtual const Matrix33 &		transform() const = 0;
	virtual const Vector3 &			transformOffset() const = 0;

	// Mutators
	virtual void					setTransform( const Matrix33 &transform, const Vector3 &transformOffset ) = 0;

	// Helpers
	static Ref create( DisplayDevice * pDisplay )
	{
		// create the primitive
		Ref pPrimitive;
		pDisplay->create( pPrimitive );
		// setup the primitive
		pPrimitive->setTransform( Matrix33( true ), Vector3( 0.0f ) );

		return pPrimitive;
	}
	static Ref create( DisplayDevice * pDevice, const Matrix33 &transform, const Vector3 &transformOffset )
	{
		Ref pPrimitive;

		// create the primitive
		pDevice->create( pPrimitive );
		// setup the primitive
		pPrimitive->setTransform( transform, transformOffset );

		return(pPrimitive);
	}
	static void	push( DisplayDevice * pDevice, const Matrix33 &transform, const Vector3 &transformOffset )
	{
		pDevice->push( create( pDevice, transform, transformOffset ) );
	}
	static void push( DisplayDevice * pDisplay )
	{
		pDisplay->push( create( pDisplay ) );
	}
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
